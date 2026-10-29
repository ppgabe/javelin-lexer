#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long get_file_size(FILE *file);

static int lexer(char *buffer);

static void get_ident_or_reserved_word(char *cursor);
static FILE *debug_output_stream = NULL;

int main(int argc, char *argv[]) {
    debug_output_stream = stderr;

    if (argc <= 1) {
        fprintf(stderr, "USAGE: javelin_lexer [filename ...]\n");

        return 1;
    }

    int debug_arg_index = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            printf("Debug logging enabled. Debug logs will be printed in stdout.\n");

            debug_arg_index = i;
            debug_output_stream = stdout;
            break;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (i == debug_arg_index) { continue; }

        const char *filename = argv[i];
        const char *filetype = NULL;

        int filename_length = 0;
        while (*filename++ != '\0') {
            filename_length++;
        }

        filetype = argv[i] + (filename_length - 5);

        if (filename_length < 5 || (filetype - argv[i] >= 0 && strcmp(filetype, ".jvln") != 0)) {
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
    int line = 1;
    int column = 1;

    while (*cursor != '\0') {
        switch (*cursor) {
            case ' ':
            case '\t':
            case '\r':
                fprintf(debug_output_stream, "Skipped whitespace at line %d, column %d...\n", line, ++column);
                cursor++;
                continue;
            case '\n':
                fprintf(debug_output_stream, "New line at line %d, column %d...\n", line++, ++column);
                column = 1;
                cursor++;
                continue;
            // Skip all whitespace characters
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
