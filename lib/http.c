#include "http.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include "string_util.h"
#include "auth.h"
#include "encoding.h"
#include "api.h"


#define USERAGENT ("UFile CSDK/2.0.0")
#define HTTP_IS_OK(CODE) ((CODE)/100 == 2)

extern struct ufile_config *_global_config;
extern int _g_debug_open;

struct ufile_error set_http_options(struct http_options *opt, 
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

    if(strlen(bucket) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "bucket connot be empty";
        return error;
    }

    if(strlen(key) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "key connot be empty";
        return error;
    }

    opt->header = NULL;
    char *buf;
    if (strlen(mime_type) != 0){
        buf = ufile_strconcat("Content-Type: ", mime_type, NULL);
        opt->header = curl_slist_append(opt->header, buf);
        free(buf);
    }
    opt->header = curl_slist_append(opt->header, "User-Agent: UFile CSDK/2.0.0");

    char *auth = ufile_file_authorization(cfg->public_key,cfg->private_key,method, bucket, key, mime_type,"","");
    buf = ufile_strconcat("Authorization: ", auth, NULL); 
    opt->header = curl_slist_append(opt->header, buf);
    free(auth); 
    free(buf);

    opt->method = ufile_strconcat(method, NULL);
    if(query != NULL){
        opt->url = ufile_strconcat(bucket, ".", cfg->file_host, "/", key, "?", query, NULL);
    }else{
        opt->url = ufile_strconcat(bucket, ".", cfg->file_host, "/", key, NULL);
    }

    return error;
}

void set_curl_options(CURL *curl,struct http_options *opt) {
    curl_easy_setopt(curl, CURLOPT_URL, opt->url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, opt->method);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, opt->header);
    if(_g_debug_open != 0){
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
}

void set_content_length(struct http_options* opt, size_t length){
    char tmp[40] = {0};
    sprintf(tmp, "Content-Length: %ld", length);
    opt->header = curl_slist_append(opt->header, tmp); 
}

void http_cleanup(CURL *curl, struct http_options *opt){
    if(curl != NULL){
        curl_easy_cleanup(curl);
    }
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
    }
}

