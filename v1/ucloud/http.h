#ifndef _UFILESDK_C_UCLOUD_HTTP_
#define _UFILESDK_C_UCLOUD_HTTP_

#include "string_util.h"
#include "error.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stddef.h>

#if defined(_WIN32)
#pragma comment(lib, "curllib.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SCHEME ("http://")

typedef struct {
    FILE *f;
    char *buffer;
}http_write_param_t;

typedef struct {
    FILE *f;
    char *buffer;
    size_t fsize;
    //需要读取的总字节数
    size_t need_total_n;
    //已经读取的总字节数
    size_t read_total_n;
}http_read_param_t;

typedef struct {
    FILE *f;
    char *buffer;
    //需要读取的总字节数
    size_t need_total_n;
    //已经读取的总字节数
    size_t read_total_n;
}http_header_param_t;

typedef struct {
    CURL *curl;
    char method[32];
    char url[1024];
    struct curl_slist *req_headers;
}http_options_t;

typedef struct {
    ssize_t begin;
    ssize_t end;
}range_t;

//只调用一次
ret_status_t *http_initialize();
void http_cleanup();

CURL *http_request_get();

//ret_status_t *http_set_options(const http_options_t http_options);
ret_status_t *http_set_options(http_options_t http_options);

void http_add_header(const char *key_value, struct curl_slist **req_headers);
char *http_request_header(const char *k, struct curl_slist *req_headers);

ret_status_t *http_round_trip(CURL *curl,
                              const http_read_param_t *read_param, 
                              const http_write_param_t *write_param, 
                              const http_header_param_t *header_param);

int http_respons_eode(CURL *curl, long *code);
void http_cleanup_curl(CURL *curl, struct curl_slist *req_headers);


size_t write_cb(char *ptr, size_t size, size_t nmemb, void *user_data);
size_t read_cb(char *buffer, size_t size, size_t nitems, void *user_data);
size_t write_header_cb(void *ptr, size_t size, size_t nmemb, void *user_data);
size_t writen(FILE *f, char *buffer, char *ptr, const size_t total);

#define curl_easy_setopt_safe(curl, opt, val)                                                 \
    if ((ret_code = curl_easy_setopt(curl, opt, val)) != CURLE_OK) {      \
            UFILE_SET_ERROR2(ERR_CSDK_SET_CURLOPT, curl_easy_strerror(ret_code));       \
            return ret_status;                                                          \
            }  

#ifdef __cplusplus
}
#endif

#endif

