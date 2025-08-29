#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#include "parser.h"
#include "tokenize.h"

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

LineRange consume_define(uint32_t *pos, Arena *arena, ArrayToken *toks, const char *source) {
    uint32_t iden = *pos;
    uint32_t line = toks->data[iden].line;
    *pos += 2;

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

void consume_func_or_global(uint32_t *pos, Arena *arena, ArrayToken *toks, const char *source) {

}

void parse_c_file(Arena *arena, Arena scratch, char *path) {
    uint32_t len    = 0;
    uint32_t pos    = 0;
    const char *source = read_file(scratch, path, &len);
    ArrayToken toks = tokenize(arena, path, source, len);

    while (pos < toks.len) {
        TokenType type = toks.data[pos].type;
        uint32_t stop = 0;
        /*
        for (uint32_t i = toks.data[pos].beg;; i++) {
            printf("%c", source[i]);
            stop++;
            if (stop > 15) break;
        }
        printf(" - %d - file: %s\n", type, path); 
        */

        if          (type == C_DEFINE || type == C_PRAGMA || type == C_INCLUDE) {
            // TODO: store defines and pragmas
            LineRange l_range = consume_define(&pos, arena, &toks, source);
            /*
            uint32_t line = l_range.start.line;

            for (uint32_t i = l_range.start.beg;; i++) {
                if (line > l_range.end_line) break;
                printf("%c", source[i]);
                if (source[i] == '\n') line += 1;
            }
            printf("\n");
            */

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

            printf("SWUCT \n");
            for (uint32_t i = c_range.start.beg; i < c_range.end_char.beg; i++) {
                printf("%c", source[i]);
            }
            printf("\n");
        } else if   (type == T_IDENT) {
            consume_func_or_global(&pos, arena, &toks, source);

            printf("here\n");
        }
    }
}
