#ifndef ID_H
#define ID_H

#include <jansson.h>
#include <stdint.h>

int64_t convert_id_to_unix(const char *id);
int compare_ids(const json_t *a, const json_t *b);

#endif
