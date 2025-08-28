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

void consume_define(uint32_t *pos, Arena *arena, ArrayToken *toks, const char *source) {
    assert(toks->data[*pos].type == C_DEFINE && "Error: expected define in consume");


}

void parse_c_file(Arena *arena, Arena scratch, char *path, ArrayCopy *copies) {
    uint32_t len    = 0;
    uint32_t pos    = 0;
    const char *source = read_file(scratch, path, &len);
    ArrayToken toks = tokenize(arena, path, source, len);

    while (pos < len) {
        if          (toks.data[pos].type == C_DEFINE) {
            consume_define(&pos, arena, &toks, source);
        } else if   (toks.data[pos].type == C_PRAGMA) {

        } else if   (toks.data[pos].type == C_INCLUDE) {

        }

        pos += 1;
    }
}
