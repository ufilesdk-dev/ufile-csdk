#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

const char* bucket_name = "csdk-create-bucket";
const char* key_name = "mput";
const char* ul_file_path = "/data/pics/valgrind-3.13.0.tar";
const char* mime_type = "";

int main(int argc, char *argv[]){
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = getenv("UFILE_BUCKET_HOST");
    cfg.file_host = getenv("UFILE_FILE_HOST");

    printf("正在初始化 SDK ......\n");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg, 1);
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

    printf("调用 ufile_multiple_upload_init 初始化分片\n");
    struct ufile_mutipart_state state;
    error = ufile_multiple_upload_init(&state, bucket_name, key_name, mime_type);
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("调用 ufile_multiple_upload_init 失败，错误信息为：%d, %s\n", error.code, error.message);
        return 1;
    }
    printf("调用 ufile_multiple_upload_init 初始化分片成功\n");

    printf("打开文件 Makefile \n");
    FILE *fp = fopen(ul_file_path, "rb");
    if (fp == NULL){
        fprintf(stderr, "打开文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }
    printf("调用 ufile_multiple_upload_part 上传分片\n");
    char *buf = malloc(state.part_size);
    int i;
    for(i=0; ; i++){
        size_t nc = fread(buf, 1, state.part_size, fp);
        if(nc == 0){
            break;
        }
        error = ufile_multiple_upload_part(&state, buf, nc, i);
        if(UFILE_HAS_ERROR(error.code)){
            printf("调用 ufile_multiple_upload_part 失败，错误信息为：%d, %s\n", error.code, error.message);
            free(buf);
            ufile_multiple_upload_abort(&state);
            ufile_sdk_cleanup();
            return 1;
        }
    }
    free(buf);
    printf("调用 ufile_multiple_upload_part 上传分片完成\n");

    printf("调用 ufile_multiple_upload_finish 完成分片上传\n");
    error = ufile_multiple_upload_finish(&state);
    if(UFILE_HAS_ERROR(error.code)){
        printf("调用 ufile_multiple_upload_part 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 ufile_multiple_upload_finish 成功\n");
    
    }
    ufile_sdk_cleanup();
    return 0;
}



