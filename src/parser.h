#ifndef PARSER_H
#define PARSER_H

#include "arena.h"

typedef struct String {
    char    *val;
    size_t  len;
} String;

typedef struct TypedCopy {
    String ident;
    String value;
} TypedCopy;

typedef struct TypedFunc {
    String ret_type;
    String ident;

    TypedCopy  *copies;
    long long   len;
} TypedFunc;

typedef struct ArrayCopy {
    TypedCopy *data;
    ptrdiff_t len;
    ptrdiff_t cap;
} ArrayCopy;

void parse_c_file(Arena *arena, Arena scratch, char *path, ArrayCopy *copies);

#endif /* PARSER_H */
