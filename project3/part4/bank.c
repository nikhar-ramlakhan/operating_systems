#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>

#define NUM_WORKERS 10
#define TRANSACTION_THRESHOLD 5000

typedef struct {
    char transaction_type;  
    char src_account[17];
    char dest_account[17];  
    char password[9];
    double amount;          
} Transaction;

typedef struct {
    Transaction* transactions;  
    int total_requests;
    account* accounts;
    int total_accounts;        
} ThreadData;

typedef struct {
    char account_number[17];
    char password[9];
    double duck_bank_balance;
    double puddles_bank_balance;
    double reward_rate;
    pthread_mutex_t lock;
} SharedAccount;

// Function declarations
void* process_transaction(void* args);
void* update_balance(void* arg);
void* puddles_savings(void* arg);
account* get_account_by_number(account* accounts, int num_accounts, char* account_number);
int is_valid_account(account* acc, char* password);
void handle_transfer(account* accounts, int num_accounts, Transaction* request);
void handle_deposit(account* accounts, int num_accounts, Transaction* request);
void handle_withdraw(account* accounts, int num_accounts, Transaction* request);
void handle_check_balance(account* accounts, int num_accounts, Transaction* request);
void clear_account_files(int num_accounts);
void clear_savings_files(int num_accounts);
void signalHandler(int signum);
void signalHandler2(int signum);

// Global variables
account bank_accounts[100];
int transactions_done = 0;
int update_id = 0;
int need_update = 0;
int completed_requests = 0;
int total_accounts = 0;
int update_count = 0;
pthread_mutex_t balance_update_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t balance_update_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t accounts_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t sync_barrier;
pthread_t updater_thread;
pthread_t worker_threads[NUM_WORKERS];
pthread_t puddles_thread;
SharedAccount* shared_accounts;
int* ready_for_signal;

void clear_account_files(int num_accounts) {
    char filename[50];
    for (int i = 0; i < num_accounts; ++i) {
        sprintf(filename, "output/%s.txt", bank_accounts[i].account_number);
        FILE* account_file = fopen(filename, "w");
        if (account_file) {
            fclose(account_file);
        }
    }
}

void clear_savings_files(int num_accounts) {
    mkdir("savings", 0777); // Create the directory if it doesn't exist
    for (int i = 0; i < num_accounts; ++i) {
        char filename[256];
        sprintf(filename, "savings/account%d.txt", i);
        FILE* account_file = fopen(filename, "w");
        if (account_file) {
            fclose(account_file);
        }
    }
}

account* get_account_by_number(account* accounts, int num_accounts, char* account_number) {
    for (int i = 0; i < num_accounts; ++i) {
        if (strcmp(accounts[i].account_number, account_number) == 0) {
            return &accounts[i];
        }
    }
    return NULL;
}

int is_valid_account(account* acc, char* password) {
    return strcmp(acc->password, password) == 0;
}

void handle_transfer(account* accounts, int num_accounts, Transaction* request) {
    pthread_mutex_lock(&accounts_lock);
    account* source_acc = get_account_by_number(accounts, num_accounts, request->src_account);
    account* dest_acc = get_account_by_number(accounts, num_accounts, request->dest_account);

    if (source_acc && dest_acc && is_valid_account(source_acc, request->password)) {
        pthread_mutex_lock(&source_acc->ac_lock);
        pthread_mutex_lock(&dest_acc->ac_lock);

        source_acc->balance -= request->amount;
        dest_acc->balance += request->amount;

        source_acc->transaction_tracter += request->amount;
        pthread_mutex_unlock(&dest_acc->ac_lock);
        pthread_mutex_unlock(&source_acc->ac_lock);
    }
    pthread_mutex_unlock(&accounts_lock);
}

void handle_deposit(account* accounts, int num_accounts, Transaction* request) {
    pthread_mutex_lock(&accounts_lock);
    
    account* acc = get_account_by_number(accounts, num_accounts, request->src_account);
    if (acc && is_valid_account(acc, request->password)) {
        pthread_mutex_lock(&acc->ac_lock);
        acc->balance += request->amount;
        acc->transaction_tracter += request->amount;
        pthread_mutex_unlock(&acc->ac_lock);
    }
    pthread_mutex_unlock(&accounts_lock); 
}

void handle_withdraw(account* accounts, int num_accounts, Transaction* request) {
    pthread_mutex_lock(&accounts_lock);
    
    account* acc = get_account_by_number(accounts, num_accounts, request->src_account);
    if (acc && is_valid_account(acc, request->password)) {
        pthread_mutex_lock(&acc->ac_lock);
        acc->balance -= request->amount;
        acc->transaction_tracter += request->amount;
        pthread_mutex_unlock(&acc->ac_lock);
    }
    pthread_mutex_unlock(&accounts_lock); 
}

