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

void parse_c_file(Arena *arena, Arena scratch, char *path);

#endif /* PARSER_H */
