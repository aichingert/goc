#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#include "arena.h"
#include "parser.h"

int main(int argc, char **argv) {
    ptrdiff_t size = (ptrdiff_t)2 * 1024 * 1024 * 1024;
    ptrdiff_t ssize = 2 * 1024 * 1024;

    Arena perm, scratch = {0};
    perm.beg = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    perm.end = perm.beg + size; 

    scratch.beg = alloc(&perm, char, ssize); 
    scratch.end = scratch.beg + ssize; 

    for (int i = 1; i < argc; i++) {
        FileContent content = parse_c_file(&perm, scratch, argv[i]);

        printf("includes=%d\ndefines=%d\nfuncs=%d\n", content.includes.len, content.defines.len, content.functions.len);
    }

    return 0;
}
