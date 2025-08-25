#ifndef PARSER_H
#define PARSER_H

#include "arena.h"

typedef struct String {
    char    *val;
    size_t  len;
} String;

typedef struct TypedDefine {
    String ident;
    String value;
} TypedDefine;

typedef struct TypedVar {
    String type;
    String ident;
} TypedVar;

typedef struct TypedStruct {
    String ident;
    // TODO: array
    TypedVar    *vars;
    long long   len;
} TypedStruct;

typedef struct TypedEnum {
    String ident;
    String *types;
    long long   len;
} TypedEnum;

typedef struct TypedFunc {
    String ret_type;
    String ident;

    TypedVar    *args;
    long long   len;
} TypedFunc;

typedef struct ArrayDefine {
    TypedDefine *data;
    ptrdiff_t len;
    ptrdiff_t cap;
} ArrayDefine;

void parse_c_file(Arena *arena, Arena scratch, char *path, ArrayDefine *defs);

#endif /* PARSER_H */
