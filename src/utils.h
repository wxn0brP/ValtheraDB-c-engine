#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>

#define VDB_ERR_INVALID_FIELDS_JSON "2: Invalid fields JSON"
#define VDB_ERR_DIRECTORY_NOT_FOUND "3: Directory not found"

typedef struct
{
    char *data;
    size_t len;
    size_t cap;
} Buffer;

typedef struct
{
    char **items;
    size_t len;
    size_t cap;
} FileList;

bool buf_init(Buffer *b);
bool buf_append_len(Buffer *b, const char *s, size_t slen);
bool buf_append(Buffer *b, const char *s);
char *copy_result(const char *s, size_t len);
bool build_path(char *out, size_t out_size, const char *dir, const char *name);
bool get_sorted_db_files(const char *dir, FileList *list);
void file_list_free(FileList *list);

#endif
