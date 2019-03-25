#ifndef _H_UFILESDK_C_UCLOUD_API_
#define _H_UFILESDK_C_UCLOUD_API_

#include "types.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ufile_file_range{
    long long begin;
    long long end;
};

//********************************************** file info
struct ufile_file_info{
    long long bytes_len;
    const char *etag;
    const char *mime_type;
};
void 
ufile_free_file_info(struct ufile_file_info info);
//***************************************************


//*********************************config
//从 json string 里面解析出一个配置文件，使用本接口你必须要使用 ufile_free_config 释放内存。
struct ufile_error
ufile_load_config_from_json(const char* json_buf, struct ufile_config *cfg);
void 
ufile_free_config(struct ufile_config cfg);
//*********************************end config

//**********************************read and write call back
typedef size_t (*ufile_reader_t)(void *ptr, size_t size, size_t nmemb, void *stream);
//**********************************read and write call back

struct ufile_error
ufile_sdk_initialize(const struct ufile_config cfg);

void
ufile_sdk_cleanup();

struct ufile_error
ufile_head(const char* bucket, const char *key, struct ufile_file_info *info);

struct ufile_error
ufile_put_buf(const char* bucket, const char *key, const char *mime_type, char *buffer, size_t buf_len);

struct ufile_error
ufile_put_file(const char* bucket, const char *key, const char *mime_type, FILE *file);
#ifdef __cplusplus
}
#endif

#endif //_H_UFILESDK_C_UCLOUD_API_
