#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "helper.h"

void randstring(char *out,  size_t length){
    static char charset[] = "abcdefghijklmnopqrstuvwxyz";
    static size_t s_len = sizeof(charset)-1;
    int i = 0;
    for (i = 0;i < length;i++) {            
        int key = rand() % (int)s_len;
        out[i] = charset[key];
    }
}

int main(int argc, char *argv[]){
    srand((unsigned)time(NULL));
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = "api.ucloud.cn";
    cfg.file_host = "cn-bj.ufileos.com";

    printf("正在初始化 SDK ......\n");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

    char bucket_name[8]={0};
    randstring(bucket_name, 7);
    error = ufile_bucket_create(bucket_name, "cn-bj", "private");
    if(UFILE_HAS_ERROR(error.code)){
        printf("创建 bucket 失败，错误信息为：%s\n", error.message);
    }else{
        printf("创建 bucket 成功\n");
    }

    error = ufile_bucket_delete(bucket_name);
    if (UFILE_HAS_ERROR(error.code)) {
        printf("删除 bucket 失败，错误信息为：%s\n", error.message);
    }else{
        printf("删除 bucket 成功\n");
    }

    ufile_sdk_cleanup();
    return 0;
}