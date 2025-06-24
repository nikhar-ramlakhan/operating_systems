#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
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

    // Read commands from input file
    while (getline(&line, &line_length, input_file) != -1) {
        command_line buffer;
        buffer = str_filler(line, " ");

        // Fork a child process
        pid_t pid = fork();

        if (pid < 0) {
            perror("Error forking process");
            return 1;
        } else if (pid == 0) {  // Child process
            // Execute the command
            if (execvp(buffer.command_list[0], buffer.command_list) == -1) {
                perror("Error executing command");
                free(line);
                free_command_line(&buffer);
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
        } 
        
        free_command_line(&buffer);
        line_number++;
    }

    while (wait(NULL) > 0);  // Wait for all child processes to finish

    free(line);
    fclose(input_file);

    return 0;
}
