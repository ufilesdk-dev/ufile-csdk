#include "common.h"

void ufile_host(const char *bucket, char *host) {
    
    if (host == NULL) return;
    sprintf(host, "%s%s%s", SCHEME, bucket, ucloud_host_suffix);
    if (host[strlen(host) - 1] != '/') {
        host[strlen(host)] = '/';
    }
}

int ufile_error_response(const char *body, int64_t *ret_code, char *error_message) {

    json_object *root = json_tokener_parse(body);
    if (!root) return -1;

    int ret = get_int64_from_json_object(root, "RetCode", ret_code);
    if (ret) {
        json_object_put(root);
        return ret;
    }

    ret = get_string_from_json_object(root, "ErrMsg", error_message);
    if (ret) {
        json_object_put(root);
        return ret;
    }

    json_object_put(root);
    return 0;
}

int fetch_content_length(FILE *pf, long *length) {

    long curpos = ftell(pf);
    if (fseek(pf, 0, SEEK_END)) {
        return -1;
    }

    *length = ftell(pf);

    if (fseek(pf, curpos, SEEK_SET)) {
        return -1;
    }
    return 0;
}