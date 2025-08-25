#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#include "arena.h"
#include "parser.h"

int main(int argc, char **argv) {
    ptrdiff_t size = 1 << 24;
    ptrdiff_t ssize = 2 * 1024 * 1024;

    Arena perm, scratch = {0};
    perm.beg = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    perm.end = perm.beg + size; 

    scratch.beg = alloc(&perm, char, ssize); 
    scratch.end = scratch.beg + ssize; 

    ArrayDefine defs = {0};

    for (int i = 1; i < argc; i++) {
        parse_c_file(&perm, scratch, argv[i], &defs);
    }

    printf("%lu\n", defs.len);
    return 0;
}
