#include "http.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "string_util.h"
#include "auth.h"

#define USERAGENT ("UFile CSDK/2.0.0")

extern struct ufile_config *_global_config;
extern int _g_debug_open;

struct ufile_error
set_http_options(struct http_options **opt, 
                 const char *method,
                 const char *mime_type,
                 const char *bucket,
                 const char *key,
                 const char *query) {
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
    if(query != NULL){
        tmp->url = ufile_strconcat(bucket, ".", cfg->file_host, "/", key, "?", query, NULL);
    }else{
        tmp->url = ufile_strconcat(bucket, ".", cfg->file_host, "/", key, NULL);
    }

    *opt = tmp;
    return error;
}

void
set_curl_options(CURL *curl,
                 struct http_options *opt) {
    curl_easy_setopt(curl, CURLOPT_URL, opt->url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, opt->method);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, opt->header);
    if(_g_debug_open != 0){
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
}
void
set_content_length(struct http_options* opt, size_t length){
    char tmp[40] = {0};
    sprintf(tmp, "Content-Length: %ld", length);
    opt->header = curl_slist_append(opt->header, tmp); 
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


size_t http_write_cb(char *ptr, size_t size, size_t nmemb, void *user_data) {
    struct http_body *param = (struct http_body *)user_data;
    if (!param) {
        return 0;
    }
    if(param->f != NULL){
        return fwrite(ptr, size, nmemb, param->f);
    }
    size_t remain_size = param->buffer_size - param->pos_n;
    if(remain_size <= 0){ 
        return 0;//buffer 已经写满。
    }
    size_t curl_writen_size = size*nmemb;
    if(curl_writen_size < remain_size){ //要写的数据还在 buffer 的范围内。
        memcpy(param->buffer+param->pos_n, ptr, curl_writen_size);
        param->pos_n += curl_writen_size;
        return curl_writen_size;
    }else{//要写的数据还在超出了 buffer 的范围，把 buffer 写满即可。
        memcpy(param->buffer+param->pos_n, ptr, remain_size);
        param->pos_n += remain_size;
        return remain_size;
    }
}

size_t http_read_cb(char *ptr, size_t size, size_t nitems, void *user_data) {
    struct http_body *param = (struct http_body*)user_data;
    if (!param) {
        return 0;
    }

    if(param->f != NULL){
        size_t nc = fread(ptr, size, nitems, param->f);
        return nc;
    }

    size_t remain_size = param->buffer_size - param->pos_n;
    if(remain_size <= 0){
        return 0; //buffer 已经读完。
    }
    size_t curl_need_bytes = size*nitems;
    if(curl_need_bytes < remain_size){ //要读取的数据还在 buffer 范围内
        memcpy(ptr, param->buffer+param->pos_n, curl_need_bytes);
        param->pos_n += curl_need_bytes;
        return curl_need_bytes;
    }else{ //要读的数据超出了 buffer 的范围，把剩余的 buffer 读完。
        memcpy(ptr, param->buffer+param->pos_n, remain_size);
        param->pos_n += remain_size;
        return remain_size;
    }
}

void
set_escaped_url(CURL *curl, struct http_options *opt, const char* query){
    char* escaped_str = curl_easy_escape(curl, query, 0);
    char *t = opt->url;
    opt->url = ufile_strconcat(opt->url, "?", query, NULL);
    free(t);
    curl_free(escaped_str);
}