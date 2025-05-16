// Includes

#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli.c"

// Defines

#define MAX_LINE_SIZE 1024

// Data


// Functionality

int read_line(char* buffer, int buffer_size) {
    if (fgets(buffer, buffer_size, stdin) == NULL) {
        buffer[0] = '\0';
        return EXIT_FAILURE;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    return EXIT_SUCCESS;
}


int main(void) {
    char line[MAX_LINE_SIZE] = {0};
    printf("Simple Key-Value DB Input (type 'quit' to exit)\n");

    while (true) {       
        printf("> ");
        fflush(stdout);
        
        int error = read_line(line, MAX_LINE_SIZE);

        if (error != EXIT_SUCCESS || error == EOF) {
            fprintf(stderr, "[ERROR] read_line (main): An error occured while calling read line for user input collection.\n");
            return EXIT_FAILURE;
        }

        for (int i = 0; i < (int)strlen(line); i++) {
            line[i] = (char)toupper(line[i]);
            if (line[i] == ' '){
                break;
            }
        }
        
        process_command(line);

        memset(line, 0, sizeof(line));
    }

    return EXIT_SUCCESS;
}
