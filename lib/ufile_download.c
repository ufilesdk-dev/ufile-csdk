#include "api.h"

#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

#include "http.h"

static struct ufile_error
download(const char *bucket,
         const char *key,
         size_t start_position,
         FILE *f,
         char *buf,
         size_t buf_len,
         size_t *return_size)
{
    struct ufile_error error = NO_ERROR;
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }
    size_t start_pos=start_position;
    size_t end_pos=start_position+buf_len-1;

    error = set_download_options(curl, bucket, key, start_position, start_position+buf_len);
    if(UFILE_HAS_ERROR(error.code)){
        curl_easy_cleanup(curl);
        return error;
    }

    struct http_body body;
    memset(&body, 0, sizeof(struct http_body));
    body.buffer = buf;
    if(body.buffer != NULL){
        body.buffer_size = buf_len;
        body.pos_n = 0;
    }
    body.f = f;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

    struct curl_slist *header=NULL; 
    if((end_pos - start_pos) > 0){
        char range[64]={0};
        sprintf(range, "Range: bytes=%ld-%ld", start_pos, end_pos);
        header = curl_slist_append(header, range);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    }

    error = curl_do(curl);
    if(return_size != NULL){
        *return_size = body.pos_n;
    }
    if((end_pos - start_pos) > 0){
        curl_slist_free_all(header);
    }
    curl_easy_cleanup(curl);
    return error;
}
struct ufile_error
ufile_download(const char *bucket, const char *key, FILE *file, size_t *return_size){
    return download(bucket, key, 0, file, NULL, 0, return_size);
}
struct ufile_error
ufile_download_piece(const char *bucket, const char *key, size_t start_position, char *buf, size_t buf_len, size_t *return_size){
    return download(bucket, key, start_position, NULL, buf, buf_len, return_size);
}