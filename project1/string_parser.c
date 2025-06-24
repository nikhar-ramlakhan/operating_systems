/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan, Monil
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"

#define _GUN_SOURCE

int count_token (char* buf, const char* delim)
{

	/*
	*	#1.	Check for NULL string
	*	#2.	iterate through string counting tokens
	*		Cases to watchout for
	*			a.	string start with delimeter
	*			b. 	string end with delimeter
	*			c.	account NULL for the last token
	*	#3. return the number of token (note not number of delimeter)
	*/

    // Check for NULL string or delimiter
    if (buf == NULL || delim == NULL) {
        return 0;
    }

    char * save_ptr = NULL;
    int count = 0;
    char* token = strtok_r(buf, delim, &save_ptr);

    // Count tokens
    while (token != NULL) {
        count++;
        token = strtok_r(NULL, delim, &save_ptr);
    }

    return count;
}

command_line str_filler (char* buf, const char* delim)
{
    /*
    *   #1. create command_line variable to be filled and returned
    *   #2. count the number of tokens with count_token function, set num_token. 
    *           one can use strtok_r to remove the \n at the end of the line.
    *   #3. malloc memory for token array inside command_line variable
    *           based on the number of tokens.
    *   #4. use function strtok_r to find out the tokens 
    *   #5. malloc each index of the array with the length of tokens,
    *           fill command_list array with tokens, and fill last spot with NULL.
    *   #6. return the variable.
    */
    // Remove newline character if present and make a copy of the string
    char * newline = NULL;
    char * newline_removed = strtok_r(buf, "\n", &newline);
    char * copy_buf = strdup(newline_removed);

    // Check if memory allocation for the copy is successful
    if (copy_buf == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Initialize a command_line struct to store tokens
    command_line result;
    result.num_token = count_token(copy_buf, delim);

    // Allocate memory for token array inside command_line variable based on the number of tokens
    result.command_list = malloc((result.num_token + 1) * sizeof(char*));
    if (result.command_list == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    char * save_ptr = NULL;
    char* token = strtok_r(buf, delim, &save_ptr);
    int i = 0;

    // Tokenize the string and store tokens in command_list array
    while (token != NULL) {
        // Allocate memory for each token and copy it into the array
        result.command_list[i] = malloc((strlen(token) + 1) * sizeof(char));
        if (result.command_list[i] == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        strcpy(result.command_list[i], token);
        i++;
        token = strtok_r(NULL, delim, &save_ptr);
    }
    // Set the last element of the array to NULL to indicate the end
    result.command_list[i] = NULL;

    // Free the copy of the string
    free (copy_buf);
    return result;
}


void free_command_line(command_line* command)
{
	/*
	*	#1.	free the array base num_token
	*/

    // Check for NULL command or command_list
    if (command == NULL || command->command_list == NULL) {
        return;
    }

    // Free the memory allocated for each token in the command_list array
    for (int i = 0; i < command->num_token; i++) {
        free(command->command_list[i]);
    }
    // Free the memory allocated for the command_list array itself
    free(command->command_list);
}
