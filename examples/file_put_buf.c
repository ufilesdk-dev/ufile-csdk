#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

int main(int argc, char *argv[]){
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = getenv("UFILE_BUCKET_HOST");
    cfg.file_host = getenv("UFILE_FILE_HOST");

    printf("正在初始化 SDK ......\n");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

    if (argc < 4) {
       printf("请依次提供bucket_name、key_name、contents、mime_type(mime_type可为空)\n"); 
       return 1;
    }
    char* bucket_name = argv[1];
    char* key_name = argv[2];
    char* contents = argv[3];
    char* mime_type = "";
    if (argc > 4) {
       mime_type = argv[4];
    }
    int file_size = strlen(contents);

    printf("调用 ufile_put_buf 上传文件......\n");
    error = ufile_put_buf(bucket_name, key_name, mime_type, contents, file_size);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 ufile_put_buf 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 ufile_put_buf 成功\n");
    }

    ufile_sdk_cleanup();
    return 0;
}