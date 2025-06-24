// CS 415
// Project 3 | Part 2
// Nikhar Ramlakhan
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "string_parser.h"
#include "account.h"

#define NUM_WORKERS 10

// Function declarations
void *process_transaction(void *args);
void *update_balance(void *arg);
int find_account_num(char *account_num, account *accounts);

// Global variables
account *accounts;
int num_accounts;
pthread_t *thread_ids;
int *update_times; // Global variable to track update times

typedef struct {
    char **lines;
    int start_index;
    int end_index;
} thread_args;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error. Input: ./bank <input file>\n");
        exit(EXIT_FAILURE);
    }

    char *input = NULL;
    size_t len_input = 0;
    ssize_t read_bytes;
    FILE *f = fopen(argv[1], "r");

    if (f == NULL) {
        fprintf(stderr, "File: %s does not exist\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    getline(&input, &len_input, f);
    num_accounts = atoi(input);

    accounts = (account *)malloc(sizeof(account) * num_accounts);
    update_times = (int *)malloc(sizeof(int) * num_accounts); // Allocate memory for update times

    for (int i = 0; i < num_accounts; ++i) {
        getline(&input, &len_input, f);
        accounts[i].transaction_tracter = 0.0;
        pthread_mutex_init(&accounts[i].ac_lock, NULL);

        getline(&input, &len_input, f);
        strncpy(accounts[i].account_number, input, sizeof(accounts[i].account_number) - 1);
        accounts[i].account_number[sizeof(accounts[i].account_number) - 1] = '\0';

        getline(&input, &len_input, f);
        strncpy(accounts[i].password, input, sizeof(accounts[i].password) - 1);
        accounts[i].password[sizeof(accounts[i].password) - 1] = '\0';

        getline(&input, &len_input, f);
        accounts[i].balance = strtod(input, NULL);

        getline(&input, &len_input, f);
        accounts[i].reward_rate = strtod(input, NULL);

        update_times[i] = 0; // Initialize update times
    }

    int line_count = 0;
    while ((read_bytes = getline(&input, &len_input, f)) != -1) {
        line_count++;
    }

    rewind(f);
    getline(&input, &len_input, f);
    for (int i = 0; i < num_accounts; ++i) {
        for (int j = 0; j < 5; ++j) {
            getline(&input, &len_input, f);
        }
    }

    char **transaction_lines = (char **)malloc(sizeof(char *) * line_count);
    for (int i = 0; i < line_count; ++i) {
        getline(&input, &len_input, f);
        transaction_lines[i] = strdup(input);
    }

    int even_slice = line_count / NUM_WORKERS;
    thread_ids = (pthread_t *)malloc(sizeof(pthread_t) * NUM_WORKERS);
    thread_args *t_args = (thread_args *)malloc(sizeof(thread_args) * NUM_WORKERS);

    for (int i = 0; i < NUM_WORKERS; ++i) {
        t_args[i].lines = transaction_lines;
        t_args[i].start_index = i * even_slice;
        t_args[i].end_index = (i == NUM_WORKERS - 1) ? line_count : (i + 1) * even_slice;
        pthread_create(&thread_ids[i], NULL, process_transaction, (void *)&t_args[i]);
    }

    for (int j = 0; j < NUM_WORKERS; ++j) {
        pthread_join(thread_ids[j], NULL);
    }

    pthread_t bank_thread;
    pthread_create(&bank_thread, NULL, update_balance, NULL);
    pthread_join(bank_thread, NULL);

    fclose(f);
    FILE *q = fopen("output.txt", "w");
    if (q == NULL) {
        fprintf(stderr, "Error opening output file\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_accounts; ++i) {
        fprintf(q, "%d balance:\t%.2f\n\n", i, accounts[i].balance);
    }

    mkdir("output", 0777);
    for (int i = 0; i < num_accounts; ++i) {
        char file_name[256];
        sprintf(file_name, "output/%s.txt", accounts[i].account_number);
        FILE *out = fopen(file_name, "w");
        if (out == NULL) {
            fprintf(stderr, "Error opening output file\n");
            exit(EXIT_FAILURE);
        }
        fprintf(out, "account %d:\n", i);
        fprintf(out, "Current Balance: %.2f\n", accounts[i].balance);
        fclose(out);
    }

    // Print update times on the terminal
    printf("Update times for each account:\n");
    for (int i = 0; i < num_accounts; ++i) {
        printf("Account %d (%s): %d updates\n", i, accounts[i].account_number, update_times[i]);
    }

    free(accounts);
    free(input);
    fclose(q);
    free(thread_ids);
    free(t_args);
    free(update_times); // Free the allocated memory for update times
    for (int i = 0; i < line_count; ++i) {
        free(transaction_lines[i]);
    }
    free(transaction_lines);

    return 0;
}

void *process_transaction(void *args) {
    thread_args *t_args = (thread_args *)args;
    for (int i = t_args->start_index; i < t_args->end_index; ++i) {
        command_line line_buffer = str_filler(t_args->lines[i], " ");
        char code = line_buffer.command_list[0][0];

        int account_index;
        int account_index2;
        double value;

        if (code == 'D') {
            account_index = find_account_num(line_buffer.command_list[1], accounts);
            pthread_mutex_lock(&(accounts[account_index].ac_lock));
            if (strcmp(accounts[account_index].password, line_buffer.command_list[2]) == 0) {
                char *endptr;
                value = strtod(line_buffer.command_list[3], &endptr);
                accounts[account_index].balance += value;
                accounts[account_index].transaction_tracter += value;
                update_times[account_index]++; // Increment update times
            }
            pthread_mutex_unlock(&(accounts[account_index].ac_lock));
        } else if (code == 'W') {
            account_index = find_account_num(line_buffer.command_list[1], accounts);
            pthread_mutex_lock(&(accounts[account_index].ac_lock));
            if (strcmp(accounts[account_index].password, line_buffer.command_list[2]) == 0) {
                char *endptr;
                value = strtod(line_buffer.command_list[3], &endptr);
                accounts[account_index].balance -= value;
                accounts[account_index].transaction_tracter += value;
                update_times[account_index]++; // Increment update times
            }
            pthread_mutex_unlock(&(accounts[account_index].ac_lock));
        } else if (code == 'T') {
            account_index = find_account_num(line_buffer.command_list[1], accounts);
            account_index2 = find_account_num(line_buffer.command_list[3], accounts);
            if (account_index < account_index2) {
                pthread_mutex_lock(&(accounts[account_index].ac_lock));
                pthread_mutex_lock(&(accounts[account_index2].ac_lock));
            } else {
                pthread_mutex_lock(&(accounts[account_index2].ac_lock));
                pthread_mutex_lock(&(accounts[account_index].ac_lock));
            }
            if (strcmp(accounts[account_index].password, line_buffer.command_list[2]) == 0) {
                char *endptr;
                value = strtod(line_buffer.command_list[4], &endptr);
                accounts[account_index].balance -= value;
                accounts[account_index].transaction_tracter += value;
                accounts[account_index2].balance += value;
                update_times[account_index]++; // Increment update times for both accounts
                update_times[account_index2]++;
            }
            pthread_mutex_unlock(&(accounts[account_index].ac_lock));
            pthread_mutex_unlock(&(accounts[account_index2].ac_lock));
        } else if (code == 'C') {
            // Check balance operation, nothing to do here.
        }

        free_command_line(&line_buffer);
    }

    return NULL;
}

void *update_balance(void *arg) {
    for (int i = 0; i < num_accounts; ++i) {
        pthread_mutex_lock(&(accounts[i].ac_lock));
        accounts[i].balance += (accounts[i].transaction_tracter * accounts[i].reward_rate);
        pthread_mutex_unlock(&(accounts[i].ac_lock));
    }

    return NULL;
}

int find_account_num(char *account_num, account *accounts) {
    for (int i = 0; i < num_accounts; ++i) {
        if (strcmp(account_num, accounts[i].account_number) == 0) {
            return i;
        }
    }

    return -1;
}
