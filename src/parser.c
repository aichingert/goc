#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#include "parser.h"

char* read_file(Arena arena, char *path, size_t *len) {
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

bool is_identifier_start(char character) {
    return character >= 'a' && character <= 'z' || character >= 'A' && character <= 'Z';
}

bool is_valid_identifier_char(char character) {
    return is_identifier_start(character) || character >= '0' && character <= '9';
}

bool is_include_or_define(char character) {
    return character == '#';
}

bool is_line_comment_start(size_t len, size_t pos, char *buf) {
    return pos + 2 < len && buf[pos] == '/' && buf[pos + 1] == '/';
}

bool is_multiline_comment_start(size_t len, size_t pos, char *buf) {
    return pos + 2 < len && buf[pos] == '/' && buf[pos + 1] == '*';
}

size_t parse_identifier(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len && is_valid_identifier_char(buf[cur])) {
        cur += 1;
    }
    return cur - pos;
}

size_t skip_line(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len && buf[cur] != '\n') {
        cur += 1;
    }
    return cur - pos;
}

size_t skip_multiline_comment(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len) {
        if (cur + 2 >= len) return len - pos;
        cur += 1;
        if (buf[cur] == '*' && buf[cur + 1] == '/') break; 
    }
    return cur - pos;
}

size_t skip_till_next(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len && buf[cur] != '#' && !is_identifier_start(buf[cur])) {
        cur += 1;
    }
    return cur - pos;
}

void parse_c_file(Arena arena, Arena scratch, char *path, ArrayExpr *exprs) {
    size_t len    = 0;
    size_t pos    = 0;
    char *content = read_file(scratch, path, &len);

    while (pos < len) {
        if          (is_include_or_define(content[pos])) {
            pos += 1;
        } else if   (is_identifier_start(content[pos])) {
            size_t end = parse_identifier(len, pos, content);

            pos += end;

        } else if   (is_line_comment_start(len, pos, content)) {
            pos += skip_line(len, pos, content);
        } else if   (is_multiline_comment_start(len, pos, content)) {
            pos += skip_multiline_comment(len, pos, content);
        } else {
            pos += skip_till_next(len, pos, content);
        }
    }

}
