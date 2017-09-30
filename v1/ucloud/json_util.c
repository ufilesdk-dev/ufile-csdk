#include "json_util.h"

int get_string_from_json_object(const json_object *root, const char *key, char *value) {
    json_object *node = NULL;
    const char *result = NULL;
    if (!root) {
         return -1;
    }

    if (json_object_object_get_ex((json_object *)root, key, &node)) {
        result = json_object_get_string(node);
        if (result == NULL) return -1;
        strcpy(value, result);
        return 0;
    }
    return -1;
}

int get_string_from_data(const char *data, const char *key, char *value) {
    int ret = 0;
    json_object *root = json_tokener_parse(data);
    if (!root) {
        return -1;
    }

    ret = get_string_from_json_object(root, key, value);
    json_object_put(root);
    return ret;
} 

int get_int64_from_json_object(const json_object *root, const char *key, int64_t *value) {
    json_object *node = NULL;
    if (!root) {
        return -1;
    }
    
    if (json_object_object_get_ex((json_object *)root, key, &node)) {
        *value = json_object_get_int64(node);
        return 0;
    }
    //不存在
    return -1;
}

int get_int64_from_data(const char *data, const char *key, int64_t *value) {
    int ret = 0;
    json_object *root = json_tokener_parse(data);
    if (!root) {
        return -1;
    }

    ret = get_int64_from_json_object(root, key, value);
    json_object_put(root);
    return ret;   
}

int get_sizet_from_json_object(const json_object *root, const char *key, size_t *value) {

    int64_t v;
    int ret = get_int64_from_json_object(root, key, &v);
    if (ret == 0) {
        *value = v;
    }
    return ret;
}

int get_sizet_from_data(const char *data, const char *key, size_t *value) {

    json_object *root = json_tokener_parse(data);
    if (!root) {
        return -1;
    }

    int ret = get_sizet_from_json_object(root, key, value);
    json_object_put(root);
    return ret;   
}