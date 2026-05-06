#include "array_helpers.h"

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
