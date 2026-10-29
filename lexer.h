#ifndef JAVELIN_LEXER_LEXER_H
#define JAVELIN_LEXER_LEXER_H

#include <stdio.h>
#include "tokens.h"

typedef struct {
    const char *source;
    const char *cursor;
    int line;
    int column;
    int previous_column;
} LexerState;

// Stream that receives the lexer's debug logs (whitespace skipping), defaulting to stderr.
extern FILE *debug_output_stream;

// Prepares `l` to scan the null-terminated `source` from its first character
void lexer_init(LexerState *l, const char *source);

// Scans and returns the next token, returning TOKEN_EOF when the end of the source is reached.
Token next_token(LexerState *l);

#endif //JAVELIN_LEXER_LEXER_H
