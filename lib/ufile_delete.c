#include "api.h"

#include <curl/curl.h>
#include <string.h>
#include "http.h"

extern struct ufile_error
ufile_delete(const char* bucket_name, const char *key){
    struct ufile_error error = NO_ERROR;
    error = check_bucket_key(bucket_name, key);
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }

    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    struct http_options opt;
    error = set_http_options(&opt, "DELETE", "", bucket_name, key, NULL);
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, &opt);
        return error;
    }

    set_curl_options(curl, &opt);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    error = curl_do(curl);
    http_cleanup(curl, &opt);
    return error;
}