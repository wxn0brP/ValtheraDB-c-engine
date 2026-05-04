#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <jansson.h>
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
    if (b->data)
        b->data[0] = '\0';
}

static void buf_append(Buffer *b, const char *s)
{
    size_t slen = strlen(s);
    if (b->len + slen + 1 > b->cap)
    {
        while (b->len + slen + 1 > b->cap)
            b->cap *= 2;
        char *new_data = (char *)realloc(b->data, b->cap);
        if (!new_data)
            return;
        b->data = new_data;
    }
    if (b->data)
    {
        memcpy(b->data + b->len, s, slen);
        b->len += slen;
        b->data[b->len] = '\0';
    }
}

bool find_on_file(
    const char *file,
    json_t *fields,
    bool findOne,
    json_error_t *error,
    Buffer *out,
    bool *first)
{
    FILE *f = fopen(file, "r");
    if (!f)
    {
        return false;
    }

    char line[4096];
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '\0')
            continue;

        json_t *json = json_loads(line, 0, error);
        if (!json)
            continue;

        if (has_fields_advanced(json, fields) > 0)
        {
            if (!findOne)
            {
                if (!*first)
                {
                    buf_append(out, ",");
                }
                else
                {
                    *first = false;
                }
            }
            buf_append(out, line);
            json_decref(json);

            if (findOne)
            {
                fclose(f);
                return true;
            }
        }
        json_decref(json);
    }

    fclose(f);
    return false;
}

char *find(const char *dir, const char *fields_json, bool findOne)
{
    json_error_t error;

    json_t *fields = json_loads(fields_json, 0, &error);
    if (!fields)
    {
        char *res = malloc(256);
        if (res)
            strcpy(res, "2: Invalid fields JSON");
        return res;
    }

    DIR *d = opendir(dir);
    if (!d)
    {
        json_decref(fields);
        char *res = malloc(256);
        if (res)
            strcpy(res, "3: Directory not found");
        return res;
    }

    Buffer out;
    buf_init(&out);
    bool first = true;

    if (!findOne)
        buf_append(&out, "[");

    struct dirent *entry;
    bool found = false;

    while ((entry = readdir(d)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char filepath[4096];
        int written = snprintf(filepath, sizeof(filepath), "%s/%s", dir, entry->d_name);
        if (written < 0 || written >= (int)sizeof(filepath))
            continue;

        struct stat st;
        if (stat(filepath, &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        if (strcmp(filepath + strlen(filepath) - 3, ".db") != 0)
            continue;

        if (find_on_file(filepath, fields, findOne, &error, &out, &first))
        {
            found = true;
            if (findOne)
                break;
        }
    }

    closedir(d);
    json_decref(fields);

    if (!findOne)
    {
        buf_append(&out, "]");
    }
    else if (!found)
    {
        free(out.data);
        char *null_res = malloc(6);
        if (null_res)
            strcpy(null_res, "null");
        return null_res;
    }

    return out.data;
}

void free_result(char *ptr)
{
    free(ptr);
}
