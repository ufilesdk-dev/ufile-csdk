#include "api.h"

ret_status_t *csdk_initialize() {
    ret_status_t *ret_status = NULL;
    ret_status = http_initialize();
    return ret_status;
}

ret_status_t *csdk_cleanup() {
    http_cleanup();
    return NULL;
}