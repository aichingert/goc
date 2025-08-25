#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#include "parser.h"

#define COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

static const char INCLUDE[] = "#include";
static const char DEFINE[]  = "#define";
static const char PRAGMA[]  = "#pragma";

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

bool is_include_define_or_pragma(char character) {
    return character == '#';
}

bool is_line_comment_start(size_t len, size_t pos, char *buf) {
    return pos + 2 < len && buf[pos] == '/' && buf[pos + 1] == '/';
}

bool is_multiline_comment_start(size_t len, size_t pos, char *buf) {
    return pos + 2 < len && buf[pos] == '/' && buf[pos + 1] == '*';
}

size_t skip_line(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len && buf[cur] != '\n') {
        cur += 1;
    }
    return 1 + cur - pos;
}

size_t skip_multiline_comment(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len) {
        if (cur + 2 >= len) return len - pos;
        cur += 1;
        if (buf[cur - 1] == '*' && buf[cur] == '/') break; 
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

size_t parse_identifier(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len && is_valid_identifier_char(buf[cur])) {
        cur += 1;
    }
    return cur - pos;
}

size_t parse_define(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len && buf[cur] != '\n') {
        if (buf[cur] == '\\') {
            cur += skip_line(len, cur, buf);
        } else {
            cur += 1;
        }
    }
    return 1 + cur - pos;
}

bool is_equal(char *content, size_t start, size_t len, const char *cmp, size_t cmp_len) {
    if (start + cmp_len > len) {
        return false;
    }

    for (size_t i = 0; i < cmp_len; i++) {
        if (content[start + i] != cmp[i]) {
            return false;
        }
    }

    return true;
}

void parse_c_file(Arena *arena, Arena scratch, char *path, ArrayDefine *defs) {
    size_t len    = 0;
    size_t pos    = 0;
    char *content = read_file(scratch, path, &len);

    while (pos < len) {
        if          (is_include_define_or_pragma(content[pos])) {
            size_t end = parse_identifier(len, pos + 1, content); 

            if          (is_equal(content, pos, len, INCLUDE, strlen(INCLUDE))) {
                // TODO: add line number and path
                printf("WARNING: `#include` is being ignored\n");
                pos += skip_line(len, pos + end, content);

            } else if   (is_equal(content, pos, len, DEFINE, strlen(DEFINE))) {
                pos += end + 1;

                while (pos < len && !is_identifier_start(content[pos])) {
                    pos += 1;
                }
                if (pos >= len) {
                    // TODO: report
                    break;
                }

                size_t ident = pos;
                size_t define_end = 0;

                end = parse_identifier(len, ident, content);
                define_end = parse_define(len, pos + end, content);

                TypedDefine def = {
                    .ident = (String){ .val = alloc(arena, char, end), .len = end },
                    .value = (String){ .val = alloc(arena, char, define_end), .len = define_end },
                };

                for (size_t i = 0; i < end; i++) def.ident.val[i] = content[pos + i];
                for (size_t i = 0; i < define_end; i++) def.value.val[i] = content[pos + end + i];
                *push(defs, arena) = def;

                pos += end + define_end;
            } else if   (is_equal(content, pos, len, PRAGMA, strlen(PRAGMA))) {
                break;
            } else {
                printf("ERROR: parser got into an invalid state :~ (ABORTING)\n");
                break;
            }
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
