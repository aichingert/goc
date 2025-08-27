#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "stdint.h"
#include "stddef.h"

#include "arena.h"

typedef enum TokenType {
    T_ENUM,
    T_STRUCT,
    T_TYPEDEF,
    T_IDENT,
    T_MACRO,
    T_PRAGMA,
    T_DEFINE,
    T_INCLUDE,
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

ArrayToken tokenize(Arena *arena, const char *source, uint32_t len);

#endif /* TOKENIZE_H */


