#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

#define CONFIG_FILE "./config.json"

int main(int argc, char *argv[]){
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = "api.ucloud.cn";
    cfg.file_host = "cn-bj.ufileos.com";

    printf("正在初始化 SDK ......\n");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg);
    if(UFILE_HAS_ERROR(error.code)){
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

    printf("调用 put 上传文件.....\n");
    FILE *fp = fopen(argv[0], "r");
    if (fp == NULL){
        fprintf(stderr, "打开文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }
    error = ufile_put_file("lapd", "test", "", fp);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 put 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 put 成功");
    }

    printf("调用 put_buf 上传文件......");
    size_t file_size = helper_get_file_size(fp);
    char *buf = malloc(file_size);
    error = ufile_put_buf("lapd", "hello", "", buf, file_size);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 put_buf 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 put_buf 成功");
    }

    free(buf);
    close(fp);
    ufile_sdk_cleanup();
    return 0;
}