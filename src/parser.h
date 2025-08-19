#ifndef PARSER_H
#define PARSER_H

#include "arena.h"

typedef enum Type {
    DEFINE,
    STRUCT,
    ENUM,
    FUNC,
    GLOBAL_VAR,
} Type;

typedef struct TypedVar {
    char        *type;
    char        *ident;
} TypedVar;

typedef struct TypedStruct {
    char        *ident;
    TypedVar    *vars;
    long long   len;
} TypedStruct;

typedef struct TypedEnum {
    char        *ident;
    char        **types;
    long long   len;
} TypedEnum;

typedef struct TypedFunc {
    char        *ret_type;
    char        *ident;

    TypedVar    *args;
    long long   len;
} TypedFunc;

typedef struct Expr {
    Type type;

    union {
        TypedVar        *var;
        TypedFunc       *tf;
        TypedStruct     *ts;
        TypedEnum       *te;
    };
} Expr;

typedef struct ArrayExpr {
    Expr     *data;
    ptrdiff_t len;
    ptrdiff_t cap;
} ArrayExpr;


void parse_c_file(Arena arena, Arena scratch, char *path, ArrayExpr *exprs);

#endif /* PARSER_H */
