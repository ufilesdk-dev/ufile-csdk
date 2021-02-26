#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

#define CONFIG_FILE "./config.json"
const char* bucket_name = "csdk-create-bucket";
const char* key_name = "file_put_file";

int main(int argc, char *argv[]){
    struct ufile_error error;
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = getenv("UFILE_BUCKET_HOST");
    cfg.file_host = getenv("UFILE_FILE_HOST");
    printf("正在初始化 SDK ......\n");
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

    struct ufile_file_info file_info;
    printf("调用 head file .....\n");
    error = ufile_head(bucket_name, key_name, &file_info);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 head 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 head 成功，文件信息为： size=%lld, etag=%s, mime-type=%s",
                file_info.bytes_len, file_info.etag, file_info.mime_type);
    }
    ufile_free_file_info(file_info);

    ufile_sdk_cleanup();
    return 0;
}