void handle_check_balance(account* accounts, int num_accounts, Transaction* request) {
    pthread_mutex_lock(&accounts_lock);
    account* acc = get_account_by_number(accounts, num_accounts, request->src_account);
    if (acc && is_valid_account(acc, request->password)) {
        pthread_mutex_lock(&acc->ac_lock);
        pthread_mutex_unlock(&acc->ac_lock);
    }
    pthread_mutex_unlock(&accounts_lock);
}

void* process_transaction(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    printf("Thread %lu started processing %d transactions\n", pthread_self(), data->total_requests);

    pthread_barrier_wait(&sync_barrier);
    for (int i = 0; i < data->total_requests; i++) {
        Transaction* request = &data->transactions[i];
        account* acc = get_account_by_number(data->accounts, data->total_accounts, request->src_account);
        
        switch(request->transaction_type) {
            case 'T':
                handle_transfer(data->accounts, data->total_accounts, request);
                break;
            case 'D':
                handle_deposit(data->accounts, data->total_accounts, request);
                break;
            case 'W':
                handle_withdraw(data->accounts, data->total_accounts, request);
                break;
            case 'C':
                handle_check_balance(data->accounts, data->total_accounts, request);
                break;
            default:
                break;
        }

        if (request->transaction_type == 'T' || request->transaction_type == 'D' || request->transaction_type == 'W') {
            if (is_valid_account(acc, request->password)) {
                pthread_mutex_lock(&balance_update_mutex);
                completed_requests++;
                pthread_mutex_unlock(&balance_update_mutex);
            }
        }
        
        pthread_mutex_lock(&balance_update_mutex);
        if (completed_requests >= TRANSACTION_THRESHOLD && !need_update) {
            need_update = 1;
            update_id++;
            printf("Update number: %d\n", update_id);
            pthread_cond_signal(&balance_update_cond);
        }
        pthread_mutex_unlock(&balance_update_mutex);

        pthread_mutex_lock(&balance_update_mutex);
        while (need_update) {
            pthread_cond_wait(&balance_update_cond, &balance_update_mutex);
        }
        pthread_mutex_unlock(&balance_update_mutex);
    }
    printf("Thread %lu finished processing transactions\n", pthread_self());
    return NULL;
}

void* update_balance(void* arg) {
    while (1) {
        pthread_mutex_lock(&balance_update_mutex);
        while (!need_update && !transactions_done) {
            pthread_cond_wait(&balance_update_cond, &balance_update_mutex);
        }

        if (transactions_done && !need_update) {
            pthread_mutex_unlock(&balance_update_mutex);
            break;
        }
        char filename[50];
        mkdir("output", 0777);
        pthread_mutex_lock(&accounts_lock);
        for (int i = 0; i < total_accounts; ++i) {
            double reward = bank_accounts[i].transaction_tracter * bank_accounts[i].reward_rate;
            bank_accounts[i].balance += reward;
            bank_accounts[i].transaction_tracter = 0;

            sprintf(filename, "output/%s.txt", bank_accounts[i].account_number);
            FILE* account_file = fopen(filename, "a");
            if (account_file) {
                fprintf(account_file, "Current Balance: %.2f\n", bank_accounts[i].balance);
                fclose(account_file);
            }

            double puddles_reward = shared_accounts[i].puddles_bank_balance * shared_accounts[i].reward_rate;
            shared_accounts[i].puddles_bank_balance += puddles_reward;

            char savings_filename[256];
            sprintf(savings_filename, "savings/account%d.txt", i);
            account_file = fopen(savings_filename, "a");
            if (account_file) {
                fprintf(account_file, "Current Savings Balance %.2f\n", shared_accounts[i].puddles_bank_balance);
                fclose(account_file);
            }
        }
        pthread_mutex_unlock(&accounts_lock);

        need_update = 0;
        completed_requests = 0;
        pthread_cond_broadcast(&balance_update_cond);

        if (transactions_done) {
            pthread_mutex_unlock(&balance_update_mutex);
            break;
        }
        pthread_mutex_unlock(&balance_update_mutex);
    }
    return NULL;
}

void signalHandler(int signum) {
    if (signum == SIGUSR1) {
        for (int i = 0; i < total_accounts; ++i) {
            double puddles_reward = shared_accounts[i].puddles_bank_balance * shared_accounts[i].reward_rate;
            shared_accounts[i].puddles_bank_balance += puddles_reward;

            char savings_filename[256];
            sprintf(savings_filename, "savings/account%d.txt", i);
            FILE* account_file = fopen(savings_filename, "a");
            if (account_file) {
                fprintf(account_file, "Current Savings Balance %.2f\n", shared_accounts[i].puddles_bank_balance);
                fclose(account_file);
            }
        }
    }
}

