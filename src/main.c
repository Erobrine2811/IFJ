#include "3AC.h"
#include "error.h"
#include "parser.h"
#include <stdio.h>

// Global 3AC code list
ThreeACList threeACcode;

int main(int argc, char *argv[])
{
    FILE *file = NULL;

    if (argc == 1)
    {
        // No file argument, read from standard input
        file = stdin;
    }
    else if (argc == 2)
    {
        // File argument provided, open the file
        file = fopen(argv[1], "r");
        if (file == NULL)
        {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
            return INTERNAL_ERROR;
        }
    }
    else
    {
        // Wrong number of arguments
        fprintf(stderr, "Usage: %s [<source_file>]\n", argv[0]);
        return INTERNAL_ERROR;
    }

    list_init(&threeACcode);
    int result = parse_program(file);

    // Only close the file if it was opened by fopen (i.e., not stdin)
    if (argc == 2)
    {
        fclose(file);
    }

    if (result == 0)
    {
        list_print(&threeACcode);
        list_dispose(&threeACcode);
    }

    return result;
}
