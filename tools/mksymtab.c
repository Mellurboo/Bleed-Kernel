#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/// @brief kernel symbol table
struct ksym {
    uint64_t addr;
    uint32_t name_off;
};

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <nm.txt> <out>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "wb");
    if (!in || !out) return 1;

    struct ksym syms[16384];
    char strtab[131072];

    size_t sc = 0;
    size_t so = 1;
    strtab[0] = 0;

    while (!feof(in)) {
        uint64_t addr;
        char type;
        char name[256];

        if (fscanf(in, "%lx %c %255s", &addr, &type, name) != 3)
            continue;

        // strip everything but functions for the stack trace
        if (type != 'T' && type != 't')
            continue;

        syms[sc].addr = addr;
        syms[sc].name_off = so;

        strcpy(&strtab[so], name);
        so += strlen(name) + 1;
        sc++;
    }

    fwrite(&sc, sizeof(sc), 1, out);
    fwrite(syms, sizeof(struct ksym), sc, out);
    fwrite(strtab, so, 1, out);

    return 0;
}
