#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("请输入一个文件 key!!!");
        exit(1);
    }

    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = "api.ucloud.cn";
    cfg.file_host = "cn-bj.ufileos.com";

    printf("正在初始化 SDK ......\n");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg, 1);
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }
    printf("正在删除文件 %s\n", argv[1]);
    error = ufile_delete("echotest2", argv[1]);
    if(UFILE_HAS_ERROR(error.code)){
        printf("删除文件失败，错误信息为：%s\n", error.message);
    }else{
        printf("删除文件成功\n");
    }
    ufile_sdk_cleanup();
}