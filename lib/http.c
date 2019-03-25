#include "http.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "string_util.h"
#include "auth.h"

#define USERAGENT ("UFile CSDK/2.0.0")

extern struct ufile_config *_global_config;

struct ufile_error
set_http_options(struct http_options **opt, 
                 const char *method,
                 const char *mime_type,
                 const char *bucket,
                 const char *key) {
    struct ufile_error error = NO_ERROR;
    struct ufile_config *cfg = _global_config;
    if (cfg == NULL){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "global configuration has not been initialization yet.";
        return error;
    }
    struct http_options *tmp = (struct http_options*)malloc(sizeof(struct http_options));
    tmp->header = NULL;

    char *buf;
    if (strlen(mime_type) != 0){
        buf = ufile_strconcat("Content-Type: ", mime_type);
        tmp->header = curl_slist_append(tmp->header, buf);
        free(buf);
    }
    tmp->header = curl_slist_append(tmp->header, "User-Agent: UFile CSDK/2.0.0");

    char *auth = ufile_file_authorization(cfg->public_key,cfg->private_key,method, bucket, key, mime_type,"","");
    buf = ufile_strconcat("Authorization: ", auth, NULL); 
    tmp->header = curl_slist_append(tmp->header, buf);
    free(auth); 
    free(buf);

    tmp->method = ufile_strconcat(method, NULL);
    tmp->url = ufile_strconcat(bucket, ".", cfg->file_host, "/", key, NULL);

    *opt = tmp;
    return error;
}

void
set_curl_options(CURL *curl,
                 struct http_options *opt) {
    curl_easy_setopt(curl, CURLOPT_URL, opt->url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, opt->method);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, opt->header);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
}

void
http_cleanup(CURL *curl,
             struct http_options *opt){
    curl_easy_cleanup(curl);
    if (opt != NULL){
        if(opt->method != NULL){
            free(opt->method);
        }
        if(opt->url != NULL){
            free(opt->url);
        }
        if(opt->header != NULL){
            curl_slist_free_all(opt->header);
        }
        free(opt);
    }
}

struct ufile_error
curl_do(CURL *curl){
    struct ufile_error error = NO_ERROR;
    CURLcode curl_code = curl_easy_perform(curl);
    if (curl_code != CURLE_OK) {
        error.code = CURL_ERROR_CODE;
        error.message = curl_easy_strerror(curl_code);
        return error;
    }

    long response_code;
    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code) != CURLE_OK){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_RESPONSE_CODE_ERROR_MSG;
        return error;
    }

    if (!HTTP_IS_OK(response_code)){
        error.message = HTTP_ERROR_MSG;
    }
    error.code = response_code;
    return error;
}


static size_t writen(FILE *f, char *buffer, char *ptr, const size_t total) {

    int    fd = -1;
    size_t left = total;
    int nw = 0;
    while(left > 0) {
        //优先使用文件对象
        if (f) {
            if (fd == -1) fd = fileno(f);
            nw = write(fd, ptr+total-left, left);
        } else {
			strcat(buffer, ptr+total-left);
            left = 0;
        }

        if (nw <= 0) return total-left;
        left -= nw;
    }
    return total-left;
}

size_t http_write_cb(char *ptr, size_t size, size_t nmemb, void *user_data) {
    struct http_body *param = (struct http_body *)user_data;
    if (!param) {
        return 0;
    }
    return writen(param->f, param->buffer, (char *)ptr, size*nmemb);
}

size_t http_read_cb(char *buffer, size_t size, size_t nitems, void *user_data) {
    struct http_body *param = (struct http_body*)user_data;
    if (!param) {
        return 0;
    }

    if(param->f != NULL){
        size_t nc =  fread(buffer, size, nitems, param->f);
        return nc;
    }
    size_t curl_need_bytes = size*nitems;
    if(curl_need_bytes > param->need_total_n){
        memcpy(buffer, param->buffer, param->need_total_n);
        param->read_total_n = param->need_total_n;
        return param->need_total_n;
    }else{
        size_t remain_bytes = param->need_total_n - param->read_total_n;
        if(remain_bytes > curl_need_bytes){
            memcpy(buffer, param->buffer+param->read_total_n, curl_need_bytes);
            param->read_total_n+=curl_need_bytes;
            return curl_need_bytes;
        }else{
            memcpy(buffer, param->buffer+param->read_total_n, remain_bytes);
            param->read_total_n+=remain_bytes;
            return remain_bytes;
        }
    }
}

struct curl_slist*
set_content_length(struct curl_slist* header, size_t length){
    char tmp[40] = {0};
    sprintf(tmp, "Content-Length: %ld", length);
    header = curl_slist_append(header, tmp); 
    return header;
}