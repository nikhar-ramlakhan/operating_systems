#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "command.h"
#include "string_parser.h"

void commands_list(char **tokens, int num_tokens) {
    if (num_tokens == 0) {
        return;  // No tokens to execute.
    }

    // Check the first token to determine which command to execute.
    char *cmd = tokens[0];

    if (strcmp(cmd, "ls") == 0) {
        if (num_tokens > 1) {
            const char *err_ls = "Error: Too many parameters for ls\n";
            write(1, err_ls, strlen(err_ls));
        } else {
            listDir();
        }
    } else if (strcmp(cmd, "pwd") == 0) {
        if (num_tokens > 1) {
            const char *err_pwd = "Error: Too many parameters for pwd\n";
            write(1, err_pwd, strlen(err_pwd));
        } else {
            showCurrentDir();
        }
    } else if (strcmp(cmd, "mkdir") == 0) {
        if (num_tokens != 2) {
            const char *err_mkdir = "Error: Incorrect number of parameters for mkdir\n";
            write(1, err_mkdir, strlen(err_mkdir));
        } else {
            makeDir(tokens[1]);
        }
    } else if (strcmp(cmd, "cd") == 0) {
        if (num_tokens != 2) {
            const char *err_cd = "Error: Incorrect number of parameters for cd\n";
            write(1, err_cd, strlen(err_cd));
        } else {
            changeDir(tokens[1]);
        }
    } else if (strcmp(cmd, "cp") == 0) {
        if (num_tokens != 3) {
            const char *err_cp = "Error: Incorrect number of parameters for cp\n";
            write(1, err_cp, strlen(err_cp));
        } else {
            copyFile(tokens[1], tokens[2]);
        }
    } else if (strcmp(cmd, "mv") == 0) {
        if (num_tokens != 3) {
            const char *err_mv = "Error: Incorrect number of parameters for mv\n";
            write(1, err_mv, strlen(err_mv));
        } else {
            moveFile(tokens[1], tokens[2]);
        }
    } else if (strcmp(cmd, "rm") == 0) {
        if (num_tokens != 2) {
            const char *err_rm = "Error: Incorrect number of parameters for rm\n";
            write(1, err_rm, strlen(err_rm));
        } else {
            deleteFile(tokens[1]);
        }
    } else if (strcmp(cmd, "cat") == 0) {
        if (num_tokens != 2) {
            const char *err_cat = "Error: Incorrect number of parameters for cat\n";
            write(1, err_cat, strlen(err_cat));
        } else {
            displayFile(tokens[1]);
        }
    } else {
        const char *err_unrecognized = "Error: Unrecognized command\n";
        write(1, err_unrecognized, strlen(err_unrecognized));
    }
}

void execute_file_mode(const char *filename) {
    FILE *file_input = freopen(filename, "r", stdin);
    FILE *file_output = freopen("output.txt", "w+", stdout);

    if (file_input == NULL || file_output == NULL) {
        const char *error_file = "Error: Could not open file\n";
        write(1, error_file, strlen(error_file));
        return;
    }

    char *line_buffer = NULL;
    size_t buffer_length = 0;
    ssize_t read_result;
    command_line cmd_line;
    command_line arg_line;

    while ((read_result = getline(&line_buffer, &buffer_length, file_input)) != -1) {
        if (line_buffer[read_result - 1] == '\n') {
            line_buffer[read_result - 1] = '\0';
        }
        cmd_line = str_filler(line_buffer, ";");
        for (int i = 0; cmd_line.command_list[i] != NULL; i++) {
            arg_line = str_filler(cmd_line.command_list[i], " ");
            commands_list(arg_line.command_list, arg_line.num_token);
            free_command_line(&arg_line);
            memset(&arg_line, 0, 0);
        }
        free_command_line(&cmd_line);
        memset(&cmd_line, 0, 0);
    }

    fclose(file_input);
    fclose(file_output);
    if (line_buffer) {
        free(line_buffer);
    }
}

void execute_interactive_mode() {
    char *line = NULL;
    size_t len = 0;
    ssize_t re;
    command_line large_token_buffer;
    command_line small_token_buffer;

    while (1) {
        write(1, ">>> ", strlen(">>> "));
        re = getline(&line, &len, stdin);

        if (line[re - 1] == '\n') {
            line[re - 1] = '\0';
        }

        if (strcmp(line, "exit") == 0) {
            break;
        }

        large_token_buffer = str_filler(line, ";");
        for (int i = 0; large_token_buffer.command_list[i] != NULL; i++) {
            small_token_buffer = str_filler(large_token_buffer.command_list[i], " ");
            commands_list(small_token_buffer.command_list, small_token_buffer.num_token);
            free_command_line(&small_token_buffer);
            memset(&small_token_buffer, 0, 0);
        }
        free_command_line(&large_token_buffer);
        memset(&large_token_buffer, 0, 0);

    }
    free(line);
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-f") == 0 && argc > 2) {
        execute_file_mode(argv[2]);
    } else {
        execute_interactive_mode();
    }

    return 0;
}

