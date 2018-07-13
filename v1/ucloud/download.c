#include "api.h"
#include "common.h"
#include "error.h"
#include "digest.h"
#include "http.h"
#include "json_util.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <json-c/json.h>
#include <errno.h>

static ret_status_t *open_file(FILE **pf, const char *file_path) {

    ret_status_t *ret_status = NULL;
    *pf = fopen(file_path, "w+");
    if (*pf == NULL) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        return ret_status;
    }
    return ret_status;
}

ret_status_t *download(const char *bucket, 
                               const char *key, 
                               const char *file_path, 
                               const range_t *range) {

    ret_status_t *ret_status = NULL;
    struct curl_slist *req_headers = NULL;
    http_options_t http_options;
    char header[256];
    char header_token[256];
    char error_message[128];
    char response_buf[1024];
    CURL *curl;
    FILE *pf = NULL;
    int64_t ret = 0;
    int parse_ret = 0;
    long code = 200;

    http_write_param_t http_write_param;

    memset(http_options.method, 0, sizeof(http_options.method));
    memset(http_options.url, 0, sizeof(http_options.url));
    memset(error_message, 0, 128);
    memset(response_buf, 0, 1024);
    memset(header, 0, 256);

    //初始化配置文件
    ret_status = init_global_config();
    if (ret_status) return ret_status;

    curl = http_request_get();

    ret_status = open_file(&pf, file_path);
    if (ret_status) return  ret_status;

    memset(header, 0, 256);
    snprintf(header, 256, "%s:%s", "User-Agent", USERAGENT);
    http_add_header(header, &req_headers);

    if (range) {
        memset(header, 0, 256);
        snprintf(header, 256, "Range:bytes=%zd-%zd", range->begin, range->end);
        http_add_header(header, &req_headers);
    }

    ufile_host(bucket, http_options.url);
    sprintf(http_options.url, "%s%s",  http_options.url, key);

    memset(header, 0, 256);
    ret = token(req_headers,
                "GET",
                HEAD_FIELD_CHECK,
                bucket,
                key,
                NULL,
                header_token,
				NULL);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        fclose(pf);
        return ret_status;
    }

    snprintf(header, 256, "%s:%s", "Authorization", header_token);
    http_add_header(header, &req_headers);
    http_options.req_headers = req_headers;
    http_options.curl = curl;

    ret_status = http_set_options(http_options);
    if (ret_status != NULL) {
        fclose(pf);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    http_write_param.f = pf;
    http_write_param.buffer = NULL;
    ret_status = http_round_trip(curl, NULL, &http_write_param, NULL);
    if (ret_status) {
        fclose(pf);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    ret = http_respons_eode(curl, &code);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CURL);
        fclose(pf);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    if (code != 200 && code != 206) {
        fseek(pf, 0, SEEK_SET);
        fgets(response_buf, 1024, pf);
        parse_ret = ufile_error_response(response_buf, &ret, error_message);
        if (parse_ret) {
             UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
             http_cleanup_curl(curl, req_headers);
             fclose(pf);
             return ret_status;
        }
        UFILE_SET_ERROR2(ret, error_message);
        http_cleanup_curl(curl, req_headers);
        fclose(pf);
        return ret_status;
    }
    http_cleanup_curl(curl, req_headers);
    fclose(pf);
    return ret_status;
}
