#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include "has_fields.h"
#include "find.h"

typedef struct
{
    char *data;
    size_t len;
    size_t cap;
} Buffer;

static void buf_init(Buffer *b)
{
    b->cap = 1024;
    b->len = 0;
    b->data = (char *)malloc(b->cap);
    b->data[0] = '\0';
}

static void buf_append(Buffer *b, const char *s)
{
    size_t slen = strlen(s);

    if (b->len + slen + 1 > b->cap)
    {
        while (b->len + slen + 1 > b->cap)
            b->cap *= 2;

        b->data = (char *)realloc(b->data, b->cap);
    }

    memcpy(b->data + b->len, s, slen);
    b->len += slen;
    b->data[b->len] = '\0';
}

char *find(const char *file, const char *fields_s, bool findOne)
{
    json_error_t error;

    json_t *fields = json_loads(fields_s, 0, &error);
    if (!fields)
    {
        char *res = malloc(3);
        strcpy(res, "c1");
        return res;
    }

    FILE *f = fopen(file, "r");
    if (!f)
    {
        json_decref(fields);
        char *res = malloc(3);
        strcpy(res, "c2");
        return res;
    }

    Buffer out;
    buf_init(&out);

    if (!findOne)
        buf_append(&out, "[");

    bool first = 1;

    char line[1024 * 4];

    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\r\n")] = 0;

        if (line[0] == '\0')
            continue;

        json_t *json = json_loads(line, 0, &error);
        if (!json)
        {
            continue;
        }

        int result = has_fields_advanced(json, fields);

        if (result > 0)
        {
            if (!findOne)
            {
                if (!first)
                    buf_append(&out, ",");
                else
                    first = 0;
            }

            buf_append(&out, line);

            json_decref(json);

            if (findOne)
                break;
        }
        else
        {
            json_decref(json);
        }
    }

    fclose(f);
    json_decref(fields);

    if (!findOne)
        buf_append(&out, "]");

    return out.data;
}

void free_result(char *ptr)
{
    free(ptr);
}
