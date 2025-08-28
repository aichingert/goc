#include <stdio.h>

// ashdlkl */

typedef struct String {
    const char *val;
    const long len;
} String;

typedef enum VALUES {
    A, B, C, D = 10,
} VALUES;

typedef int Zahl;
typedef bool Boolean;

static char *hello() {
    return "dshflasj";
}

static const char h = { 10 };


int main(void) {
    printf("%d %s\n", h, hello());
    VALUES a = D;
    return 0;

} =====
=
