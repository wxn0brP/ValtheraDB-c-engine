#include "deep_check.h"
#include "array_helpers.h"

int deep_check(json_t *value_obj, json_t *target_obj, json_t *(*get_func)(const json_t *, const char *))
{
    if (!json_is_object(target_obj))
        return 0;

    const char *key;
    json_t *value;
    json_object_foreach(value_obj, key, value)
    {
        json_t *target_value = get_func(target_obj, key);
        if (!target_value)
            return 0;

        if (json_is_object(value) && !json_is_array(value))
        {
            if (!deep_check(value, target_value, get_func))
                return 0;
        }
        else
        {
            if (!json_equal(target_value, value))
                return 0;
        }
    }
    return 1;
}

int has_fields_simple(json_t *obj, json_t *fields)
{
    const char *key;
    json_t *value;
    json_object_foreach(fields, key, value)
    {
        json_t *target = json_object_get(obj, key);
        if (!target)
        {
            return 0;
        }
        if (json_is_object(value) && !json_is_array(value))
        {
            if (!deep_check(value, target, json_object_get))
                return 0;
        }
        else
        {
            if (!json_equal(target, value))
                return 0;
        }
    }
    return 1;
}
