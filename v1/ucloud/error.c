#include "error.h"

ret_status_t *error_desc(const int ret_code) {
    int i = 0;
    while (ret_status[i].ret_code != 0) {
        if (ret_status[i].ret_code == ret_code) {
            return &(ret_status[i]);
        }
        i++;
    }
    return NULL;
}

ret_status_t *error_desc_http(int retcode, const char *error_message) {
    ret_http_status.ret_code = retcode;
    memset(ret_http_status.error_message, 0, 128);
    strncpy(ret_http_status.error_message, error_message, 128);
    return &ret_http_status;
}