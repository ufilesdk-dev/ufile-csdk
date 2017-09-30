#include "api.h"
#include "common.h"
#include "error.h"
#include "digest.h"
#include "http.h"
#include "json_util.h"
#include "config.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <json-c/json.h>

ret_status_t *delete(const char *bucket, const char *key) {

    ret_status_t *ret_status = NULL;
    struct curl_slist *req_headers = NULL;
    http_options_t http_options;
    char header[256];
    char header_token[256];
    char response_buf[1024];
    char error_message[128];
    CURL *curl;
    int64_t ret = 0;
    int parse_ret = 0;
    long code = 204;
    http_write_param_t http_write_param;

    memset(header, 0, 64);
    memset(http_options.method, 0, sizeof(http_options.method));
    memset(http_options.url, 0, sizeof(http_options.url));
    memset(response_buf, 0, 1024);
    memset(error_message, 0, 128);
    memset(header, 0, 256);
    memset(header_token, 0, 256);

    //初始化配置文件
    ret_status = init_global_config();
    if (ret_status) return ret_status;

    curl = http_request_get();

    snprintf(header, 256, "%s:%s", "User-Agent", USERAGENT);
    http_add_header(header, &req_headers);

    sprintf(http_options.method, "%s", "DELETE");
    ufile_host(bucket, http_options.url);
    sprintf(http_options.url, "%s%s", http_options.url, key);

    memset(header, 0, 256);
    ret = token(req_headers,
                "DELETE",
                HEAD_FIELD_CHECK,
                bucket,
                key,
                NULL,
                header_token);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        return ret_status;
    }

    snprintf(header, 256, "%s:%s", "Authorization", header_token);
    http_add_header(header, &req_headers);
    http_options.req_headers = req_headers;
    http_options.curl = curl;
    
    ret_status = http_set_options(http_options);
    if (ret_status != NULL) {
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    http_write_param.f = NULL;
    http_write_param.buffer = (char*)response_buf;
    ret_status = http_round_trip(curl, NULL, &http_write_param, NULL);
    if (ret_status) {
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    
    ret = http_respons_eode(curl, &code);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CURL);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    if (code != 204) {
        parse_ret = ufile_error_response(response_buf, &ret, error_message);
        if (parse_ret) {
             UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
             http_cleanup_curl(curl, req_headers);
             return ret_status;
        }
        UFILE_SET_ERROR2(ret, error_message);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    http_cleanup_curl(curl, req_headers);
    return ret_status;
}