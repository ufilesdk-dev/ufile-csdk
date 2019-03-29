#include "api.h"

#include <curl/curl.h>
#include <string.h>
#include "http.h"

static void 
print_header(struct curl_slist* header){
    struct curl_slist *tmp = header;
    while(tmp != NULL){
        printf("%s\n", tmp->data);
        tmp = tmp->next;
    }
}

struct ufile_error
ufile_put_buf(const char* bucket_name, const char *key, const char *mime_type, char *buffer, size_t buf_len){
    struct ufile_error error = NO_ERROR;
    error = check_bucket_key(bucket_name, key);
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
    }

    struct http_options opt;
    error = set_http_options(&opt, "PUT", mime_type, bucket_name, key, NULL);
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, &opt);
        return error;
    }
    struct http_body body;
    memset(&body, 0, sizeof(struct http_body));
    body.buffer = buffer;
    body.buffer_size = buf_len;
    set_content_length(&opt, body.buffer_size);
    set_curl_options(curl, &opt);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, http_read_cb);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, &body);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)buf_len);

    error = curl_do(curl);
    http_cleanup(curl, &opt);
    return error;
}

struct ufile_error
ufile_put_file(const char* bucket_name, const char *key, const char *mime_type, FILE *file){
    struct ufile_error error = NO_ERROR;
    error = check_bucket_key(bucket_name, key);
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
    }

    struct http_options opt;
    error = set_http_options(&opt, "PUT", mime_type, bucket_name, key, NULL);
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, &opt);
        return error;
    }
    struct http_body body;
    memset(&body, 0, sizeof(struct http_body));
    body.f = file;
    body.buffer = NULL;
    
    fseek(file, 0L, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0L, SEEK_SET);
    set_content_length(&opt, fsize);

    //print_header(opt->header);
    set_curl_options(curl, &opt);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, http_read_cb);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, &body);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);

    error = curl_do(curl);
    http_cleanup(curl, &opt);
    return error;
}