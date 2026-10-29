//
// Created by Gabriel Virrey on 10/29/26.
//

#ifndef JAVELIN_LEXER_TOKENS_H
#define JAVELIN_LEXER_TOKENS_H

#include <stdbool.h>

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_INVALID,

    // IDENTIFIERS
    TOKEN_IDENT,

    // LITERALS
    TOKEN_CHAR_LIT,
    TOKEN_BOOL_LIT,
    TOKEN_DOUBLE_LIT,
    TOKEN_STRING_LIT,
    TOKEN_INT_LIT,

    // NOISE WORDS AND COMMENTS (ignored by a parser, but still recorded in the symbol table)
    TOKEN_NOISE,
    TOKEN_COMMENT,

    // KEYWORDS
    TOKEN_KW_AS,
    TOKEN_KW_SAFE_AS,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_KW_DO,
    TOKEN_KW_ELSE,
    TOKEN_KW_FUN,
    TOKEN_KW_IF,
    TOKEN_KW_RETURN,
    TOKEN_KW_VAL,
    TOKEN_KW_VAR,
    TOKEN_KW_WHEN,
    TOKEN_KW_WHILE,

    // RESERVED WORDS
    TOKEN_RW_BOOL,
    TOKEN_RW_CHAR,
    TOKEN_RW_DOUBLE,
    TOKEN_RW_INT,
    TOKEN_RW_RECORD,

    // OPERATORS
    TOKEN_OP_AND,
    TOKEN_OP_BANG,
    TOKEN_OP_NOT,
    TOKEN_OP_OR,
    TOKEN_OP_MUL,
    TOKEN_OP_DIV,
    TOKEN_OP_INT_DIV,
    TOKEN_OP_MOD,
    TOKEN_OP_EXPONENT,
    TOKEN_OP_ADD,
    TOKEN_OP_SUB,
    TOKEN_OP_GT,
    TOKEN_OP_GTE,
    TOKEN_OP_LT,
    TOKEN_OP_LTE,
    TOKEN_OP_EQUALS,
    TOKEN_OP_NOT_EQ,
    TOKEN_OP_ASSIGN,
    TOKEN_OP_SUB_ASSIGN,
    TOKEN_OP_ADD_ASSIGN,

    // DELIMITERS AND BRACKETS
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE
} TokenType;

typedef struct {
    TokenType type;
    const char *lexeme;
    int length;
    int line;
    int column;
    const char *error;

    union {
        int int_val;
        double double_val;
        bool bool_val;
        char char_val;
        char *string_val;
    };
} Token;

static inline const char *token_type_name(const TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_INVALID: return "INVALID";
        case TOKEN_IDENT: return "IDENT";
        case TOKEN_CHAR_LIT: return "CHAR_LIT";
        case TOKEN_BOOL_LIT: return "BOOL_LIT";
        case TOKEN_DOUBLE_LIT: return "DOUBLE_LIT";
        case TOKEN_STRING_LIT: return "STRING_LIT";
        case TOKEN_INT_LIT: return "INT_LIT";
        case TOKEN_NOISE: return "NOISE";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_KW_AS: return "KW_AS";
        case TOKEN_KW_SAFE_AS: return "KW_SAFE_AS";
        case TOKEN_KW_BREAK: return "KW_BREAK";
        case TOKEN_KW_CONTINUE: return "KW_CONTINUE";
        case TOKEN_KW_DO: return "KW_DO";
        case TOKEN_KW_ELSE: return "KW_ELSE";
        case TOKEN_KW_FUN: return "KW_FUN";
        case TOKEN_KW_IF: return "KW_IF";
        case TOKEN_KW_RETURN: return "KW_RETURN";
        case TOKEN_KW_VAL: return "KW_VAL";
        case TOKEN_KW_VAR: return "KW_VAR";
        case TOKEN_KW_WHEN: return "KW_WHEN";
        case TOKEN_KW_WHILE: return "KW_WHILE";
        case TOKEN_RW_BOOL: return "RW_BOOL";
        case TOKEN_RW_CHAR: return "RW_CHAR";
        case TOKEN_RW_DOUBLE: return "RW_DOUBLE";
        case TOKEN_RW_INT: return "RW_INT";
        case TOKEN_RW_RECORD: return "RW_RECORD";
        case TOKEN_OP_AND: return "OP_AND";
        case TOKEN_OP_BANG: return "OP_BANG";
        case TOKEN_OP_NOT: return "OP_NOT";
        case TOKEN_OP_OR: return "OP_OR";
        case TOKEN_OP_MUL: return "OP_MUL";
        case TOKEN_OP_DIV: return "OP_DIV";
        case TOKEN_OP_INT_DIV: return "OP_INT_DIV";
        case TOKEN_OP_MOD: return "OP_MOD";
        case TOKEN_OP_EXPONENT: return "OP_EXPONENT";
        case TOKEN_OP_ADD: return "OP_ADD";
        case TOKEN_OP_SUB: return "OP_SUB";
        case TOKEN_OP_GT: return "OP_GT";
        case TOKEN_OP_GTE: return "OP_GTE";
        case TOKEN_OP_LT: return "OP_LT";
        case TOKEN_OP_LTE: return "OP_LTE";
        case TOKEN_OP_EQUALS: return "OP_EQUALS";
        case TOKEN_OP_NOT_EQ: return "OP_NOT_EQ";
        case TOKEN_OP_ASSIGN: return "OP_ASSIGN";
        case TOKEN_OP_SUB_ASSIGN: return "OP_SUB_ASSIGN";
        case TOKEN_OP_ADD_ASSIGN: return "OP_ADD_ASSIGN";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_COLON: return "COLON";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
    }

    return "UNKNOWN";
}

#endif //JAVELIN_LEXER_TOKENS_H
