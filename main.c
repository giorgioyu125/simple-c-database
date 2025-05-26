// Includes

#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli.h"
#include "hashtable.h"

// Constants

#define MAX_LINE_SIZE 1024

// Functionality

int read_line(char* buffer, int buffer_size) {
    if (buffer == NULL) {
        #if DEBUG_MODE
            printf("[ERROR] read_line: buffer is null.\n");
        #endif

        return EXIT_FAILURE;
    }

    if (fgets(buffer, buffer_size, stdin) == NULL) {
        buffer[0] = '\0';
        return EXIT_FAILURE;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    return EXIT_SUCCESS;
}

int main(void) {
    char line[MAX_LINE_SIZE] = {0};
    time_t now = time(NULL);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("Simple C Database - Started at %s\n", time_str);
    printf("Listening for commands... (Type 'HELP')\n");
    printf("Simple Key-Value DB Input (type 'quit' to exit)\n");

    size_t db_size = 0;
    void* out_db = NULL;
    char* db_type = NULL;
    
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
        
        process_command(line, db_type, db_size, out_db);

        memset(line, 0, sizeof(line));
    }

    return EXIT_SUCCESS;
}
