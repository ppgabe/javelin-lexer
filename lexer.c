#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

FILE *debug_output_stream = NULL;

static int is_ident_char(char ch);

static Token make_token(const LexerState *l, TokenType type, const char *start, int line, int col);
static Token make_invalid_token(const LexerState *l, const char *start, int line, int col, const char *error);
static Token make_int_token(const LexerState *l, const char *start, int line, int col, int value);
static Token make_double_token(const LexerState *l, const char *start, int line, int col, double value);
static Token make_bool_token(const LexerState *l, const char *start, int line, int col, bool value);
static Token make_char_token(const LexerState *l, const char *start, int line, int col, char value);
static Token make_string_token(const LexerState *l, const char *start, int line, int col, char *value);

static Token scan_identifier(LexerState *l);
static Token scan_comment(LexerState *l);
static Token scan_number(LexerState *l);
static Token scan_char(LexerState *l);
static Token scan_string(LexerState *l);
static Token scan_operator(LexerState *l);

void lexer_init(LexerState *l, const char *source) {
    *l = (LexerState){.source = source, .cursor = source, .line = 1, .column = 1};
}

static char peek(const LexerState *l) {
    return *l->cursor;
}

static char advance(LexerState *l) {
    char c = *l->cursor++;

    if (c == '\n') {
        l->line++;
        l->column = 1;
    } else {
        l->column++;
    }

    return c;
}

static void skip_whitespace(LexerState *l) {
    while (isspace((unsigned char) peek(l))) {
        advance(l);
        fprintf(debug_output_stream, "Line %d Column %d - Skipped whitespace\n", l->line, l->column);
    }
}

Token next_token(LexerState *l) {
    skip_whitespace(l);

    const char start = peek(l);
    const int start_col = l->column;

    if (start == '\0') {
        return (Token){ .type = TOKEN_EOF, .line = l->line, .column = start_col };
    }

    if (start == '#') {
        return scan_comment(l);
    }

    if (isalpha((unsigned char) start) || start == '_') {
        return scan_identifier(l);
    }

    if (isdigit((unsigned char) start)) {
        return scan_number(l);
    }

    if (start == '\'') {
        return scan_char(l);
    }

    if (start == '"') {
        return scan_string(l);
    }

    return scan_operator(l);
}

Token scan_comment(LexerState *l) {
    const char *start = l->cursor;
    const int start_line = l->line;
    const int start_col = l->column;

    while (peek(l) != '\n' && peek(l) != '\0') {
        advance(l);
    }

    fprintf(debug_output_stream, "Line %d Column %d - Scanned comment\n", start_line, start_col);

    return make_token(l, TOKEN_COMMENT, start, start_line, start_col);
}

Token make_token(const LexerState *l, const TokenType type, const char *start, const int line, const int col) {
    return (Token){
        .type = type,
        .lexeme = start,
        .length = (int)(l->cursor - start),
        .line = line,
        .column = col
    };
}

Token make_invalid_token(const LexerState *l, const char *start, const int line, const int col, const char *error) {
    Token token = make_token(l, TOKEN_INVALID, start, line, col);
    token.error = error;

    return token;
}

Token make_int_token(const LexerState *l, const char *start, const int line, const int col, const int value) {
    Token token = make_token(l, TOKEN_INT_LIT, start, line, col);
    token.int_val = value;

    return token;
}

Token make_double_token(const LexerState *l, const char *start, const int line, const int col, const double value) {
    Token token = make_token(l, TOKEN_DOUBLE_LIT, start, line, col);
    token.double_val = value;

    return token;
}

Token make_bool_token(const LexerState *l, const char *start, const int line, const int col, const bool value) {
    Token token = make_token(l, TOKEN_BOOL_LIT, start, line, col);
    token.bool_val = value;

    return token;
}

Token make_char_token(const LexerState *l, const char *start, const int line, const int col, const char value) {
    Token token = make_token(l, TOKEN_CHAR_LIT, start, line, col);
    token.char_val = value;

    return token;
}

Token make_string_token(const LexerState *l, const char *start, const int line, const int col, char *value) {
    Token token = make_token(l, TOKEN_STRING_LIT, start, line, col);
    token.string_val = value;

    return token;
}

