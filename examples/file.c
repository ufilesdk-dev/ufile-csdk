#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "helper.h"
#define fourMegabyte 1 << 22 //4M

const char region[20] = "cn-sh2";

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

    // 上传文件
    char bucket_name[20];
    struct timeval start;
    gettimeofday( &start, NULL );
    sprintf(bucket_name,"%d",start.tv_sec);
    error = ufile_bucket_create(bucket_name, region, "private");
    if(UFILE_HAS_ERROR(error.code)){
        printf("创建 bucket 失败，错误信息为：%s\n", error.message);
    }else{
        printf("创建 bucket 成功\n");
    }

    // 上传文件
    char *contents = "云想衣裳花想容，春风拂槛露华浓 若非群玉山头见 会向瑶台月下逢";
    char *key_name = "清平调";
    printf("调用 ufile_put_buf 上传文件......\n");
    error = ufile_put_buf(bucket_name, key_name, "", contents, strlen(contents));
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 ufile_put_file 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 ufile_put_file 成功\n");
    }

    // 获取文件基本信息
    printf("调用 ufile_head 获取文件基本信息\n");
    struct ufile_file_info file_info;
    error = ufile_head(bucket_name, key_name, &file_info);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 head 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 ufile_head 获取文件基本信息成功，信息为: size=%lld, etag=%s \n", file_info.bytes_len, file_info.etag);
    }
    ufile_free_file_info(file_info);

    // 下载文件
    printf("调用 ufile_download_piece 下载文件\n");
    char buf[fourMegabyte]; 
    size_t pos = 0;
    size_t return_size;
    error = ufile_download_piece(bucket_name, key_name, pos, buf, fourMegabyte, &return_size);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 ufile_download_piece 下载失败，错误信息为：%s\n", error.message);
    }else{
        printf("文件内容为：%s \n", buf);
    }

    // 删除文件
    printf("正在删除文件 %s\n", key_name);
    error = ufile_delete(bucket_name, key_name);
    if(UFILE_HAS_ERROR(error.code)){
        printf("删除文件失败，错误信息为：%s\n", error.message);
    }else{
        printf("删除文件成功\n");
    }

    // 删除bucket
    error = ufile_bucket_delete(bucket_name);
    if (UFILE_HAS_ERROR(error.code)) {
        printf("删除 bucket 失败，错误信息为：%s\n", error.message);
    }else{
        printf("删除 bucket 成功\n");
    }

    ufile_sdk_cleanup();
    return 0;
}
