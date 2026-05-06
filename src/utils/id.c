#include "id.h"
#include <string.h>

int64_t convert_id_to_unix(const char *id)
{
    int64_t result = 0;

    if (!id)
        return 0;

    for (const char *p = id; *p && *p != '-'; ++p)
    {
        char c = *p;
        int v;

        if (c >= '0' && c <= '9')
            v = c - '0';
        else if (c >= 'a' && c <= 'z')
            v = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z')
            v = c - 'A' + 10;
        else
            break;

        result = result * 36 + v;
    }

    return result;
}

static int cmp_i64(int64_t a, int64_t b)
{
    return (a > b) - (a < b);
}

static int64_t json_to_value(const json_t *v)
{
    if (!v)
        return 0;

    switch (json_typeof(v))
    {
    case JSON_STRING:
        return convert_id_to_unix(json_string_value(v));
    case JSON_INTEGER:
        return json_integer_value(v);
    case JSON_REAL:
        return (int64_t)json_real_value(v);
    default:
        return 0;
    }
}

int compare_ids(const json_t *a, const json_t *b)
{
    int ta = json_typeof(a);
    int tb = json_typeof(b);

    if (ta == JSON_STRING && tb == JSON_STRING)
    {
        const char *sa = json_string_value(a);
        const char *sb = json_string_value(b);

        int64_t ua = convert_id_to_unix(sa);
        int64_t ub = convert_id_to_unix(sb);

        int diff = cmp_i64(ua, ub);
        if (diff != 0)
            return diff;

        return strcmp(sa, sb);
    }

    if (ta == JSON_INTEGER && tb == JSON_INTEGER)
    {
        return cmp_i64(json_integer_value(a), json_integer_value(b));
    }

    return cmp_i64(json_to_value(a), json_to_value(b));
}
