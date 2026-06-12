#define _POSIX_C_SOURCE 200809L

#include "remove.h"

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "has_fields.h"
#include "utils.h"

typedef struct
{
    json_t *fields;
    bool one;
} RemoveOptions;

typedef struct
{
    Buffer removed;
    bool first_removed;
    bool already_removed_one;
} RemoveState;

typedef struct
{
    RemoveOptions opts;
    RemoveState state;
    json_error_t error;
} RemoveContext;

static bool append_removed(RemoveContext *ctx, const char *line, size_t line_len)
{
    if (!ctx->state.first_removed && !buf_append(&ctx->state.removed, ","))
        return false;

    ctx->state.first_removed = false;
    return buf_append_len(&ctx->state.removed, line, line_len);
}

static bool write_line(FILE *f, const char *line, size_t line_len)
{
    return fwrite(line, 1, line_len, f) == line_len && fputc('\n', f) != EOF;
}

static void trim_line(char **line, size_t *line_len)
{
    while (*line_len > 0 && ((*line)[*line_len - 1] == '\n' || (*line)[*line_len - 1] == '\r'))
        (*line)[--(*line_len)] = '\0';

    while (**line == ' ' || **line == '\t')
    {
        (*line)++;
        (*line_len)--;
    }

    while (*line_len > 0 && ((*line)[*line_len - 1] == ' ' || (*line)[*line_len - 1] == '\t'))
        (*line)[--(*line_len)] = '\0';
}

static bool should_remove_line(RemoveContext *ctx, const char *line)
{
    if (ctx->opts.one && ctx->state.already_removed_one)
        return false;

    json_t *json = json_loads(line, 0, &ctx->error);
    if (!json)
        return false;

    bool should_remove = has_fields_advanced(json, ctx->opts.fields) > 0;
    json_decref(json);
    return should_remove;
}

static bool remove_on_file(const char *file, RemoveContext *ctx)
{
    FILE *in = fopen(file, "r");
    if (!in)
        return true;

    char tmpfile[4100];
    int written = snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", file);
    if (written < 0 || written >= (int)sizeof(tmpfile))
    {
        fclose(in);
        return false;
    }

    FILE *out = fopen(tmpfile, "w");
    if (!out)
    {
        fclose(in);
        return false;
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t read;
    bool ok = true;

    while ((read = getline(&line, &cap, in)) != -1)
    {
        size_t line_len = (size_t)read;
        char *trimmed = line;
        trim_line(&trimmed, &line_len);

        if (line_len == 0)
            continue;

        if (should_remove_line(ctx, trimmed))
        {
            if (!append_removed(ctx, trimmed, line_len))
            {
                ok = false;
                break;
            }

            if (ctx->opts.one)
                ctx->state.already_removed_one = true;
            continue;
        }

        if (!write_line(out, trimmed, line_len))
        {
            ok = false;
            break;
        }
    }

    free(line);

    if (fclose(in) != 0)
        ok = false;
    if (fclose(out) != 0)
        ok = false;

    if (ok && rename(tmpfile, file) != 0)
        ok = false;

    if (!ok)
        remove(tmpfile);

    return ok;
}

static RemoveContext make_remove_context(json_t *fields, bool one)
{
    RemoveContext ctx = {
        .opts = {
            .fields = fields,
            .one = one,
        },
        .state = {
            .removed = {0},
            .first_removed = true,
            .already_removed_one = false,
        },
    };

    return ctx;
}

char *remove_entries(const char *dir, const char *fields_json, bool one)
{
    json_error_t error;

    json_t *fields = json_loads(fields_json, 0, &error);
    if (!fields)
        return copy_result(VDB_ERR_INVALID_FIELDS_JSON, strlen(VDB_ERR_INVALID_FIELDS_JSON));

    FileList files;
    if (!get_sorted_db_files(dir, &files))
    {
        json_decref(fields);
        return copy_result(VDB_ERR_DIRECTORY_NOT_FOUND, strlen(VDB_ERR_DIRECTORY_NOT_FOUND));
    }

    RemoveContext ctx = make_remove_context(fields, one);
    bool ok = buf_init(&ctx.state.removed) && buf_append(&ctx.state.removed, "[");

    size_t file_limit = one && files.len > 0 ? 1 : files.len;
    for (size_t i = 0; ok && i < file_limit; i++)
    {
        char filepath[4096];
        if (!build_path(filepath, sizeof(filepath), dir, files.items[i]))
            continue;

        ok = remove_on_file(filepath, &ctx);
    }

    file_list_free(&files);
    json_decref(fields);

    if (!ok || !buf_append(&ctx.state.removed, "]"))
    {
        free(ctx.state.removed.data);
        return NULL;
    }

    return ctx.state.removed.data;
}
