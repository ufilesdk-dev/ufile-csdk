#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

#define fourMegabyte 1 << 22 //4M

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
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

   FILE *fp = fopen(argv[1], "wb");
   printf("调用 download 下载文件.....\n");
   error = ufile_download("echotest2", argv[1], fp, NULL);
   if UFILE_HAS_ERROR(error.code){
       printf("调用 download 失败，错误信息为:%s\n", error.message);
       ufile_sdk_cleanup();
       return 1;
   }
   fclose(fp);
   printf("调用 download 成功。\n");

    char buf[fourMegabyte]; 
    size_t pos = 0;
    size_t return_size;
    while(!UFILE_HAS_ERROR(error.code)){
        error = ufile_download_piece("echotest2", argv[1], pos, buf, fourMegabyte, &return_size);
        if(return_size < fourMegabyte){ //像fread一样，如果的实际读取的字节数小于buf大小，那么就表示已经读到了文件结尾。
            break;
        }
        pos += fourMegabyte;
    }
    if(UFILE_HAS_ERROR(error.code)){
        printf("调用 download buf 失败，错误信息为:%s\n", error.message);
        ufile_sdk_cleanup();
        return 1;
    }
    printf("调用 download buf 成功。\n");
    ufile_sdk_cleanup();
    return 0;
}