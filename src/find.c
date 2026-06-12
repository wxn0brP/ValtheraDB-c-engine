#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <jansson.h>
#include "has_fields.h"
#include "find.h"

#define FIND_ERR_INVALID_FIELDS_JSON "2: Invalid fields JSON"
#define FIND_ERR_DIRECTORY_NOT_FOUND "3: Directory not found"

typedef struct
{
    char *data;
    size_t len;
    size_t cap;
} Buffer;

static bool buf_init(Buffer *b)
{
    b->cap = 1024;
    b->len = 0;
    b->data = (char *)malloc(b->cap);
    if (!b->data)
        return false;

    b->data[0] = '\0';
    return true;
}

static bool buf_reserve(Buffer *b, size_t needed)
{
    if (needed <= b->cap)
        return true;

    size_t new_cap = b->cap;
    while (needed > new_cap)
    {
        if (new_cap > ((size_t)-1) / 2)
            return false;
        new_cap *= 2;
    }

    char *new_data = (char *)realloc(b->data, new_cap);
    if (!new_data)
        return false;

    b->data = new_data;
    b->cap = new_cap;
    return true;
}

static bool buf_append_len(Buffer *b, const char *s, size_t slen)
{
    if (b->len > ((size_t)-1) - 1 || slen > ((size_t)-1) - b->len - 1)
        return false;

    if (!buf_reserve(b, b->len + slen + 1))
        return false;

    memcpy(b->data + b->len, s, slen);
    b->len += slen;
    b->data[b->len] = '\0';
    return true;
}

static bool buf_append(Buffer *b, const char *s)
{
    return buf_append_len(b, s, strlen(s));
}

static char *copy_result(const char *s, size_t len)
{
    char *res = (char *)malloc(len + 1);
    if (!res)
        return NULL;

    memcpy(res, s, len);
    res[len] = '\0';
    return res;
}

static bool has_db_suffix(const char *name)
{
    size_t len = strlen(name);
    return len >= 3 && strcmp(name + len - 3, ".db") == 0;
}

static char *find_on_file(
    const char *file,
    json_t *fields,
    bool findOne,
    json_error_t *error,
    Buffer *out,
    bool *first,
    bool *ok)
{
    FILE *f = fopen(file, "r");
    if (!f)
    {
        return NULL;
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t read;
    char *match = NULL;

    while ((read = getline(&line, &cap, f)) != -1)
    {
        size_t line_len = (size_t)read;
        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r'))
        {
            line[--line_len] = '\0';
        }

        if (line_len == 0)
            continue;

        json_t *json = json_loads(line, 0, error);
        if (!json)
            continue;

        if (has_fields_advanced(json, fields) > 0)
        {
            if (findOne)
            {
                match = copy_result(line, line_len);
                if (!match)
                    *ok = false;
                json_decref(json);
                break;
            }

            if (!*first && !buf_append(out, ","))
            {
                *ok = false;
                json_decref(json);
                break;
            }
            *first = false;

            if (!buf_append_len(out, line, line_len))
            {
                *ok = false;
                json_decref(json);
                break;
            }
        }
        json_decref(json);
    }

    free(line);
    fclose(f);
    return match;
}

char *find(const char *dir, const char *fields_json, bool findOne)
{
    json_error_t error;

    json_t *fields = json_loads(fields_json, 0, &error);
    if (!fields)
    {
        return copy_result(FIND_ERR_INVALID_FIELDS_JSON, strlen(FIND_ERR_INVALID_FIELDS_JSON));
    }

    DIR *d = opendir(dir);
    if (!d)
    {
        json_decref(fields);
        return copy_result(FIND_ERR_DIRECTORY_NOT_FOUND, strlen(FIND_ERR_DIRECTORY_NOT_FOUND));
    }

    Buffer out;
    out.data = NULL;
    out.len = 0;
    out.cap = 0;
    bool first = true;
    bool ok = true;

    if (!findOne)
    {
        if (!buf_init(&out) || !buf_append(&out, "["))
            ok = false;
    }

    struct dirent *entry;
    char *result = NULL;

    while (ok && (entry = readdir(d)) != NULL)
    {
        if (!has_db_suffix(entry->d_name))
            continue;

        char filepath[4096];
        int written = snprintf(filepath, sizeof(filepath), "%s/%s", dir, entry->d_name);
        if (written < 0 || written >= (int)sizeof(filepath))
            continue;

        struct stat st;
        if (stat(filepath, &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        result = find_on_file(filepath, fields, findOne, &error, findOne ? NULL : &out, &first, &ok);
        if (findOne && result)
        {
            break;
        }
    }

    closedir(d);
    json_decref(fields);

    if (!ok)
    {
        free(out.data);
        free(result);
        return NULL;
    }

    if (findOne)
    {
        return result ? result : copy_result("null", strlen("null"));
    }

    if (!findOne)
    {
        if (!buf_append(&out, "]"))
        {
            free(out.data);
            return NULL;
        }
    }

    return out.data;
}

void free_result(char *ptr)
{
    free(ptr);
}
