#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "tokens.h"

static long get_file_size(FILE *file);

// Exit status used by lexer when invalid tokens were encountered
#define LEXER_INVALID_TOKENS 3

static int lexer(const char *source, const char *filename);

typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
} TokenTable;

static void table_init(TokenTable *table) {
    table->tokens = NULL;
    table->count = 0;
    table->capacity = 0;
}

// Appends a token to the table in encounter order.
static int table_append(TokenTable *table, const Token token) {
    if (table->count == table->capacity) {
        const size_t new_capacity = table->capacity == 0 ? 64 : table->capacity * 2;
        Token *grown = realloc(table->tokens, new_capacity * sizeof(Token));

        if (!grown) {
            return 0;
        }

        table->tokens = grown;
        table->capacity = new_capacity;
    }

    table->tokens[table->count++] = token;

    return 1;
}

static void table_free(TokenTable *table) {
    for (size_t i = 0; i < table->count; i++) {
        if (table->tokens[i].type == TOKEN_STRING_LIT) {
            free(table->tokens[i].string_val);
        }
    }

    free(table->tokens);
    table_init(table);
}


static void print_token(const Token *t) {
    printf("Line %d Column %d - %s", t->line, t->column, token_type_name(t->type));

    if (t->type != TOKEN_EOF) {
        printf(" '%.*s'", t->length, t->lexeme);
    }

    switch (t->type) {
        case TOKEN_INT_LIT: printf(" value=%d", t->int_val); break;
        case TOKEN_DOUBLE_LIT: printf(" value=%g", t->double_val); break;
        case TOKEN_BOOL_LIT: printf(" value=%s", t->bool_val ? "true" : "false"); break;
        case TOKEN_CHAR_LIT: printf(" value=%d", (unsigned char) t->char_val); break;
        case TOKEN_STRING_LIT: printf(" value=\"%s\"", t->string_val); break;
        default: break;
    }

    printf("\n");
}

static void report_invalid_token(const char *filename, const Token *t) {
    fprintf(stderr, "%s:%d:%d: error: %s: '%.*s'\n", filename, t->line, t->column,
            t->error ? t->error : "invalid token", t->length, t->lexeme);
}

static void write_csv_text(FILE *out, const char *text, const int length) {
    fputc('"', out);

    for (int i = 0; i < length; i++) {
        if (text[i] == '"') {
            fputc('"', out);
        }
        fputc(text[i], out);
    }

    fputc('"', out);
}

static int flush_table(const TokenTable *table, const char *path) {
    FILE *out = fopen(path, "w");
    if (!out) {
        perror(path);

        return 0;
    }

    fprintf(out, "index,line,column,type,lexeme,value,error\n");

    for (size_t i = 0; i < table->count; i++) {
        const Token *t = &table->tokens[i];

        fprintf(out, "%zu,%d,%d,%s,", i, t->line, t->column, token_type_name(t->type));
        write_csv_text(out, t->lexeme, t->length);
        fputc(',', out);

        switch (t->type) {
            case TOKEN_INT_LIT: fprintf(out, "%d", t->int_val); break;
            case TOKEN_DOUBLE_LIT: fprintf(out, "%.17g", t->double_val); break;
            case TOKEN_BOOL_LIT: fprintf(out, "%s", t->bool_val ? "true" : "false"); break;
            case TOKEN_CHAR_LIT: fprintf(out, "%d", (unsigned char) t->char_val); break;
            case TOKEN_STRING_LIT: write_csv_text(out, t->string_val, (int) strlen(t->string_val)); break;
            default: break;
        }

        fputc(',', out);

        if (t->type == TOKEN_INVALID && t->error) {
            write_csv_text(out, t->error, (int) strlen(t->error));
        }

        fputc('\n', out);
    }

    if (fclose(out) != 0) {
        perror(path);

        return 0;
    }

    return 1;
}

// Set when --debug is passed
static int debug_enabled = 0;

int main(int argc, char *argv[]) {
    debug_output_stream = stderr;

    if (argc <= 1) {
        fprintf(stderr, "USAGE: jvlex [filename ...]\n");

        return 1;
    }

    int debug_arg_index = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            printf("Debug logging enabled. Debug logs will be printed in stdout.\n");

            debug_arg_index = i;
            debug_enabled = 1;
            debug_output_stream = stdout;
            break;
        }
    }

    FILE *null_stream = NULL;
    if (debug_arg_index == -1) {
        null_stream = fopen("/dev/null", "w");
        if (null_stream) {
            debug_output_stream = null_stream;
        }
    }

    int exit_status = 0;

    for (int i = 1; i < argc; i++) {
        if (i == debug_arg_index) { continue; }

        const char *filename = argv[i];
        const char *filetype = NULL;

        int filename_length = 0;
        while (*filename++ != '\0') {
            filename_length++;
        }

        if (filename_length < 5) {
            fprintf(stderr, "This lexer only recognizes Javelin source code (.jvln).\n");

            return 2;
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

        if (debug_enabled) {
            printf("Processing file %s...\n", argv[i]);
        }

        long file_size = get_file_size(input_file);

        char *buffer = malloc(file_size + 1);

        size_t bytes_read = fread(buffer, 1, file_size, input_file); // Read the file into a contiguous memory block
        buffer[bytes_read] = '\0'; // Add a null terminator at the end as our sentinel

        const int status = lexer(buffer, argv[i]);
        if (status > exit_status) {
            exit_status = status;
        }

        free(buffer);
        fclose(input_file);
    }

    if (null_stream) {
        fclose(null_stream);
    }

    return exit_status;
}

long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    return size;
}

int lexer(const char *source, const char *filename) {
    LexerState l;
    lexer_init(&l, source);
    TokenTable table;
    table_init(&table);
    size_t invalid_count = 0;

    for (;;) {
        const Token token = next_token(&l);

        if (debug_enabled) {
            print_token(&token);
        }

        if (token.type == TOKEN_EOF) {
            break;
        }

        if (token.type == TOKEN_INVALID) {
            report_invalid_token(filename, &token);
            invalid_count++;
        }

        if (!table_append(&table, token)) {
            fprintf(stderr, "Out of memory while recording tokens.\n");

            if (token.type == TOKEN_STRING_LIT) {
                free(token.string_val);
            }

            break;
        }
    }

    const size_t path_length = strlen(filename) + strlen(".symb") + 1;
    char *symb_path = malloc(path_length);
    int status = 0;

    if (symb_path) {
        snprintf(symb_path, path_length, "%s.symb", filename);

        if (flush_table(&table, symb_path)) {
            printf("Wrote %zu tokens to %s\n", table.count, symb_path);
        } else {
            status = 1;
        }

        free(symb_path);
    } else {
        status = 1;
    }

    table_free(&table);

    if (invalid_count > 0) {
        fprintf(stderr, "%s: %zu invalid token%s found.\n", filename, invalid_count, invalid_count == 1 ? "" : "s");

        if (status == 0) {
            status = LEXER_INVALID_TOKENS;
        }
    }

    return status;
}

