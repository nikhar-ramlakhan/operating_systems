#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "string_parser.h"

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
    int process_cnt = 0;

    // Calculate the number of lines in the file
    int line_count = 0;
    char ch[1024];
    while (fgets(ch, sizeof(ch), input_file) != NULL) {
        line_count++;
    }
    rewind(input_file); // Reset the file pointer to the beginning
    pid_t *children = (pid_t *)malloc(sizeof(pid_t) * line_count);

    // Read commands from input file
    while (getline(&line, &line_length, input_file) != -1) {
        command_line buffer;
        buffer = str_filler(line, " ");

        // Fork a child process
        children[process_cnt] = fork();

        if (children[process_cnt] < 0) {
            perror("Error forking process");
            return 1;
        } else if (children[process_cnt] == 0) {  // Child process
            // Execute the command
            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGUSR1);

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
        process_cnt++; 
        free_command_line(&buffer);
        line_number++;
    }

    fprintf(stderr, "Sending SIGUSER1...\n");
    for (int i = 0; i < process_cnt; i++) {
        kill(children[i], SIGUSR1);
    }

    
    sleep(1);

    fprintf(stderr, "Sending SIGSTOP...\n");
    for (int i = 0; i < process_cnt; i++) {
        kill(children[i], SIGSTOP);
    }

    sleep(1);

    fprintf(stderr, "Sending SIGCONT...\n");
    for (int i = 0; i < process_cnt; i++) {
        kill(children[i], SIGCONT); 
    }

    int status;
    for (int i = 0; i < process_cnt; i++) {
        waitpid(children[i], &status, 0);
    }

    free(children);
    free(line);
    fclose(input_file);

    return 0;
}