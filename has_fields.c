#include "has_fields.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

int compare_ids(const char *id1, const char *id2)
{
    return strcmp(id1, id2);
}

int value_in_array(json_t *array, json_t *value)
{
    size_t index;
    json_t *element;
    json_array_foreach(array, index, element)
    {
        if (json_equal(element, value))
        {
            return 1;
        }
    }
    return 0;
}

int all_values_in_array(json_t *array, json_t *values)
{
    size_t index;
    json_t *value;
    json_array_foreach(values, index, value)
    {
        if (!value_in_array(array, value))
        {
            return 0;
        }
    }
    return 1;
}

int any_values_in_array(json_t *array, json_t *values)
{
    size_t index;
    json_t *value;
    json_array_foreach(values, index, value)
    {
        if (value_in_array(array, value))
        {
            return 1;
        }
    }
    return 0;
}

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

int check_condition(json_t *obj, const char *op, json_t *condition_obj)
{
    const char *key;
    json_t *value;
    json_object_foreach(condition_obj, key, value)
    {
        json_t *target_value = json_object_get(obj, key);

        if (strcmp(op, "$gt") == 0)
        {
            if (json_is_number(target_value))
            {
                if (json_typeof(target_value) == JSON_INTEGER && json_typeof(value) == JSON_INTEGER)
                {
                    if (!(json_integer_value(target_value) > json_integer_value(value)))
                        return 0;
                }
                else if (json_typeof(target_value) == JSON_REAL || json_typeof(value) == JSON_REAL)
                {
                    double t = json_typeof(target_value) == JSON_INTEGER ? json_integer_value(target_value) : json_real_value(target_value);
                    double v = json_typeof(value) == JSON_INTEGER ? json_integer_value(value) : json_real_value(value);
                    if (!(t > v))
                        return 0;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$lt") == 0)
        {
            if (json_is_number(target_value))
            {
                if (json_typeof(target_value) == JSON_INTEGER && json_typeof(value) == JSON_INTEGER)
                {
                    if (!(json_integer_value(target_value) < json_integer_value(value)))
                        return 0;
                }
                else if (json_typeof(target_value) == JSON_REAL || json_typeof(value) == JSON_REAL)
                {
                    double t = json_typeof(target_value) == JSON_INTEGER ? json_integer_value(target_value) : json_real_value(target_value);
                    double v = json_typeof(value) == JSON_INTEGER ? json_integer_value(value) : json_real_value(value);
                    if (!(t < v))
                        return 0;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$gte") == 0)
        {
            if (json_is_number(target_value))
            {
                if (json_typeof(target_value) == JSON_INTEGER && json_typeof(value) == JSON_INTEGER)
                {
                    if (!(json_integer_value(target_value) >= json_integer_value(value)))
                        return 0;
                }
                else if (json_typeof(target_value) == JSON_REAL || json_typeof(value) == JSON_REAL)
                {
                    double t = json_typeof(target_value) == JSON_INTEGER ? json_integer_value(target_value) : json_real_value(target_value);
                    double v = json_typeof(value) == JSON_INTEGER ? json_integer_value(value) : json_real_value(value);
                    if (!(t >= v))
                        return 0;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$lte") == 0)
        {
            if (json_is_number(target_value))
            {
                if (json_typeof(target_value) == JSON_INTEGER && json_typeof(value) == JSON_INTEGER)
                {
                    if (!(json_integer_value(target_value) <= json_integer_value(value)))
                        return 0;
                }
                else if (json_typeof(target_value) == JSON_REAL || json_typeof(value) == JSON_REAL)
                {
                    double t = json_typeof(target_value) == JSON_INTEGER ? json_integer_value(target_value) : json_real_value(target_value);
                    double v = json_typeof(value) == JSON_INTEGER ? json_integer_value(value) : json_real_value(value);
                    if (!(t <= v))
                        return 0;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$in") == 0)
        {
            if (!value_in_array(value, target_value))
                return 0;
        }
        else if (strcmp(op, "$nin") == 0)
        {
            if (value_in_array(value, target_value))
                return 0;
        }
        else if (strcmp(op, "$exists") == 0)
        {
            int should_exist = json_boolean_value(value);
            int exists = json_object_get(obj, key) != NULL;
            if ((should_exist && !exists) || (!should_exist && exists))
                return 0;
        }
        else if (strcmp(op, "$type") == 0)
        {
            const char *expected_type = json_string_value(value);
            const char *actual_type = NULL;
            switch (json_typeof(target_value))
            {
            case JSON_OBJECT:
                actual_type = "object";
                break;
            case JSON_ARRAY:
                actual_type = "array";
                break;
            case JSON_STRING:
                actual_type = "string";
                break;
            case JSON_INTEGER:
                actual_type = "integer";
                break;
            case JSON_REAL:
                actual_type = "real";
                break;
            case JSON_TRUE:
            case JSON_FALSE:
                actual_type = "boolean";
                break;
            case JSON_NULL:
                actual_type = "null";
                break;
            }
            if (actual_type == NULL || strcmp(actual_type, expected_type) != 0)
                return 0;
        }
        else if (strcmp(op, "$regex") == 0)
        {
            fprintf(stderr, "Regex is not implemented in C version.\n");
            return 0;
        }
        else if (strcmp(op, "$size") == 0)
        {
            if (json_is_array(target_value))
            {
                if (json_array_size(target_value) != (size_t)json_integer_value(value))
                    return 0;
            }
            else if (json_is_string(target_value))
            {
                if (strlen(json_string_value(target_value)) != (size_t)json_integer_value(value))
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$startswith") == 0)
        {
            if (json_is_string(target_value))
            {
                const char *s = json_string_value(target_value);
                const char *prefix = json_string_value(value);
                if (strncmp(s, prefix, strlen(prefix)) != 0)
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$endswith") == 0)
        {
            if (json_is_string(target_value))
            {
                const char *s = json_string_value(target_value);
                const char *suffix = json_string_value(value);
                size_t len_s = strlen(s);
                size_t len_suffix = strlen(suffix);
                if (len_s < len_suffix || strcmp(s + (len_s - len_suffix), suffix) != 0)
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$between") == 0)
        {
            if (json_is_number(target_value))
            {
                double val = json_typeof(target_value) == JSON_INTEGER ? json_integer_value(target_value) : json_real_value(target_value);
                json_t *min_val = json_array_get(value, 0);
                json_t *max_val = json_array_get(value, 1);
                double min_d = json_typeof(min_val) == JSON_INTEGER ? json_integer_value(min_val) : json_real_value(min_val);
                double max_d = json_typeof(max_val) == JSON_INTEGER ? json_integer_value(max_val) : json_real_value(max_val);
                if (val < min_d || val > max_d)
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$arrinc") == 0)
        {
            if (json_is_array(target_value))
            {
                if (!any_values_in_array(target_value, value))
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$arrincall") == 0)
        {
            if (json_is_array(target_value))
            {
                if (!all_values_in_array(target_value, value))
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$idgt") == 0)
        {
            if (json_is_string(target_value))
            {
                const char *id1 = json_string_value(target_value);
                const char *id2 = json_string_value(value);
                if (compare_ids(id1, id2) <= 0)
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$idlt") == 0)
        {
            if (json_is_string(target_value))
            {
                const char *id1 = json_string_value(target_value);
                const char *id2 = json_string_value(value);
                if (compare_ids(id1, id2) >= 0)
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$idgte") == 0)
        {
            if (json_is_string(target_value))
            {
                const char *id1 = json_string_value(target_value);
                const char *id2 = json_string_value(value);
                if (compare_ids(id1, id2) < 0)
                    return 0;
            }
            else
                return 0;
        }
        else if (strcmp(op, "$idlte") == 0)
        {
            if (json_is_string(target_value))
            {
                const char *id1 = json_string_value(target_value);
                const char *id2 = json_string_value(value);
                if (compare_ids(id1, id2) > 0)
                    return 0;
            }
            else
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

int has_fields_advanced(json_t *obj, json_t *fields)
{
    if (!json_is_object(fields))
    {
        fprintf(stderr, "Fields must be an object\n");
        exit(1);
    }

    if (json_object_size(fields) == 0)
        return 1;

    if (json_object_get(fields, "$and"))
    {
        json_t *and_array = json_object_get(fields, "$and");
        if (!json_is_array(and_array))
            return 0;
        size_t index;
        json_t *sub_fields;
        json_array_foreach(and_array, index, sub_fields)
        {
            if (!has_fields_advanced(obj, sub_fields))
                return 0;
        }
        return 1;
    }

    if (json_object_get(fields, "$or"))
    {
        json_t *or_array = json_object_get(fields, "$or");
        if (!json_is_array(or_array))
            return 0;
        size_t index;
        json_t *sub_fields;
        json_array_foreach(or_array, index, sub_fields)
        {
            if (has_fields_advanced(obj, sub_fields))
                return 1;
        }
        return 0;
    }

    json_t *conditions = json_object();
    json_t *simple_fields = json_object();

    const char *key;
    json_t *value;
    json_object_foreach(fields, key, value)
    {
        if (key[0] == '$')
        {
            json_object_set(conditions, key, value);
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
