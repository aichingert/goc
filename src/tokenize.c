#include "string.h"

#include "tokenize.h"

bool is_ident_start(char character) {
    return character >= 'a' && character <= 'z' || character >= 'A' && character <= 'Z';
}

bool is_ident(char character) {
    return is_ident_start(character) || character >= '0' && character <= '9';
}

Token consume_identifier(uint32_t *pos, uint32_t len, const char *source) {
    Token tok = {
        .beg  = *pos,
        .type = T_IDENT,
    };

    while (*pos < len && is_ident(source[*pos])) {
        *pos += 1;
    }

    uint32_t diff = *pos - tok.beg;

    if (diff == 6 && strcmp(source + *pos, "struct")) {
        tok.type = T_STRUCT;
    }

    return tok;
}

ArrayToken tokenize(Arena *arena, const char *source, uint32_t len) {
    ArrayToken tokens = {0};

    uint32_t pos = 0;
    uint32_t line = 0;

    while (pos < len) {
        if          (is_ident_start(source[pos])) {
            *push(&tokens, arena) = consume_identifier(&pos, len, source);
        } else if (false) {}

    }

    return tokens;
}
