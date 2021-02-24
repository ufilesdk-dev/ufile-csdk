#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define fourMegabyte 1 << 22 //4M

int main(int argc, char *argv[]){
    if (argc < 4) {
       printf("请依次提供bucket_name、key_name、file_path\n"); 
       return 1;
    }
    char* bucket_name = argv[1];
    char* key_name = argv[2];
    char* file_path = argv[3];
    printf("分片上传: bucket_name=%s key_name=%s file_path=%s \n", bucket_name, key_name, file_path);


    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = getenv("UFILE_BUCKET_HOST");
    cfg.file_host = getenv("UFILE_FILE_HOST");

    printf("正在初始化 SDK ......\n");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        ufile_sdk_cleanup();
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

    printf("创建本地文件 %s\n", file_path);
    FILE *fp = fopen(file_path, "a+");
    if (fp == NULL){
        fprintf(stderr, "打开文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }

    printf("调用 ufile_download_piece 分片下载\n");
    char buf[fourMegabyte]; 
    size_t pos = 0;
    size_t return_size;
    while(!UFILE_HAS_ERROR(error.code)){
        error = ufile_download_piece(bucket_name, key_name, pos, buf, fourMegabyte, &return_size);
        size_t nc = fwrite(buf, 1, return_size, fp);
        if(return_size < fourMegabyte){ //像fread一样，如果的实际读取的字节数小于buf大小，那么就表示已经读到了文件结尾。
            printf("return_size < fourMegabyte: %d < %d \n", return_size, fourMegabyte);
            break;
        }
        //fputs(buf, fp);
        pos += fourMegabyte;
        printf("pos = %d, fourMegabyte = %d. 写入 %d 字节\n", pos, fourMegabyte, nc);
        sleep(1); 
    }


    // char buf[fourMegabyte]; 
    // size_t pos = 0;
    // size_t return_size;
    // while(!UFILE_HAS_ERROR(error.code)){
    //     error = ufile_download_piece(bucket_name, key_name, pos, buf, fourMegabyte, &return_size);
    //     if(return_size < fourMegabyte){ //像fread一样，如果的实际读取的字节数小于buf大小，那么就表示已经读到了文件结尾。
    //         printf("return_size < fourMegabyte: %d < %d \n", return_size, fourMegabyte);
    //         break;
    //     }
    //     pos += fourMegabyte;
    //     fputs(buf, fp);
    //     printf("pos = %d, 正在下载第%d片...", pos, (pos/fourMegabyte)+1);
    // }
    fclose(fp);
    
    if(UFILE_HAS_ERROR(error.code)){
        printf("调用 ufile_download_piece 失败，错误信息为:%s\n", error.message);
        ufile_sdk_cleanup();
        return 1;
    }
    printf("调用 ufile_download_piece 成功。\n");
    ufile_sdk_cleanup();
    return 0;
}