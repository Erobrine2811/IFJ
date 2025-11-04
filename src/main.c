#include <stdio.h>
#include "parser.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }

    int result = parse_program(file);

    fclose(file);

    if (result == 0) {
        printf("Compilation successful.\n");
    } else {
        printf("Compilation failed.\n");
    }

    return result;
}
