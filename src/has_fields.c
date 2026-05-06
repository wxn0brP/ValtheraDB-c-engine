#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "has_fields.h"
#include "array_helpers.h"
#include "conditions.h"
#include "deep_check.h"

static void normalize_key(const char *src, char *dest)
{
    while (*src)
    {
        *dest = (char)tolower((unsigned char)*src);
        dest++;
        src++;
    }
    *dest = '\0';
}

int has_fields_advanced(json_t *obj, json_t *fields)
{
    if (!json_is_object(fields))
    {
        fprintf(stderr, "Fields must be an object\n");
        exit(1);
    }

    if (json_object_size(fields) == 0)
        return 1;

    const char *key;
    json_t *value;
    json_object_foreach(fields, key, value)
    {
        if (key[0] == '$')
        {
            char normalized[256];
            normalize_key(key, normalized);

            if (strcmp(normalized, "$and") == 0)
            {
                if (!json_is_array(value))
                    return 0;
                size_t index;
                json_t *sub_fields;
                json_array_foreach(value, index, sub_fields)
                {
                    if (!has_fields_advanced(obj, sub_fields))
                        return 0;
                }
                return 1;
            }

            if (strcmp(normalized, "$or") == 0)
            {
                if (!json_is_array(value))
                    return 0;
                size_t index;
                json_t *sub_fields;
                json_array_foreach(value, index, sub_fields)
                {
                    if (has_fields_advanced(obj, sub_fields))
                        return 1;
                }
                return 0;
            }
        }
    }

    json_t *conditions = json_object();
    json_t *simple_fields = json_object();

    json_object_foreach(fields, key, value)
    {
        if (key[0] == '$')
        {
            char normalized[256];
            normalize_key(key, normalized);
            json_object_set(conditions, normalized, value);
        }
        else
        {
            json_object_set(simple_fields, key, value);
        }
    }

    int cond_ok = 1;
    const char *op_key;
    json_t *op_value;
    json_object_foreach(conditions, op_key, op_value)
    {
        if (strcmp(op_key, "$not") == 0)
        {
            if (has_fields_advanced(obj, op_value))
                cond_ok = 0;
        }
        else if (strcmp(op_key, "$subset") == 0)
        {
            if (!has_fields_simple(obj, op_value))
                cond_ok = 0;
        }
        else
        {
            if (!check_condition(obj, op_key, op_value))
                cond_ok = 0;
        }
    }

    if (!cond_ok)
    {
        json_decref(conditions);
        json_decref(simple_fields);
        return 0;
    }

    int fields_ok = 1;
    if (json_object_size(simple_fields) > 0)
    {
        fields_ok = has_fields_simple(obj, simple_fields);
    }

    json_decref(conditions);
    json_decref(simple_fields);
    return fields_ok;
}
