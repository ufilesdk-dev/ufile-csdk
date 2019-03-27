#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "helper.h"

#define CONFIG_FILE "./config.json"

int main(int argc, char *argv[]){

    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL){
        fprintf(stderr, "打开配置文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }
    long fsize = helper_get_file_size(fp);
    if(fsize == 0){
        printf("配置文件为空\n");
        return 1;
    } 
    char *buf = (char *)malloc(fsize+1);
    size_t n = fread(buf, 1, fsize, fp);
    if (n!=fsize){
        printf("read %zu, file size %ld\n", n, fsize);
        int err_no = ferror(fp);
        fprintf(stderr, "Value of errno: %d, message is: %s\n", err_no, strerror(err_no)); 
        fclose(fp);
        return 1;
    }
    fclose(fp);
    
    struct ufile_error error;
    struct ufile_config cfg;
    buf[fsize] = '\0';
    printf("正在解析配置文件 .....\n");
    error = ufile_load_config_from_json(buf, &cfg);
    if(UFILE_HAS_ERROR(error.code)){
        printf("加载配置文件失败，错误信息为：%s\n", error.message);
        return 1;
    }
    free(buf);

    printf("正在初始化 SDK ......\n");
    error = ufile_sdk_initialize(cfg, 0);
    if(UFILE_HAS_ERROR(error.code)){
        printf("初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }
    ufile_free_config(cfg);

    struct ufile_file_info file_info;
    printf("调用 head file .....\n");
    error = ufile_head("iop-build","centos6_ufile.tar", &file_info);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 head 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 head 成功，文件信息为： size=%lld, etag=%s, mime-type=%s",
                file_info.bytes_len, file_info.etag, file_info.mime_type);
    }
    ufile_free_file_info(file_info);

    printf("调用 put 上传文件.....\n");

    fp = fopen(argv[0], "r");
    if (fp == NULL){
        fprintf(stderr, "打开文件失败, 错误信息为: %s\n", strerror(errno));
        return 1;
    }
    error = ufile_put_file("iop-build", "test", "", fp);
    if UFILE_HAS_ERROR(error.code) {
        printf("调用 PUT 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 PUT 成功");
    }
    fclose(fp);

    ufile_sdk_cleanup();
    return 0;
}