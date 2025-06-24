// CS 415
// Project 3 | Part 1
// Nikhar Ramlakhan
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "string_parser.h"
#include "account.h"

// Function declarations
void * process_transaction(void *args);
void * update_balance(void *arg);
int find_account_num(char *account_num, account *accounts);

account *accounts;
int num_accounts;
int line = 0;
int *update_times; // Global variable to track update times

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
        // index
        getline(&input, &len_input, f);
        accounts[i].transaction_tracter = 0.0;

        // account num
        getline(&input, &len_input, f);
        strncpy(accounts[i].account_number, input, sizeof(accounts[i].account_number) - 1);
        accounts[i].account_number[sizeof(accounts[i].account_number) - 1] = '\0';

        // password
        getline(&input, &len_input, f);
        strncpy(accounts[i].password, input, sizeof(accounts[i].password) - 1);
        accounts[i].password[sizeof(accounts[i].password) - 1] = '\0';

        // balance
        getline(&input, &len_input, f);
        accounts[i].balance = strtod(input, NULL);

        // reward rate 
        getline(&input, &len_input, f);
        accounts[i].reward_rate = strtod(input, NULL);

        update_times[i] = 0; // Initialize update times
    }

    while ((read_bytes = getline(&input, &len_input, f)) != -1) {
        command_line line_buffer = str_filler(input, " ");
        process_transaction(&line_buffer);
        free_command_line(&line_buffer);
    }

    update_balance(NULL);
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
    free(update_times); // Free the allocated memory for update times
    fclose(q);

    return 0;
}


void * process_transaction(void *args) {
    command_line *buffer = (command_line *)args;
    char code = buffer->command_list[0][0];

    int account_index;
    int account_index2;
    double value;

    if (code == 'D'){
        account_index = find_account_num(buffer->command_list[1], accounts);
        if (strcmp(accounts[account_index].password, buffer->command_list[2]) == 0){
            char *endptr;
            value = strtod(buffer->command_list[3], &endptr);
            accounts[account_index].balance = accounts[account_index].balance + value;
            accounts[account_index].transaction_tracter = accounts[account_index].transaction_tracter + value;
            update_times[account_index]++; // Increment update times
        } else{
            // printf("wrong password: [%d]\n", line);
            return NULL;
        }
    } else if (code == 'W'){
        account_index = find_account_num(buffer->command_list[1], accounts);
        if (strcmp(accounts[account_index].password, buffer->command_list[2]) == 0){
            char *endptr;
            value = strtod(buffer->command_list[3], &endptr);
            accounts[account_index].balance = accounts[account_index].balance - value;
            accounts[account_index].transaction_tracter = accounts[account_index].transaction_tracter + value;
            update_times[account_index]++; // Increment update times
        } else{
            // printf("wrong password: [%d]\n", line);
            return NULL;
        }
    } else if (code == 'T'){
        account_index = find_account_num(buffer->command_list[1], accounts);
        account_index2 = find_account_num(buffer->command_list[3], accounts);
        if (strcmp(accounts[account_index].password, buffer->command_list[2]) == 0){
            char *endptr;
            value = strtod(buffer->command_list[4], &endptr);
            accounts[account_index].balance = accounts[account_index].balance - value;
            accounts[account_index].transaction_tracter = accounts[account_index].transaction_tracter + value;
            accounts[account_index2].balance = accounts[account_index2].balance + value;
            update_times[account_index]++; // Increment update times for both accounts
            update_times[account_index2]++;
        } else{
            // printf("wrong password: [%d]\n", line);
            return NULL;
        }
    } else if (code == 'C'){
        return NULL;
    }

    return NULL;
}

void * update_balance(void *arg) {
    for (int i = 0; i < num_accounts; ++i) {
        accounts[i].balance = accounts[i].balance + (accounts[i].transaction_tracter * accounts[i].reward_rate);
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
