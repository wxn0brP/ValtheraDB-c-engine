#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "conditions.h"
#include "array_helpers.h"
#include "utils/id.h"

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
            if (actual_type == NULL)
                return 0;
            if (strcmp(expected_type, "number") == 0)
            {
                if (strcmp(actual_type, "integer") != 0 && strcmp(actual_type, "real") != 0)
                    return 0;
            }
            else if (strcmp(actual_type, expected_type) != 0)
                return 0;
        }
        else if (strcmp(op, "$regex") == 0)
        {
            if (!json_is_string(target_value) || !json_is_string(value))
                return 0;
            const char *text = json_string_value(target_value);
            const char *pattern = json_string_value(value);
            regex_t regex;
            int ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
            if (ret != 0)
                return 0;
            ret = regexec(&regex, text, 0, NULL, 0);
            regfree(&regex);
            if (ret != 0)
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
            return compare_ids(target_value, value) > 0;
        }
        else if (strcmp(op, "$idlt") == 0)
        {
            return compare_ids(target_value, value) < 0;
        }
        else if (strcmp(op, "$idgte") == 0)
        {
            return compare_ids(target_value, value) >= 0;
        }
        else if (strcmp(op, "$idlte") == 0)
        {
            return compare_ids(target_value, value) <= 0;
        }
    }
    return 1;
}
