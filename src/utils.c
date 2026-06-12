#include "utils.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

bool buf_init(Buffer *b)
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

bool buf_append_len(Buffer *b, const char *s, size_t slen)
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

bool buf_append(Buffer *b, const char *s)
{
    return buf_append_len(b, s, strlen(s));
}

char *copy_result(const char *s, size_t len)
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

static bool is_numeric_db_name(const char *name)
{
    size_t len = strlen(name);
    if (len <= 3 || !has_db_suffix(name))
        return false;

    for (size_t i = 0; i < len - 3; i++)
    {
        if (name[i] < '0' || name[i] > '9')
            return false;
    }

    return true;
}

static int compare_db_names(const void *a, const void *b)
{
    const char *name_a = *(const char *const *)a;
    const char *name_b = *(const char *const *)b;
    long num_a = strtol(name_a, NULL, 10);
    long num_b = strtol(name_b, NULL, 10);

    if (num_a < num_b)
        return -1;
    if (num_a > num_b)
        return 1;
    return strcmp(name_a, name_b);
}

void file_list_free(FileList *list)
{
    for (size_t i = 0; i < list->len; i++)
        free(list->items[i]);
    free(list->items);
    list->items = NULL;
    list->len = 0;
    list->cap = 0;
}

static bool file_list_push(FileList *list, const char *name)
{
    if (list->len == list->cap)
    {
        size_t new_cap = list->cap == 0 ? 16 : list->cap * 2;
        if (new_cap < list->cap)
            return false;

        char **new_items = (char **)realloc(list->items, new_cap * sizeof(char *));
        if (!new_items)
            return false;

        list->items = new_items;
        list->cap = new_cap;
    }

    char *copy = copy_result(name, strlen(name));
    if (!copy)
        return false;

    list->items[list->len++] = copy;
    return true;
}

bool build_path(char *out, size_t out_size, const char *dir, const char *name)
{
    int written = snprintf(out, out_size, "%s/%s", dir, name);
    return written >= 0 && written < (int)out_size;
}

bool get_sorted_db_files(const char *dir, FileList *list)
{
    list->items = NULL;
    list->len = 0;
    list->cap = 0;

    DIR *d = opendir(dir);
    if (!d)
        return false;

    struct dirent *entry;
    bool ok = true;

    while ((entry = readdir(d)) != NULL)
    {
        if (!is_numeric_db_name(entry->d_name))
            continue;

        char filepath[4096];
        if (!build_path(filepath, sizeof(filepath), dir, entry->d_name))
            continue;

        struct stat st;
        if (stat(filepath, &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        if (!file_list_push(list, entry->d_name))
        {
            ok = false;
            break;
        }
    }

    closedir(d);

    if (!ok)
    {
        file_list_free(list);
        return false;
    }

    qsort(list->items, list->len, sizeof(char *), compare_db_names);
    return true;
}
