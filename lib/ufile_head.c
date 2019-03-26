#include "api.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "string_util.h"

#define CONTENT_TYPE "Content-Type: "
#define CONTENT_LENGTH "Content-Length: "
#define ETAG "ETag: "
static size_t head_header_cb(char *buffer, size_t size, size_t nitems, void *userdata){
    buffer[nitems] = '\0';
    char *p = NULL;
    struct ufile_file_info *info = (struct ufile_file_info*)userdata;
    if((p = strstr(buffer, CONTENT_TYPE)) != NULL){
        info->mime_type = ufile_strconcat(p+strlen(CONTENT_TYPE), NULL);
    }else if((p = strstr(buffer, CONTENT_LENGTH)) != NULL){
        info->bytes_len = atoll(p+strlen(CONTENT_LENGTH));
    }else if((p = strstr(buffer, ETAG)) != NULL){
        info->etag = ufile_strconcat(p+strlen(ETAG), NULL);
    }
    
    return nitems * size;
}

struct ufile_error
ufile_head(const char* bucket, const char *key, struct ufile_file_info *info){
    struct ufile_error error = NO_ERROR;
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    struct http_options *opt;
    error = set_http_options(&opt, "HEAD", "", bucket, key, NULL);
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, opt);
        return error;
    }

    set_curl_options(curl, opt);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, head_header_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)info);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    error = curl_do(curl);
    http_cleanup(curl, opt);
    return error;
}