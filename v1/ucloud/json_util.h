#ifndef _UFILESDK_C_UCLOUD_API_JSION_UTIL_
#define _UFILESDK_C_UCLOUD_API_JSION_UTIL_
#include <json-c/json.h>
#include <string.h>

int get_string_from_json_object(const json_object *root, const char *key, char *value);

int get_string_from_data(const char *data, const char *key, char *value);

int get_int64_from_json_object(const json_object *root, const char *key, int64_t *value);

int get_int64_from_data(const char *data, const char *key, int64_t *value);

int get_sizet_from_json_object(const json_object *root, const char *key, size_t *value);

int get_sizet_from_data(const char *data, const char *key, size_t *value);
#endif