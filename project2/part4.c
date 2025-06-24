#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "string_parser.h"

#define MAX_PROC_INFO_LENGTH 1024

void the_alarm(int signo);
void print_stats();

int process_cnt = 0;
int process_num = 0;
pid_t * children;

int main(int argc, char *argv[]) {
    int opt;
    char *input_filename = NULL;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                input_filename = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s -f <input_file>\n", argv[0]);
                return 1;
        }
    }

    if (input_filename == NULL) {
        fprintf(stderr, "Usage: %s -f <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        perror("Error opening input file");
        return 1;
    }

    char *line = NULL;
    size_t line_length = 0;
    int line_number = 0;

    // Calculate the number of lines in the file
    int line_count = 0;
    char ch[1024];
    while (fgets(ch, sizeof(ch), input_file) != NULL) {
        line_count++;
    }
    rewind(input_file); // Reset the file pointer to the beginning
    children = malloc(sizeof(pid_t) * line_count);

    // Read commands from input file
    while (getline(&line, &line_length, input_file) != -1) {
        command_line buffer;
        buffer = str_filler(line, " ");

        // Fork a child process
        pid_t pid = fork();

        children[process_cnt++] = pid;

        if (pid < 0) {
            perror("Error forking process");
            return 1;
        } else if (pid == 0) {  // Child process
            // Execute the command
            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGCONT);

            sigprocmask(SIG_BLOCK, &mask, NULL);

            int sig;
            int error = sigwait(&mask, &sig);

            if (error != 0) {
                perror("Error waiting for signal");
                exit(EXIT_FAILURE);
            }
            
            if (execvp(buffer.command_list[0], buffer.command_list) == -1) {
                perror("Error executing command");
                free(children);
                free(line);
                free_command_line(&buffer);
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
        } 

        free_command_line(&buffer);
        line_number++;
    }

    if (signal(SIGALRM, the_alarm) == SIG_ERR) {
        perror("Error setting alarm signal");
        exit(-1);
    }

    alarm(1); // Set the alarm for 1 seconds 

    int status;
    pid_t wpid;

    while ((wpid = waitpid(-1, &status,0)) > 0){
        if (WIFEXITED(status)) {
            printf("Child %d terminated with status: %d\n", wpid, WEXITSTATUS(status));
        }
    
    } // Wait for all child processes to finish

    free(children);
    free(line);
    fclose(input_file);

    return 0;
}

void the_alarm(int signo) {
    kill (children[process_num], SIGSTOP);

    process_num = (process_num + 1) % process_cnt;
    
    for (int i = process_num; i < (process_num + process_cnt + 1); i++) {
        int running = kill(children[i], 0);
        if (running == 0) {
            kill(children[process_num], SIGCONT);
            break;
        }
        else {
            process_num = (process_num + 1) % process_cnt;
        }
    }
    alarm(1); // Set the alarm for 1 seconds (repeating alarm)
    print_stats();
}   

void print_stats(){
    system("clear");
    printf("PID\t\tState\t\tUser Time\tKernel Time\tPriority\tThreads\n");
    // Loop through child processes

    for (int i = 0; i < process_cnt; i++) {
        // Open corresponding /proc/<pid>/stat file
        char stat_file_path[256];
        snprintf(stat_file_path, sizeof(stat_file_path), "/proc/%d/stat", children[i]);
        FILE *stat_file = fopen(stat_file_path, "r");
        if (stat_file == NULL) {
            
            continue;
        }

        // Read and interpret relevant data
        char proc_info[MAX_PROC_INFO_LENGTH];
        if (fgets(proc_info, MAX_PROC_INFO_LENGTH, stat_file) != NULL) {
            // Split the line into tokens using space as delimiter
            char *token = strtok(proc_info, " ");
            int index = 1;
            while (token != NULL) {
                // Print the token along with its index
                if (index == 1 || index == 3 || index == 14 || index == 15 || index == 18 || index == 20) {
                    printf("%s\t\t", token );
                }
                
                // Move to the next token
                token = strtok(NULL, " ");
                index++;
            }
        }

        printf("\n");


        // Close stat file
        fclose(stat_file);
    }
}