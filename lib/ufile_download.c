#include "api.h"

#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

#include "http.h"

static struct ufile_error download(const char *bucket_name,
         const char *key,
         size_t start_position,
         FILE *f,
         char *buf,
         size_t buf_len,
         size_t *return_size)
{
    struct ufile_error error = NO_ERROR;
    error = check_bucket_key(bucket_name, key);
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }

    CURL *curl = curl_easy_init(); // easy模式：阻塞； multi模式：非阻塞(使用curl_multi_init)
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    // 设置dns共享数据
    static CURLSH* share_handle = NULL; // 使用静态变量
    if (!share_handle)
    {  
        share_handle = curl_share_init();  // curl 共享句柄：作用是允许curl句柄间共享数据
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS); // 设置需要共享的数据，CURLSHOPT_SHARE表示设置dns缓存共享
    }  
    curl_easy_setopt(curl, CURLOPT_SHARE, share_handle);  // CURLOPT_SHARE：表示curl使用share_handle内的数据
    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 5); // 缓存过期时间 300s 默认为120秒
    
    
    size_t start_pos=start_position;
    size_t end_pos=0;
    if(buf_len > 0){
        end_pos = start_position+buf_len-1;
    }

    error = set_download_options(curl, bucket_name, key, start_position, end_pos);
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


struct ufile_error ufile_download(const char *bucket_name, const char *key, FILE *file, size_t *return_size){
    return download(bucket_name, key, 0, file, NULL, 0, return_size);
}

struct ufile_error ufile_download_piece(const char *bucket_name, const char *key, size_t start_position, char *buf, size_t buf_len, size_t *return_size){
    return download(bucket_name, key, start_position, NULL, buf, buf_len, return_size);
}