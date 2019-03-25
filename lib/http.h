#ifndef __UFILE_HTTP_H_
#define __UFILE_HTTP_H_

#include <curl/curl.h>
#include "types.h"


struct http_options{
    char *method;
    char * url;
    struct curl_slist *header;
};

struct http_body{
    FILE *f;
    char *buffer;
    size_t fsize;
    //需要读取的总字节数
    size_t need_total_n;
    //已经读取的总字节数
    size_t read_total_n;
};


struct ufile_error
set_http_options(struct http_options **opt, 
                 const char *method,
                 const char *mime_type,
                 const char *bucket,
                 const char *key);

void
set_curl_options(CURL *curl, struct http_options *opt);

struct curl_slist*
set_content_length(struct curl_slist* header, size_t length);

void
http_cleanup(CURL *curl, struct http_options *opt);

struct ufile_error
curl_do(CURL *curl);

size_t http_write_cb(char *ptr, size_t size, size_t nmemb, void *user_data);
size_t http_read_cb(char *buffer, size_t size, size_t nitems, void *user_data);
#endif