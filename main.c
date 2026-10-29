#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long get_file_size(FILE *file);

static int lexer(char *buffer);

static void get_ident_or_reserved_word(char *cursor);

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "USAGE: javelin_lexer [filename ...]\n");

        return 1;
    }

    for (int i = 1; i < argc; i++) {
        const char* filename = argv[i];
        const char* filetype = NULL;

        int filename_length = 0;
        while (*filename++ != '\0') {
            filename_length++;
        }

        filetype = argv[i] + (filename_length - 5);

        if (strcmp(filetype, ".jvln") != 0) {
            fprintf(stderr, "This lexer only recognizes Javelin source code (.jvln).\n");

            return 2;
        }

        // Open the file as read-only in binary mode
        FILE *input_file = fopen(argv[i], "rb");
        if (!input_file) {
            perror(argv[i]);
            continue;
        }

        printf("Processing file %s...\n", argv[i]);

        long file_size = get_file_size(input_file);

        char *buffer = malloc(file_size + 1);

        fread(buffer, 1, file_size, input_file); // Read the file into a contiguous memory block
        buffer[file_size] = '\0'; // Add a null terminator at the end as our sentinel

        lexer(buffer);

        free(buffer);
        fclose(input_file);
    }

    return 0;
}

long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    return size;
}

int lexer(char *buffer) {
    char *cursor = buffer;

    while (*cursor != '\0') {
        switch (*cursor) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                cursor++;
                continue; // Skip all whitespace characters
            default:
                if (isalpha(*cursor)) {
                    get_ident_or_reserved_word(cursor);
                }
        }

        cursor++;
    }
}

void get_ident_or_reserved_word(char *cursor) {
    // TODO : Implement Trie matching
}
