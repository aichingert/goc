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

static const char TYPEDEF[] = "typedef";
static const char STRUCT[]  = "struct";
static const char ENUM[]    = "enum";

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
    return pos + 1 < len && buf[pos] == '/' && buf[pos + 1] == '/';
}

bool is_multiline_comment_start(size_t len, size_t pos, char *buf) {
    return pos + 1 < len && buf[pos] == '/' && buf[pos + 1] == '*';
}

size_t skip_whitespace_and_newline(size_t len, size_t pos, char *buf) {
    size_t cur = pos;
    while (cur < len && (buf[cur] == ' ' || buf[cur] == '\n')) {
        cur += 1;
    }
    return cur - pos;
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
    while (cur + 1 < len && (buf[cur] != '*' || buf[cur + 1] != '/')) {
        cur += 1;
    }
    return 2 + cur - pos;
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

size_t parse_include_define_or_pragma(Arena *arena, ArrayCopy *defs, size_t len, size_t pos, char *buf) {
    size_t end = parse_identifier(len, pos + 1, buf); 

    if          (is_equal(buf, pos, len, INCLUDE, strlen(INCLUDE))) {
        // TODO: add line number and path
        printf("WARNING: `#include` is being ignored\n");
        pos += skip_line(len, pos + end, buf);
    } else if   (is_equal(buf, pos, len, DEFINE, strlen(DEFINE))) {
        pos += end + 1;

        size_t next = skip_till_next(len, pos, buf);
        // TODO: report if next == 0 => invalid c
        if (next == 0) return len;
        pos += next;

        size_t ident = pos;
        size_t define_end = 0;

        end = parse_identifier(len, ident, buf);
        define_end = parse_define(len, pos + end, buf);

        TypedCopy def = {
            .ident = (String){ .val = alloc(arena, char, end),          .len = end },
            .value = (String){ .val = alloc(arena, char, define_end),   .len = define_end },
        };
        for (size_t i = 0; i < end; i++)        def.ident.val[i] = buf[pos + i];
        for (size_t i = 0; i < define_end; i++) def.value.val[i] = buf[pos + end + i];
        *push(defs, arena) = def;

        return end + define_end;
    } else if   (is_equal(buf, pos, len, PRAGMA, strlen(PRAGMA))) {
        printf("ERROR: parser got into an invalid state :~ (ABORTING)\n");
        return len;
    } else {
        printf("ERROR: parser got into an invalid state :~ (ABORTING)\n");
        return len;
    }

    return pos;
}

TypedCopy parse_struct(Arena *arena, size_t len, size_t *pos, char *buf) {
    size_t end = parse_identifier(len, *pos, buf);

    size_t cur    = 0;
    size_t braces = 1;
    size_t skip   = *pos + end;

    while (skip < len && buf[skip] != '{') skip += 1; 
    cur = skip + 1;

    while (cur < len && braces > 0) {
        if      (buf[cur] == '{') braces += 1;
        else if (buf[cur] == '}') braces -= 1;

        cur += 1;
    } 

    // TODO: error handling
    if (cur >= len) return (TypedCopy){0};

    TypedCopy plex = {
        .ident = (String){ .val = alloc(arena, char, end),              .len = end },
        .value = (String){ .val = alloc(arena, char, cur - skip),  .len = cur - skip },
    };
    for (size_t i = 0; i < end; i++)    plex.ident.val[i]           = buf[*pos + i];
    for (size_t i = skip; i < cur; i++) plex.value.val[i - skip]    = buf[i];

    cur += skip_line(len, cur, buf);
    *pos = cur;
    return plex;
}

TypedCopy parse_enum(Arena *arena, size_t len, size_t *pos, char *buf) {
    size_t end = parse_identifier(len, *pos, buf);

    size_t skip = *pos + end;
    while (skip < len && buf[skip] != '{') skip += 1;

    size_t cur = skip;
    while (cur < len && buf[cur] != '}') cur += 1;
    cur += 1;

    TypedCopy data = {
        .ident = (String){ .val = alloc(arena, char, end),           .len = end },
        .value = (String){ .val = alloc(arena, char, cur - skip),    .len = cur - skip },
    };
    for (size_t i = 0; i < end; i++)    data.ident.val[i] = buf[*pos + i];
    for (size_t i = skip; i < cur; i++) data.value.val[i - skip] = buf[i];

    cur += skip_line(len, cur, buf);
    *pos = cur;
    return data;
}

size_t parse_identifier_states(Arena *arena, size_t len, size_t pos, char *buf) {
    size_t cur = pos;

    if          (is_equal(buf, cur, len, TYPEDEF, strlen(TYPEDEF))) {
        cur += strlen(TYPEDEF);
        cur += skip_till_next(len, cur, buf);

        if          (is_equal(buf, cur, len, STRUCT, strlen(STRUCT))) {
            cur += strlen(STRUCT);
            cur += skip_till_next(len, cur, buf);

            TypedCopy plex = parse_struct(arena, len, &cur, buf);

        } else if   (is_equal(buf, cur, len, ENUM, strlen(ENUM))) {
            cur += strlen(ENUM);
            cur += skip_till_next(len, cur, buf);

            TypedCopy data = parse_enum(arena, len, &cur, buf);
        } else {
            size_t end = parse_identifier(len, cur, buf);
            for (size_t i = 0; i < end; i++) {
                printf("%c", buf[cur + i]);
            }
            printf("\n");
        }
    }

    return cur - pos;
}

void parse_c_file(Arena *arena, Arena scratch, char *path, ArrayCopy *copies) {
    size_t len    = 0;
    size_t pos    = 0;
    char *content = read_file(scratch, path, &len);

    while (pos < len) {
        pos += skip_whitespace_and_newline(len, pos, content);

        if          (is_include_define_or_pragma(content[pos])) {
            pos += parse_include_define_or_pragma(arena, copies, len, pos, content);
        } else if   (is_identifier_start(content[pos])) {
            pos += parse_identifier_states(arena, len, pos, content);
        } else if   (is_line_comment_start(len, pos, content)) {
            pos += skip_line(len, pos, content);
        } else if   (is_multiline_comment_start(len, pos, content)) {
            pos += skip_multiline_comment(len, 2 + pos, content) + 2;
        } else {
            pos += skip_till_next(len, pos, content);
        }
    }

}
