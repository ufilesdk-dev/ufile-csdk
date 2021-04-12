#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


int main(int argc, char *argv[]){
    if (argc < 3) {
       printf("请依次提供bucket_name、key_name\n"); 
       return 1;
    }
    char* bucket_name = argv[1];
    char* key_name = argv[2];
    printf("获取文件基本信息: bucket_name=%s key_name=%s\n", bucket_name, key_name);

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

    printf("调用 ufile_head 获取文件基本信息\n");
    struct ufile_file_info file_info;
    file_info.etag = NULL;
    file_info.mime_type = NULL;
    error = ufile_head(bucket_name, key_name, &file_info);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 head 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 ufile_head 获取文件基本信息成功，信息为: size=%lld,etag=%s,mime-type=%s\n", file_info.bytes_len, file_info.etag, file_info.mime_type);
    }
    ufile_free_file_info(file_info);
    ufile_sdk_cleanup();
    return 0;
}