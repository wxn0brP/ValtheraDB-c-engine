#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "find.h"

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        fprintf(stderr, "Usage: %s <file> <fields_json> [true]\n", argv[0]);
        return 1;
    }

    bool findOne = argc == 4 && strcmp(argv[3], "true") == 0;

    char *result = find(argv[1], argv[2], findOne);

    if (!result)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }

    printf("%s\n", result);

    free_result(result);

    return 0;
}
