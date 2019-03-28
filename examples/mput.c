#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("请输入一个文件路径！！！！");
        exit(1);
    }
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

    printf("调用 (mput)分片 上传文件.....\n");
    struct ufile_mutipart_state state;
    error = ufile_multiple_upload_init(&state, "echotest2", "multiple_upload", "");
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("调用 ufile_multiple_upload_init 失败，错误信息为：%d, %s\n", error.code, error.message);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
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
            ufile_multiple_upload_abort(&state);
            ufile_sdk_cleanup();
            return 1;
        }
    }

    error = ufile_multiple_upload_finish(&state);
    if(UFILE_HAS_ERROR(error.code)){
        printf("调用 ufile_multiple_upload_part 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 (mput)分片上传成功\n");
    }
    ufile_sdk_cleanup();
    return 0;
}