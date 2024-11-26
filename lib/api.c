#include "api.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "string_util.h"

struct ufile_config *_global_config = NULL;
int _g_debug_open = 0;

static struct ufile_error
config_validation(struct ufile_config cfg){
    struct ufile_error error=NO_ERROR;
    if (!cfg.private_key || strlen(cfg.private_key) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "private_key cannot be empty";
        return error;
    }

    if (!cfg.public_key || strlen(cfg.public_key) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "public_key cannot be empty";
        return error;
    }

    if (!cfg.file_host || strlen(cfg.file_host) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "file_host cannot be empty";
        return error;
    }

    if (!cfg.bucket_host || strlen(cfg.bucket_host) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "bucket host cannot be empty";
        return error;
    }
    return error;
}

struct ufile_error
ufile_sdk_initialize(const struct ufile_config cfg, int debug_open){
    _g_debug_open = debug_open; 
    struct ufile_error error = NO_ERROR;
    if(_global_config != NULL){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "the sdk has been initialization.";
        return error;
    }
    error=config_validation(cfg);
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }
    _global_config = (struct ufile_config*)malloc(sizeof(struct ufile_config));
    _global_config->public_key = ufile_strconcat(cfg.public_key, NULL);
    _global_config->private_key = ufile_strconcat(cfg.private_key, NULL);
    _global_config->file_host = ufile_strconcat(cfg.file_host, NULL);
    _global_config->bucket_host = ufile_strconcat(cfg.bucket_host, NULL);

    CURLcode ret_code = curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK != ret_code){
        error.code = CURL_ERROR_CODE;
        error.message = curl_easy_strerror(ret_code);
    }
    return error;
}

void
ufile_sdk_cleanup(){
    if (!_global_config) {
      return;
    }
    ufile_free_config(*_global_config);
    free((void*)_global_config);
    _global_config = NULL;
    curl_global_cleanup();
}

void 
ufile_free_file_info(struct ufile_file_info info){
    if(info.etag != NULL){
        free((void*)info.etag);
    }
    if(info.etag != NULL){
        free((void*)info.mime_type);
    }
}

struct ufile_error
ufile_load_config_from_json(const char* json_buf, struct ufile_config *cfg){
    struct ufile_error error = NO_ERROR;
    struct cJSON *json = cJSON_Parse(json_buf); 
    if(json==NULL){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = cJSON_GetErrorPtr();
        return error;
    }
    struct cJSON *item = json->child;
    while(item!=NULL){
        if (strcmp(item->string, "public_key") == 0){
            cfg->public_key = ufile_strconcat(item->valuestring,NULL);
        }else if(strcmp(item->string,"private_key") == 0){
            cfg->private_key = ufile_strconcat(item->valuestring, NULL);
        }else if(strcmp(item->string,"file_host") == 0){
            cfg->file_host = ufile_strconcat(item->valuestring, NULL);
        }else if(strcmp(item->string,"bucket_host") == 0){
            cfg->bucket_host = ufile_strconcat(item->valuestring, NULL);
        }
        item = item->next;
    }
    cJSON_Delete(json);
    error = config_validation(*cfg);
    return error;
}

void 
ufile_free_config(struct ufile_config cfg){
    free((void*)cfg.public_key);
    free((void*)cfg.private_key);
    free((void*)cfg.bucket_host);
    free((void*)cfg.file_host);
}
