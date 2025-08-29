#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#include "parser.h"
#include "tokenize.h"

bool is_func(uint32_t pos, ArrayToken *toks) {
    while (pos < toks->len && !(toks->data[pos].type == TT_L_BRACE || toks->data[pos].type == O_EQ)) {
        pos += 1;
    }

    return pos < toks->len && toks->data[pos].type == TT_L_BRACE;
}

char* read_file(Arena arena, char *path, uint32_t *len) {
    FILE *f = fopen(path, "r");
    if (f == NULL) assert(false && "error: file not found");

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    rewind(f);

    char *buf = alloc(&arena, char, *len, false);
    fread(buf, sizeof(char), *len, f);

    fclose(f);
    return buf;
}

LineRange consume_define_pragma_and_include(
        uint32_t *pos, 
        Arena *arena, 
        ArrayToken *toks, 
        const char *source
) {
    uint32_t iden = *pos;
    uint32_t line = toks->data[iden].line;

    while (*pos < toks->len && toks->data[*pos].line == line) {
        if (toks->data[*pos].type == TT_BACKSLASH) {
            line += 1;
        }

        *pos += 1;
    }

    return (LineRange){
        .start = toks->data[iden],
        .end_line = line,
    };
}

CharRange consume_struct_or_enum(uint32_t *pos, Arena *arena, ArrayToken *toks, const char *source) {
    *pos += 1;
    assert(toks->data[*pos].type == T_IDENT && "Error: expected identifier after struct or enum");

    Token iden = toks->data[*pos];
    uint16_t braces = 1;
    *pos += 2;

    while (*pos < toks->len && braces > 0) {
        if      (toks->data[*pos].type == TT_L_BRACE) braces += 1;
        else if (toks->data[*pos].type == TT_R_BRACE) braces -= 1;

        *pos += 1;
    }

    Token end_char = toks->data[*pos];

    while (*pos < toks->len && toks->data[*pos].type != TT_SEMICOLON) {
        *pos += 1;
    }
    *pos += 1;

    return (CharRange){
        .start = iden,
        .end_char = end_char,
    };
}

CharRange consume_typedef(uint32_t *pos, Arena *arena, ArrayToken *toks, const char *source) {
    Token start = toks->data[*pos];

    *pos += 1;
    TokenType type = toks->data[*pos].type;

    if (type == T_STRUCT || type == T_ENUM) {
        return (CharRange){.start = {0}, .end_char = {0}};
    }

    assert(type == T_IDENT && "Error: expected identifier after typedef");

    while (*pos < toks->len && toks->data[*pos].type != TT_SEMICOLON) {
        *pos += 1;
    }
    *pos += 1;

    return (CharRange){
        .start = start,
        .end_char = toks->data[*pos],
    };
}

CharRange consume_func(uint32_t *pos, Arena *arena, ArrayToken *toks, const char *source) {
    Token start = toks->data[*pos];

    while (*pos < toks->len && toks->data[*pos].type != TT_L_PAREN) {
        *pos += 1;
    }

    // TODO: check for occurances somewhere else
    Token ident = toks->data[*pos - 1];
    uint16_t braces = 1;

    while (*pos < toks->len && toks->data[*pos].type != TT_L_BRACE) {
        *pos += 1;
    }
    *pos += 1;

    while (*pos < toks->len && braces > 0) {
        if      (toks->data[*pos].type == TT_L_BRACE) braces += 1;
        else if (toks->data[*pos].type == TT_R_BRACE) braces -= 1;

        *pos += 1;
    }

    return (CharRange){
        .start = start,
        .end_char = toks->data[*pos],
    };
}

CharRange consume_func_or_global(
        uint32_t *pos, 
        bool is_function, 
        Arena *arena, 
        ArrayToken *toks, 
        const char *source
) {
    if (is_function) {
        return consume_func(pos, arena, toks, source);
    }

    Token start = toks->data[*pos];

    while (*pos < toks->len && toks->data[*pos].type != TT_SEMICOLON) {
        *pos += 1;
    }
    *pos += 1;

    // TODO: iterate backwards to check identifier for 
    // redefinitions or maybe do it later when correcting errors
    return (CharRange){
        .start = start,
        .end_char = toks->data[*pos - 1],
    };
}

void parse_c_file(Arena *arena, Arena scratch, char *path) {
    uint32_t len    = 0;
    uint32_t pos    = 0;
    const char *source = read_file(scratch, path, &len);
    ArrayToken toks = tokenize(arena, path, source, len);
    printf("%d\n", toks.len);

    while (toks.data[pos].type != R_EOF) {
        TokenType type = toks.data[pos].type;
        printf("%d\n", type);

        if          (type == C_DEFINE || type == C_PRAGMA || type == C_INCLUDE) {
            LineRange l_range = consume_define_pragma_and_include(&pos, arena, &toks, source);
        } else if   (type == T_TYPEDEF) {
            CharRange c_range = consume_typedef(&pos, arena, &toks, source);
            if (c_range.start.beg == 0 && c_range.end_char.beg == 0) continue;

            printf("TYPEDEF\n");
            for (uint32_t i = c_range.start.beg; i < c_range.end_char.beg; i++) {
                printf("%c", source[i]);
            }
            printf("\n");

        } else if   (type == T_STRUCT || type == T_ENUM) {
            CharRange c_range = consume_struct_or_enum(&pos, arena, &toks, source);

            printf("%s\n", type == T_STRUCT ? "STRUCT" : "ENUM");
            for (uint32_t i = c_range.start.beg; i < c_range.end_char.beg; i++) {
                printf("%c", source[i]);
            }
            printf("\n");
        } else if   (type == T_IDENT) {
            bool is_function = is_func(pos, &toks);
            CharRange c_range = consume_func_or_global(&pos, is_function, arena, &toks, source);

            printf("%s\n", is_function ? "FUNC" : "GLOB");
            for (uint32_t i = c_range.start.beg; i < c_range.end_char.beg; i++) {
                printf("%c", source[i]);
            }
            printf("\n");

        }
    }
}
