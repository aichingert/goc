#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

typedef struct Arena {
    char *beg;
    char *end;
} Arena;

void *arena_alloc(Arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count, bool zero) {
    ptrdiff_t padding = -(uintptr_t)a->beg & (align - 1);
    ptrdiff_t available = a->end - a->beg - padding;

    if (available < 0 || count > available / size) {
        assert(false && "TODO: realloc");
    }

    void *p = a->beg + padding;
    a->beg += padding + count * size;

    if (!zero) {
        return p;
    }

    return memset(p, 0, count * size);
}

#define alloc(...)            allocx(__VA_ARGS__,alloc4,alloc3,alloc2)(__VA_ARGS__)
#define allocx(a,b,c,d,e,...) e
#define alloc2(a, t)          (t *)arena_alloc(a, sizeof(t), alignof(t), 1, true)
#define alloc3(a, t, n)       (t *)arena_alloc(a, sizeof(t), alignof(t), n, true)
#define alloc4(a, t, n, b)    (t *)arena_alloc(a, sizeof(t), alignof(t), n, b)

#define sizeof(x)    (ptrdiff_t)sizeof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define len(s)  (countof(s) - 1)

typedef enum Type {
    DEFINE,
    STRUCT,
    ENUM,
    FUNC,
    GLOBAL_VAR,
} Type;

typedef struct TypedVar {
    char        *type;
    char        *ident;
} TypedVar;

typedef struct TypedStruct {
    char        *ident;
    TypedVar    *vars;
    long long   len;
} TypedStruct;

typedef struct TypedEnum {
    char        *ident;
    char        **types;
    long long   len;
} TypedEnum;

typedef struct TypedFunc {
    char        *ret_type;
    char        *ident;

    TypedVar    *args;
    long long   len;
} TypedFunc;

typedef struct Expr {
    Type type;

    union {
        TypedVar        *var;
        TypedFunc       *tf;
        TypedStruct     *ts;
        TypedEnum       *te;
    };
} Expr;

int main(int argc, char **argv) {
    ptrdiff_t size = 1 << 24;
    Arena perm = {0};
    perm.beg = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    perm.end = perm.beg + size; 

    printf("%lu\n", size);

    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        // TODO: report error
        if (f == NULL) continue;

        fseek(f, 0, SEEK_END);
        long long len = ftell(f);
        rewind(f);

        char *buf = alloc(&perm, char, len, false);
        fread(buf, sizeof(char), len, f);

        for (int j = 0; j < len; j++) {
            printf("%c", buf[j]);
        }

        fclose(f);
    }

    return 0;
}
