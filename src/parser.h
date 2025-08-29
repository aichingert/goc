#ifndef PARSER_H
#define PARSER_H

#include "arena.h"
#include "tokenize.h"

typedef struct LineRange {
    Token start;
    uint32_t end_line;
} LineRange;

typedef struct CharRange {
    Token start;
    Token end_char;
} CharRange;

typedef struct ArrayLineRange {
    LineRange *data;
    ptrdiff_t len;
    ptrdiff_t cap;
} ArrayLineRange;

typedef struct ArrayCharRange {
    CharRange *data;
    ptrdiff_t len;
    ptrdiff_t cap;
} ArrayCharRange;

// TODO: parse ifdefs
typedef struct FileContent {
    ArrayLineRange pragmas;
    ArrayLineRange defines;
    ArrayLineRange includes;

    ArrayCharRange enums;
    ArrayCharRange structs;
    ArrayCharRange globals;
    ArrayCharRange typedefs;
    ArrayCharRange functions;
} FileContent;

FileContent parse_c_file(Arena *arena, Arena scratch, char *path);

#endif /* PARSER_H */
