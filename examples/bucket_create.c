#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

    char* bucket_name;
    char* region;
    char* bucket_type;
    if (argc > 1) {
        bucket_name = argv[1];
    } else {
        bucket_name = (char *)malloc(8);
        randstring(bucket_name, 7);
    }
    if (argc > 2) {
        region = argv[2];
    } else {
        region = (char *)malloc(6);
        region = "cn-bj";
    }
    if (argc > 3) {
        bucket_type = argv[3];
    } else {
        bucket_type = (char *)malloc(8);
        bucket_type = "private";
    }

    printf("正在初始化 SDK ......\n");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }
    printf("创建 Bucket: bucket_name=%s region=%s bucket_type=%s\n", bucket_name, region, bucket_type);
    error = ufile_bucket_create(bucket_name, region, bucket_type);
    if(UFILE_HAS_ERROR(error.code)){
        printf("创建 bucket 失败，错误信息为：%s\n", error.message);
    }else{
        printf("创建 bucket 成功\n");
    }

    ufile_sdk_cleanup();
    return 0;
}