size_t http_write_cb(char *ptr, size_t size, size_t nmemb, void *user_data) {
    struct http_body *param = (struct http_body *)user_data;
    if (!param) {
        return 0;
    }
    if(param->f != NULL){
        size_t nc = fwrite(ptr, size, nmemb, param->f);
        param->pos_n += nc;
        return nc;
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
    }else{//要写的数据超出了 buffer 的范围，把 buffer 写满即可。
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
        param->pos_n += nc;
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

void set_escaped_url(CURL *curl, struct http_options *opt, const char* query){
    char* escaped_str = curl_easy_escape(curl, query, 0);
    char *t = opt->url;
    opt->url = ufile_strconcat(opt->url, "?", query, NULL);
    free(t);
    curl_free(escaped_str);
}

struct ufile_error set_download_options(CURL *curl,
                    const char* bucket,
                    const char* key,
                    size_t start_pos,
                    size_t end_pos) {
    struct ufile_error error = NO_ERROR;
    struct ufile_config *cfg = _global_config;

    if (cfg == NULL){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "global configuration has not been initialization yet.";
        return error;
    }

    if(strlen(bucket) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "bucket connot be empty";
        return error;
    }

    if(strlen(key) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "key connot be empty";
        return error;
    }

    char tmp[64]={0};
    long long now = (long long)time(NULL);
    now += 24*60*60;  //tomorrow
    sprintf(tmp, "%lld", now);
    char *signature = ufile_download_authorization(cfg->private_key, bucket, key, "GET", tmp, "", "");

    char escaped_pubkey[128]={0};
    query_escape(escaped_pubkey, cfg->public_key, 0);

    char url[512] = {0};
    sprintf(url, "%s.%s/%s?UCloudPublicKey=%s&Signature=%s&Expires=%lld", bucket, _global_config->file_host, 
                      key, escaped_pubkey,signature, now);
    free(signature);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

    if(_g_debug_open != 0){
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    return error;
}

struct ufile_error check_bucket_key(const char *bucket_name, const char *key){
    struct ufile_error error=NO_ERROR;
    if(!bucket_name || *bucket_name == '\0'){
        error.code = UFILE_PARAM_ERROR_CODE;
        error.message = "bucket_name cannot be nil.";
        return error;
    }

    if(!key || *key == '\0'){
        error.code = UFILE_PARAM_ERROR_CODE;
        error.message = "key cannot be nil.";
        return error;
    }
    return error;
}


pthread_rwlock_t rwlock;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 临界区

struct ufile_error curl_do(CURL *curl)
{
    static CURLSH* share_handle = NULL;
    if (share_handle == NULL) {
        if(pthread_mutex_lock(&mutex) != 0) // 临界区，防止对rwlock和share_handle多次初始化
        {
            perror("pthread_mutex_lock");
            exit(EXIT_FAILURE);
        }
        if (share_handle == NULL) {
            pthread_rwlock_init(&rwlock, NULL); 
            share_handle = curl_share_init();  // curl 共享句柄：作用是允许curl句柄间共享数据
            curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS); // 设置需要共享的数据，CURLSHOPT_SHARE表示设置dns缓存共享
            curl_share_setopt(share_handle, CURLSHOPT_LOCKFUNC, lock_cb);
            curl_share_setopt(share_handle, CURLSHOPT_UNLOCKFUNC, unlock_cb);  
        }
        if(pthread_mutex_unlock(&mutex) != 0)
        {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }
    }
    curl_easy_setopt(curl, CURLOPT_SHARE, share_handle);  // CURLOPT_SHARE：表示curl使用share_handle内的数据

    struct ufile_error error = NO_ERROR;
    // struct timeval start, end;
    // gettimeofday( &start, NULL );
    CURLcode curl_code = curl_easy_perform(curl);
    if (curl_code != CURLE_OK) {
        error.code = CURL_ERROR_CODE;
        error.message = curl_easy_strerror(curl_code);
        return error;
    }
    // gettimeofday( &end, NULL );
    // int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec; 
    // if (timeuse/1000 > 500) {
    //     printf("curl_easy_perform: start=%d\n", start);
    //     printf("curl_easy_perform: end=%d\n", end);
    //     printf("curl_easy_perform: timeuse=%d ms\n", timeuse/1000);
    // }

    long response_code;
    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code) != CURLE_OK){
        error.code = CURL_ERROR_CODE;
        error.message = "Get curl response code failed.";
        return error;
    }

    if (!HTTP_IS_OK(response_code)){
        error.message = "http is not OK, check the code as HTTP code.";
    }

    error.code = response_code;
    if(error.code == 404){
        error.message = "File not found.";
    }
    return error;
}

// CURLSH* share_handle = NULL;
void init_share_handle() {
    // 设置dns共享数据
    pthread_rwlock_init(&rwlock, NULL); 
    CURLSH* share_handle = curl_share_init();  // curl 共享句柄：作用是允许curl句柄间共享数据
    curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS); // 设置需要共享的数据，CURLSHOPT_SHARE表示设置dns缓存共享
    curl_share_setopt(share_handle, CURLSHOPT_LOCKFUNC, lock_cb);
    curl_share_setopt(share_handle, CURLSHOPT_UNLOCKFUNC, unlock_cb);  
}

static void lock_cb(CURL *handle, curl_lock_data data, curl_lock_access access,void *userptr){ 
    if (data == CURL_LOCK_DATA_DNS){ 
        if ( access == CURL_LOCK_ACCESS_SHARED ){
            pthread_rwlock_rdlock(&rwlock); 
        }
        else if( access == CURL_LOCK_ACCESS_SINGLE ) {
            pthread_rwlock_wrlock(&rwlock); 
        } 
    }
}

static void unlock_cb(CURL *handle, curl_lock_data data, void *userptr){ 
    if (data == CURL_LOCK_DATA_DNS){
        pthread_rwlock_unlock(&rwlock); 
    }
}


