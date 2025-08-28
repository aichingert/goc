#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "stdint.h"
#include "stddef.h"

#include "arena.h"

typedef enum TokenType {
    T_ENUM      = 0,
    T_STRUCT    = 1,
    T_TYPEDEF   = 2,
    T_IDENT     = 3,
    C_PRAGMA    = 4,
    C_DEFINE    = 5,
    C_INCLUDE   = 6,

    O_EQ        = 8,
    TT_L_BRACE  = 9,
    TT_R_BRACE  = 10,
    TT_L_PAREN  = 11,
    TT_R_PAREN  = 12,
    TT_L_BRACKET= 13,
    TT_R_BRACKET= 14,
    TT_COMMA    = 15,
    TT_SEMICOLON= 16,

    R_EOF       = 17,
    R_IGNORE    = 18,
} TokenType;

typedef struct Token {
    uint32_t    beg;
    TokenType   type;
} Token;

typedef struct ArrayToken {
    Token *data;
    ptrdiff_t len;
    ptrdiff_t cap;
} ArrayToken;

ArrayToken tokenize(Arena *arena, const char const *path, const char *source, uint32_t len);

#endif /* TOKENIZE_H */


