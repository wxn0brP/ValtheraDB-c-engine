#ifndef CONDITIONS_H
#define CONDITIONS_H

#include <jansson.h>

int check_condition(json_t *obj, const char *op, json_t *condition_obj);

#endif
