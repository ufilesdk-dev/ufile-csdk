#ifndef _H_UFILESDK_C_UCLOUD_API_COMMON_
#define _H_UFILESDK_C_UCLOUD_API_COMMON_

#include "json_util.h"
#include "config.h"
#include "error.h"

#include <json-c/json.h>
#include <string.h>

#define SCHEME ("http://")


typedef struct {
    size_t blk_idx;
    char etag[64];
}etag_t;

void ufile_host(const char *bucket, char *host);

int ufile_error_response(const char *body, int64_t *ret_code, char *error_message);

int fetch_content_length(FILE *pf, long *length);
#endif