#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void randstring(char *out,  size_t length){
    static char charset[] = "abcdefghijklmnopqrstuvwxyz";
    static size_t s_len = sizeof(charset)-1;
    int i = 0;
    for (i = 0;i < length - 1;i++) {            
        int key = rand() % (int)s_len;
        out[i] = charset[key];
    }
}

int main(int argc, char *argv[]){
    srand((unsigned)time(NULL));
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = getenv("UFILE_BUCKET_HOST");
    cfg.file_host = getenv("UFILE_FILE_HOST");

    char* bucket_name;
    char* region = "cn-bj";
    char* bucket_type = "private";
    if (argc > 1) {
        bucket_name = argv[1];
    } else {
        char tmp_bucket[10];
        randstring(tmp_bucket, 9);
        tmp_bucket[9] = '\0';
        bucket_name = tmp_bucket;
    }
    if (argc > 2) {
        region = argv[2];
    }
    if (argc > 3) {
        bucket_type = argv[3];
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