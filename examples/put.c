#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

const char* bucket_name = "csdk-create-bucket";
const char* key_name = "config.json";
const char* ul_file_path = "config.json";
const char* mime_type = "plain/json";

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

    printf("打开文件 %s\n", argv[0]);
    FILE *fp = fopen(ul_file_path, "rb");
    if (fp == NULL){
        fprintf(stderr, "打开文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }
    printf("调用 put 上传文件 %s\n", argv[0]);
    error = ufile_put_file(bucket_name, key_name, mime_type, fp);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 put 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 put 成功\n");
    }

    printf("调用 put_buf 上传文件......\n");
    size_t file_size = helper_get_file_size(fp);
    char *buf = malloc(file_size);
    fseek(fp, 0, SEEK_SET);
    fread(buf, 1, file_size, fp);
    error = ufile_put_buf(bucket_name, key_name, mime_type, buf, file_size);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 put_buf 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 put_buf 成功\n");
    }

    free(buf);
    fclose(fp);
    ufile_sdk_cleanup();
    return 0;
}