Token scan_identifier(LexerState *l) {
    const char *start = l->cursor;
    const int start_line = l->line;
    const int start_col = l->column;

    switch (advance(l)) { // State START
        case 'a': // State I_A
            if (peek(l) == 'n') { // State I_AN
                advance(l);
                if (peek(l) == 'd') { // State I_AND
                    advance(l);
                    if (!is_ident_char(peek(l))) { // Rule 2: AND *
                        return make_token(l, TOKEN_OP_AND, start, start_line, start_col);
                    }
                }
            } else if (peek(l) == 's') { // State I_AS
                advance(l);
                if (peek(l) == '?') { // State I_ASQ
                    advance(l);
                    return make_token(l, TOKEN_KW_SAFE_AS, start, start_line, start_col);
                }

                if (!is_ident_char(peek(l))) { // Rule 2: AS *
                    return make_token(l, TOKEN_KW_AS, start, start_line, start_col);
                }
            }
            break;

        case 'b': // State I_B
            if (peek(l) == 'o') { // State I_BO
                advance(l);
                if (peek(l) == 'o') { // State I_BOO
                    advance(l);
                    if (peek(l) == 'l') { // State I_BOOL
                        advance(l);
                        if (!is_ident_char(peek(l))) { // Rule 2: BOOL *
                            return make_token(l, TOKEN_RW_BOOL, start, start_line, start_col);
                        }
                    }
                }
            } else if (peek(l) == 'r') { // State I_BR
                advance(l);
                if (peek(l) == 'e') { // State I_BRE
                    advance(l);
                    if (peek(l) == 'a') { // State I_BREA
                        advance(l);
                        if (peek(l) == 'k') { // State I_BREAK
                            advance(l);
                            if (!is_ident_char(peek(l))) { // Rule 2: BREAK *
                                return make_token(l, TOKEN_KW_BREAK, start, start_line, start_col);
                            }
                        }
                    }
                }
            }
            break;

        case 'c': // State I_C
            if (peek(l) == 'h') { // State I_CH
                advance(l);
                if (peek(l) == 'a') { // State I_CHA
                    advance(l);
                    if (peek(l) == 'r') { // State I_CHAR
                        advance(l);
                        if (!is_ident_char(peek(l))) { // Rule 2: CHAR *
                            return make_token(l, TOKEN_RW_CHAR, start, start_line, start_col);
                        }
                    }
                }
            } else if (peek(l) == 'o') { // State I_CO
                advance(l);
                if (peek(l) == 'n') { // State I_CON
                    advance(l);
                    if (peek(l) == 't') { // State I_CONT
                        advance(l);
                        if (peek(l) == 'i') { // State I_CONTI
                            advance(l);
                            if (peek(l) == 'n') { // State I_CONTIN
                                advance(l);
                                if (peek(l) == 'u') { // State I_CONTINU
                                    advance(l);
                                    if (peek(l) == 'e') { // State I_CONTINUE
                                        advance(l);
                                        if (!is_ident_char(peek(l))) { // Rule 2: CONTINUE *
                                            return make_token(l, TOKEN_KW_CONTINUE, start, start_line, start_col);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;

        case 'd': // State I_D
            if (peek(l) == 'i') { // State I_DI
                advance(l);
                if (peek(l) == 'v') { // State I_DIV
                    advance(l);
                    if (!is_ident_char(peek(l))) { // Rule 2: DIV *
                        return make_token(l, TOKEN_OP_INT_DIV, start, start_line, start_col);
                    }
                }
            } else if (peek(l) == 'o') { // State I_DO
                advance(l);
                if (!is_ident_char(peek(l))) { // Rule 2: DO *
                    return make_token(l, TOKEN_KW_DO, start, start_line, start_col);
                }

                if (peek(l) == 'u') { // State I_DOU
                    advance(l);
                    if (peek(l) == 'b') { // State I_DOUB
                        advance(l);
                        if (peek(l) == 'l') { // State I_DOUBL
                            advance(l);
                            if (peek(l) == 'e') { // State I_DOUBLE
                                advance(l);
                                if (!is_ident_char(peek(l))) { // Rule 2: DOUBLE *
                                    return make_token(l, TOKEN_RW_DOUBLE, start, start_line, start_col);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case 'e': // State I_E
            if (peek(l) == 'l') { // State I_EL
                advance(l);
                if (peek(l) == 's') { // State I_ELS
                    advance(l);
                    if (peek(l) == 'e') { // State I_ELSE
                        advance(l);
                        if (!is_ident_char(peek(l))) { // Rule 2: ELSE *
                            return make_token(l, TOKEN_KW_ELSE, start, start_line, start_col);
                        }
                    }
                }
            }
            break;

        case 'f': // State I_F
            if (peek(l) == 'a') { // State I_FA
                advance(l);
                if (peek(l) == 'l') { // State I_FAL
                    advance(l);
                    if (peek(l) == 's') { // State I_FALS
                        advance(l);
                        if (peek(l) == 'e') { // State I_FALSE
                            advance(l);
                            if (!is_ident_char(peek(l))) { // Rule 2: FALSE *
                                return make_bool_token(l, start, start_line, start_col, false);
                            }
                        }
                    }
                }
            } else if (peek(l) == 'u') { // State I_FU
                advance(l);
                if (peek(l) == 'n') { // State I_FUN
                    advance(l);
                    if (!is_ident_char(peek(l))) { // Rule 2: FUN *
                        return make_token(l, TOKEN_KW_FUN, start, start_line, start_col);
                    }
                }
            }
            break;

        case 'i': // State I_I
            if (peek(l) == 'f') { // State I_IF
                advance(l);
                if (!is_ident_char(peek(l))) { // Rule 2: IF *
                    return make_token(l, TOKEN_KW_IF, start, start_line, start_col);
                }
            } else if (peek(l) == 'n') { // State I_IN
                advance(l);
                if (peek(l) == 't') { // State I_INT
                    advance(l);
                    if (!is_ident_char(peek(l))) { // Rule 2: INT *
                        return make_token(l, TOKEN_RW_INT, start, start_line, start_col);
                    }
                }
            }
            break;

        case 'n': // State I_N
            if (peek(l) == 'o') { // State I_NO
                advance(l);
                if (peek(l) == 't') { // State I_NOT
                    advance(l);
                    if (!is_ident_char(peek(l))) { // Rule 2: NOT *
                        return make_token(l, TOKEN_OP_NOT, start, start_line, start_col);
                    }
                }
            }
            break;

        case 'o': // State I_O
            if (peek(l) == 'r') { // State I_OR
                advance(l);
                if (!is_ident_char(peek(l))) { // Rule 2: OR *
                    return make_token(l, TOKEN_OP_OR, start, start_line, start_col);
                }
            }
            break;

        case 'r': // State I_R
            if (peek(l) == 'e') { // State I_RE
                advance(l);
                if (peek(l) == 'c') { // State I_REC
                    advance(l);
                    if (peek(l) == 'o') { // State I_RECO
                        advance(l);
                        if (peek(l) == 'r') { // State I_RECOR
                            advance(l);
                            if (peek(l) == 'd') { // State I_RECORD
                                advance(l);
                                if (!is_ident_char(peek(l))) { // Rule 2: RECORD *
                                    return make_token(l, TOKEN_RW_RECORD, start, start_line, start_col);
                                }
                            }
                        }
                    }
                } else if (peek(l) == 't') { // State I_RET
                    advance(l);
                    if (peek(l) == 'u') { // State I_RETU
                        advance(l);
                        if (peek(l) == 'r') { // State I_RETUR
                            advance(l);
                            if (peek(l) == 'n') { // State I_RETURN
                                advance(l);
                                if (!is_ident_char(peek(l))) { // Rule 2: RETURN *
                                    return make_token(l, TOKEN_KW_RETURN, start, start_line, start_col);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case 't': // State I_T
            if (peek(l) == 'h') { // State I_TH
                advance(l);
                if (peek(l) == 'e') { // State I_THE
                    advance(l);
                    if (peek(l) == 'n') { // State I_THEN
                        advance(l);
                        if (!is_ident_char(peek(l))) { // Rule 2: THEN *
                            return make_token(l, TOKEN_NOISE, start, start_line, start_col);
                        }
                    }
                }
            } else if (peek(l) == 'r') { // State I_TR
                advance(l);
                if (peek(l) == 'u') { // State I_TRU
                    advance(l);
                    if (peek(l) == 'e') { // State I_TRUE
                        advance(l);
                        if (!is_ident_char(peek(l))) { // Rule 2: TRUE *
                            return make_bool_token(l, start, start_line, start_col, true);
                        }
                    }
                }
            }
            break;

        case 'v': // State I_V
            if (peek(l) == 'a') { // State I_VA
                advance(l);
                if (peek(l) == 'l') { // State I_VAL
                    advance(l);
                    if (!is_ident_char(peek(l))) { // Rule 2: VAL *
                        return make_token(l, TOKEN_KW_VAL, start, start_line, start_col);
                    }
                } else if (peek(l) == 'r') { // State I_VAR
                    advance(l);
                    if (!is_ident_char(peek(l))) { // Rule 2: VAR *
                        return make_token(l, TOKEN_KW_VAR, start, start_line, start_col);
                    }
                }
            }
            break;

        case 'w': // State I_W
            if (peek(l) == 'h') { // State I_WH
                advance(l);
                if (peek(l) == 'e') { // State I_WHE
                    advance(l);
                    if (peek(l) == 'n') { // State I_WHEN
                        advance(l);
                        if (!is_ident_char(peek(l))) { // Rule 2: WHEN *
                            return make_token(l, TOKEN_KW_WHEN, start, start_line, start_col);
                        }
                    }
                } else if (peek(l) == 'i') { // State I_WHI
                    advance(l);
                    if (peek(l) == 'l') { // State I_WHIL
                        advance(l);
                        if (peek(l) == 'e') { // State I_WHILE
                            advance(l);
                            if (!is_ident_char(peek(l))) { // Rule 2: WHILE *
                                return make_token(l, TOKEN_KW_WHILE, start, start_line, start_col);
                            }
                        }
                    }
                }
            }
            break;

        default: // Not the first letter of any keyword
            break;
    }

    while (is_ident_char(peek(l))) {
        advance(l);
    }

    return make_token(l, TOKEN_IDENT, start, start_line, start_col);
}

static Token invalid_number(LexerState *l, const char *start, const int line, const int col) {
    while (is_ident_char(peek(l))) {
        advance(l);
    }

    return make_invalid_token(l, start, line, col, "malformed number literal (unexpected characters after the number)");
}

Token scan_number(LexerState *l) {
    const char *start = l->cursor;
    const int start_line = l->line;
    const int start_col = l->column;

    // Binary (0b0101) and hexadecimal (0xAD) int literals
    if (l->cursor[0] == '0' && (l->cursor[1] == 'b' || l->cursor[1] == 'B' ||
                                l->cursor[1] == 'x' || l->cursor[1] == 'X')) {
        const int base = (l->cursor[1] == 'b' || l->cursor[1] == 'B') ? 2 : 16;

        advance(l);
        advance(l);

        unsigned long long value = 0;
        int digits = 0;
        int bad_digit = 0;
        int out_of_range = 0;

        while (is_ident_char(peek(l))) {
            const char c = advance(l);
            int digit;

            if (c >= '0' && c <= '9') {
                digit = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                digit = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'F') {
                digit = c - 'A' + 10;
            } else {
                digit = 99;
            }

            if (digit >= base) {
                bad_digit = 1;
                continue;
            }

            value = value * base + digit;
            digits++;

            // Int literals are 32-bit
            if (value > INT_MAX) {
                out_of_range = 1;
                value = 0;
            }
        }

        if (bad_digit) {
            return make_invalid_token(l, start, start_line, start_col,
                                      base == 2 ? "invalid digit in binary literal" : "invalid digit in hexadecimal literal");
        }

        if (digits == 0) {
            return make_invalid_token(l, start, start_line, start_col, "missing digits after the 0b/0x prefix");
        }

        if (out_of_range) {
            return make_invalid_token(l, start, start_line, start_col, "int literal is out of 32-bit range");
        }

        return make_int_token(l, start, start_line, start_col, (int) value);
    }

    int is_double = 0;

    while (isdigit((unsigned char) peek(l))) {
        advance(l);
    }

    // Fractional part (3.14); a '.' must be followed by a digit
    if (peek(l) == '.' && isdigit((unsigned char) l->cursor[1])) {
        is_double = 1;
        advance(l);

        while (isdigit((unsigned char) peek(l))) {
            advance(l);
        }
    }

    // Exponent (3e-1)
    if (peek(l) == 'e' || peek(l) == 'E') {
        const char next = l->cursor[1];

        if (isdigit((unsigned char) next) ||
            ((next == '+' || next == '-') && isdigit((unsigned char) l->cursor[2]))) {
            is_double = 1;
            advance(l); // e
            if (peek(l) == '+' || peek(l) == '-') {
                advance(l);
            }

            while (isdigit((unsigned char) peek(l))) {
                advance(l);
            }
        }
    }

    // 'D' suffix is not part of the numeric value
    const char *number_end = l->cursor;

    if (peek(l) == 'D') {
        is_double = 1;
        advance(l);
    }

    // If the next character is an identifier character, the number is malformed (e.g. "123abc")
    if (is_ident_char(peek(l))) {
        return invalid_number(l, start, start_line, start_col);
    }

    const size_t length = (size_t) (number_end - start);
    char *text = malloc(length + 1);
    memcpy(text, start, length);
    text[length] = '\0';

    Token token;

    if (is_double) {
        errno = 0;
        const double value = strtod(text, NULL);

        token = errno == ERANGE
                    ? make_invalid_token(l, start, start_line, start_col, "double literal is out of range")
                    : make_double_token(l, start, start_line, start_col, value);
    } else {
        errno = 0;
        const long long value = strtoll(text, NULL, 10);

        token = (errno == ERANGE || value > INT_MAX)
                    ? make_invalid_token(l, start, start_line, start_col, "int literal is out of 32-bit range")
                    : make_int_token(l, start, start_line, start_col, (int) value);
    }

    free(text);

    return token;
}

// Reads the character after a backslash
static int read_escape(LexerState *l, char *out) {
    const char c = peek(l);

    if (c == '\0' || c == '\n') {
        return 0;
    }

    advance(l);

    switch (c) {
        case 'n': *out = '\n'; return 1;
        case 't': *out = '\t'; return 1;
        case 'r': *out = '\r'; return 1;
        case '0': *out = '\0'; return 1;
        case '\\': *out = '\\'; return 1;
        case '\'': *out = '\''; return 1;
        case '"': *out = '"'; return 1;
        default: return 0;
    }
}

// Skips the remainder of a malformed char/string literal so lexing can resume after it
static Token recover_literal(LexerState *l, const char *start, const int line, const int col, const char quote,
                             const char *error) {
    while (peek(l) != '\0' && peek(l) != '\n') {
        const char c = advance(l);

        if (c == '\\' && peek(l) != '\0' && peek(l) != '\n') {
            advance(l);
        } else if (c == quote) {
            break;
        }
    }

    return make_invalid_token(l, start, line, col, error);
}

Token scan_char(LexerState *l) {
    const char *start = l->cursor;
    const int start_line = l->line;
    const int start_col = l->column;

    advance(l); // opening quote

    const char c = peek(l);
    char value;

    if (c == '\0' || c == '\n' || c == '\'') {
        // Unterminated or empty char literal
        if (c == '\'') {
            advance(l);

            return make_invalid_token(l, start, start_line, start_col, "empty character literal");
        }

        return make_invalid_token(l, start, start_line, start_col, "unterminated character literal");
    }

    if (c == '\\') {
        advance(l);

        if (!read_escape(l, &value)) {
            return recover_literal(l, start, start_line, start_col, '\'', "invalid escape sequence in character literal");
        }
    } else if ((unsigned char) c >= 0x80) {
        // Not representable by a single UTF-8 code unit
        return recover_literal(l, start, start_line, start_col, '\'',
                               "character literal is not a single-byte character");
    } else {
        value = advance(l);
    }

    if (peek(l) != '\'') {
        if (peek(l) == '\0' || peek(l) == '\n') {
            return make_invalid_token(l, start, start_line, start_col, "unterminated character literal");
        }

        return recover_literal(l, start, start_line, start_col, '\'',
                               "character literal must contain exactly one character");
    }

    advance(l); // closing quote

    return make_char_token(l, start, start_line, start_col, value);
}

Token scan_string(LexerState *l) {
    const char *start = l->cursor;
    const int start_line = l->line;
    const int start_col = l->column;

    advance(l); // opening quote

    size_t capacity = 16;
    size_t length = 0;
    char *buffer = malloc(capacity);
    int valid = 1;

    while (peek(l) != '"') {
        if (peek(l) == '\0' || peek(l) == '\n') {
            // Unterminated string
            free(buffer);

            return make_invalid_token(l, start, start_line, start_col, "unterminated string literal");
        }

        char c = advance(l);

        if (c == '\\') {
            if (!read_escape(l, &c)) {
                valid = 0;
                continue;
            }
        }

        if (length + 1 >= capacity) {
            capacity *= 2;
            buffer = realloc(buffer, capacity);
        }

        buffer[length++] = c;
    }

    advance(l); // closing quote

    if (!valid) {
        free(buffer);

        return make_invalid_token(l, start, start_line, start_col, "invalid escape sequence in string literal");
    }

    buffer[length] = '\0';

    return make_string_token(l, start, start_line, start_col, buffer);
}

Token scan_operator(LexerState *l) {
    const char *start = l->cursor;
    const int start_line = l->line;
    const int start_col = l->column;

    const char first = advance(l);

    switch (first) {
        case '+':
            if (peek(l) == '=') {
                advance(l);
                return make_token(l, TOKEN_OP_ADD_ASSIGN, start, start_line, start_col);
            }
            return make_token(l, TOKEN_OP_ADD, start, start_line, start_col);

        case '-':
            if (peek(l) == '=') {
                advance(l);
                return make_token(l, TOKEN_OP_SUB_ASSIGN, start, start_line, start_col);
            }
            return make_token(l, TOKEN_OP_SUB, start, start_line, start_col);

        case '*':
            return make_token(l, TOKEN_OP_MUL, start, start_line, start_col);

        case '/':
            return make_token(l, TOKEN_OP_DIV, start, start_line, start_col);

        case '%':
            return make_token(l, TOKEN_OP_MOD, start, start_line, start_col);

        case '^':
            return make_token(l, TOKEN_OP_EXPONENT, start, start_line, start_col);

        case '<':
            if (peek(l) == '=') {
                advance(l);
                return make_token(l, TOKEN_OP_LTE, start, start_line, start_col);
            }
            return make_token(l, TOKEN_OP_LT, start, start_line, start_col);

        case '>':
            if (peek(l) == '=') {
                advance(l);
                return make_token(l, TOKEN_OP_GTE, start, start_line, start_col);
            }
            return make_token(l, TOKEN_OP_GT, start, start_line, start_col);

        case '=':
            if (peek(l) == '=') {
                advance(l);
                return make_token(l, TOKEN_OP_EQUALS, start, start_line, start_col);
            }
            return make_token(l, TOKEN_OP_ASSIGN, start, start_line, start_col);

        case '!':
            if (peek(l) == '=') {
                advance(l);
                return make_token(l, TOKEN_OP_NOT_EQ, start, start_line, start_col);
            }
            return make_token(l, TOKEN_OP_BANG, start, start_line, start_col);

        case ';':
            return make_token(l, TOKEN_SEMICOLON, start, start_line, start_col);

        case ',':
            return make_token(l, TOKEN_COMMA, start, start_line, start_col);

        case ':':
            return make_token(l, TOKEN_COLON, start, start_line, start_col);

        case '(':
            return make_token(l, TOKEN_LPAREN, start, start_line, start_col);

        case ')':
            return make_token(l, TOKEN_RPAREN, start, start_line, start_col);

        case '{':
            return make_token(l, TOKEN_LBRACE, start, start_line, start_col);

        case '}':
            return make_token(l, TOKEN_RBRACE, start, start_line, start_col);

        default:
            while (((unsigned char) peek(l) & 0xC0) == 0x80) {
                advance(l);
            }

            return make_invalid_token(l, start, start_line, start_col, "unexpected character");
    }
}

int is_ident_char(const char ch) {
    return isalnum((unsigned char) ch) || ch == '_';
}
