#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

#define CONFIG_FILE "./config.json"
const char* bucket_name = "csdk-create-bucket";
const char* key_name = "file_put_file";

int main(int argc, char *argv[]){

    // FILE *fp = fopen(CONFIG_FILE, "r");
    // if (fp == NULL){
    //     fprintf(stderr, "打开配置文件失败, 错误信息为: %s\n", strerror(errno));
    //     return 1;
    // }
    // long fsize = helper_get_file_size(fp);
    // if(fsize == 0){
    //     printf("配置文件为空\n");
    //     return 1;
    // } 
    // char *buf = (char *)malloc(fsize+1);
    // size_t n = fread(buf, 1, fsize, fp);
    // if (n!=fsize){
    //     printf("read %zu, file size %ld\n", n, fsize);
    //     int err_no = ferror(fp);
    //     fprintf(stderr, "Value of errno: %d, message is: %s\n", err_no, strerror(err_no)); 
    //     fclose(fp);
    //     return 1;
    // }
    // fclose(fp);
    // struct ufile_error error;
    // struct ufile_config cfg;
    // buf[fsize] = '\0';
    // printf("正在解析配置文件 .....\n");
    // error = ufile_load_config_from_json(buf, &cfg);
    // if(UFILE_HAS_ERROR(error.code)){
    //     printf("加载配置文件失败，错误信息为：%s\n", error.message);
    //     return 1;
    // }
    // free(buf);
    // printf("正在初始化 SDK ......\n");
    // error = ufile_sdk_initialize(cfg, 0);
    // if(UFILE_HAS_ERROR(error.code)){
    //     printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
    //     return 1;
    // }
    // ufile_free_config(cfg);

    struct ufile_error error;
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = getenv("UFILE_BUCKET_HOST");
    cfg.file_host = getenv("UFILE_FILE_HOST");
    printf("正在初始化 SDK ......\n");
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }

    struct ufile_file_info file_info;
    printf("调用 head file .....\n");
    error = ufile_head(bucket_name, key_name, &file_info);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 head 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 head 成功，文件信息为： size=%lld, etag=%s, mime-type=%s",
                file_info.bytes_len, file_info.etag, file_info.mime_type);
    }
    ufile_free_file_info(file_info);

    ufile_sdk_cleanup();
    return 0;
}