void signalHandler2(int signum) {
    if (signum == SIGUSR2) {
        exit(0);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Invalid number of commands");
        return -1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    char line[100];
    fgets(line, sizeof(line), file);
    sscanf(line, "%d", &total_accounts);

    shared_accounts = mmap(NULL, total_accounts * sizeof(SharedAccount), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_accounts == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    ready_for_signal = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ready_for_signal == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    *ready_for_signal = 0;

    for (int i = 0; i < total_accounts; i++) {
        fgets(line, sizeof(line), file);

        fgets(line, sizeof(line), file);
        sscanf(line, "%s", bank_accounts[i].account_number);

        fgets(line, sizeof(line), file);
        sscanf(line, "%s", bank_accounts[i].password);

        fgets(line, sizeof(line), file);
        sscanf(line, "%lf", &bank_accounts[i].balance);

        fgets(line, sizeof(line), file);
        sscanf(line, "%lf", &bank_accounts[i].reward_rate);

        pthread_mutex_init(&bank_accounts[i].ac_lock, NULL);
        bank_accounts[i].transaction_tracter = 0;

        strcpy(shared_accounts[i].account_number, bank_accounts[i].account_number);
        strcpy(shared_accounts[i].password, bank_accounts[i].password);
        shared_accounts[i].duck_bank_balance = bank_accounts[i].balance;
        shared_accounts[i].puddles_bank_balance = bank_accounts[i].balance * 0.2;
        shared_accounts[i].reward_rate = 0.02;
        pthread_mutex_init(&shared_accounts[i].lock, NULL);
    }

    clear_account_files(total_accounts);
    clear_savings_files(total_accounts);

    int total_transactions = 0;
    char buffer[100];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        total_transactions++;
    }
    rewind(file);

    Transaction* all_transactions = calloc(total_transactions + 1, sizeof(Transaction));
    if (all_transactions == NULL) {
        perror("Failed to allocate transactions");
        exit(-1);
    }

    int transaction_idx = 0;
    for (int i = 0; i < total_accounts * 5; i++) {
        fgets(buffer, sizeof(buffer), file);
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        Transaction* request = &all_transactions[transaction_idx];
        char transaction_type;
        if (sscanf(buffer, " %c", &transaction_type) == 1) {
            request->transaction_type = transaction_type;
            if (transaction_type == 'T') {
                sscanf(buffer, "T %16s %8s %16s %lf", request->src_account, request->password, request->dest_account, &request->amount);
            } else if (transaction_type == 'C') {
                sscanf(buffer, "C %16s %8s", request->src_account, request->password);
            } else if (transaction_type == 'D' || transaction_type == 'W') {
                sscanf(buffer, "%*c %16s %8s %lf", request->src_account, request->password, &request->amount);
            }
            transaction_idx++;
        }
    }

    pthread_barrier_init(&sync_barrier, NULL, NUM_WORKERS + 1);
    ThreadData worker_data[NUM_WORKERS];

    int base_requests_per_thread = transaction_idx / NUM_WORKERS;
    int remaining_requests = transaction_idx % NUM_WORKERS;

    for (int i = 0; i < NUM_WORKERS; i++) {
        int start_index = i * base_requests_per_thread;
        int requests_for_thread = base_requests_per_thread;
        if (i == NUM_WORKERS - 1) {
            requests_for_thread += remaining_requests;
        }
        worker_data[i].transactions = &all_transactions[start_index];
        worker_data[i].total_requests = requests_for_thread;
        worker_data[i].accounts = bank_accounts;
        worker_data[i].total_accounts = total_accounts;
        pthread_create(&worker_threads[i], NULL, process_transaction, &worker_data[i]);
        printf("Creating worker thread %lu to handle transactions %d to %d\n", pthread_self(), start_index, start_index + requests_for_thread - 1);
    }
    pthread_create(&updater_thread, NULL, update_balance, NULL);

    pid_t puddles_bank_pid = fork();
    if (puddles_bank_pid == 0) {
        signal(SIGUSR1, signalHandler);
        signal(SIGUSR2, signalHandler2);

        while (1) {
            pause();
        }
    } else {
        pthread_barrier_wait(&sync_barrier);
        for (int i = 0; i < NUM_WORKERS; i++) {
            pthread_join(worker_threads[i], NULL);
        }

        pthread_mutex_lock(&balance_update_mutex);
        transactions_done = 1;
        if (completed_requests > 0) {
            need_update = 1;
            update_id++;
            printf("Update number: %d\n", update_id);
            pthread_cond_signal(&balance_update_cond);
        }
        pthread_mutex_unlock(&balance_update_mutex);

        pthread_join(updater_thread, NULL);

        while (1) {
            if (*ready_for_signal == 1) {
                kill(puddles_bank_pid, SIGUSR2);
                break;
            }
        }
    }

    munmap(shared_accounts, total_accounts * sizeof(SharedAccount));
    munmap(ready_for_signal, sizeof(int));
    pthread_barrier_destroy(&sync_barrier);
    pthread_cond_destroy(&balance_update_cond);
    pthread_mutex_destroy(&balance_update_mutex);
    free(all_transactions);
    fclose(file);

    return 0;
}
