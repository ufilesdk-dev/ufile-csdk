#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

const char* bucket_name = "csdk-create-bucket";
const char* key_name = "test.txt";
const char* ul_file_path = "ul-test.txt";
const char* dl_file_path = "dl-test.txt";
const char* mime_type = "plain/text";

int main(int argc, char *argv[]){
    // 读取配置初始化SDK
    struct ufile_config cfg;
    cfg.public_key = getenv("UFILE_PUBLIC_KEY");
    cfg.private_key = getenv("UFILE_PRIVATE_KEY");
    cfg.bucket_host = getenv("UFILE_BUCKET_HOST");
    cfg.file_host = getenv("UFILE_FILE_HOST");
    struct ufile_error error;
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }
    printf("初始化 sdk 成功\n");

    // 调用 ufile_put_file 上传文件
    printf("调用 ufile_put_file 上传文件 %s\n", ul_file_path);
    FILE *fp = fopen(ul_file_path, "rb");
    if (fp == NULL){
        fprintf(stderr, "打开文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }
    error = ufile_put_file(bucket_name, key_name, mime_type, fp);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 ufile_put_file 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 ufile_put_file 成功\n");
    }
    fclose(fp);

    // 获取文件基本信息
    printf("调用 ufile_head 获取文件基本信息\n");
    struct ufile_file_info file_info;
    error = ufile_head(bucket_name, key_name, &file_info);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 head 失败，错误信息为：%s\n", error.message);
        ufile_sdk_cleanup();
        return 1;
    }else{
        printf("调用 ufile_head 获取文件基本信息成功，信息为: size=%lld,etag=%s,mime-type=%s\n", file_info.bytes_len, file_info.etag, file_info.mime_type);
    }
    ufile_free_file_info(file_info);

    // 下载文件
    printf("创建本地文件 %s\n", dl_file_path);
    FILE *f = fopen(dl_file_path, "wb");
    if (f == NULL){
        fprintf(stderr, "打开文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }
    printf("调用 ufile_download 下载文件\n");
    error = ufile_download(bucket_name, key_name, f, NULL);
    if UFILE_HAS_ERROR(error.code){
        printf("调用 download 失败，错误信息为:%s\n", error.message);
        ufile_sdk_cleanup();
        return 1;
    }
    fclose(f);
    printf("调用 ufile_download 下载文件成功。\n");

    // 删除文件
    printf("正在删除文件 %s\n", key_name);
    error = ufile_delete(bucket_name, key_name);
    if(UFILE_HAS_ERROR(error.code)){
        printf("删除文件失败，错误信息为：%s\n", error.message);
    }else{
        printf("删除文件成功\n");
    }

    ufile_sdk_cleanup();
    return 0;
}