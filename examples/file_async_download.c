#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "thpool.h" //线程池模块，仅做为测试用，不包含在sdk模块中。

void dl_file(void *param){
    struct timeval start, end;
    gettimeofday( &start, NULL );
    int *index = (int*)param;
    char filepath[10];
    sprintf(filepath,"%d",*index);
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL){
        printf("打开文件失败");
        return;
    }
    struct ufile_error error;
    error = ufile_download("csdk-for-dahua", filepath, fp, NULL);
    fclose(fp);
    gettimeofday( &end, NULL );
    int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec; 
    if (timeuse/1000 > 500) {
        printf("time: %d, timeuse gt 500ms: %d ms\n", end.tv_sec, timeuse/1000);
        // printf("timeuse=%d ms \n", timeuse/1000);
        // printf("start=%d\n", start);
        // printf("end=%d\n", end);
        // printf("index=%d\n", *index);
    }
    return;
}

int main(int argc, char *argv[]){
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
    int index[1000000];
    threadpool thpool = thpool_init(30);
    for (int i=0; i < 1000000; i++) {
        index[i]=i;
        thpool_add_work(thpool, &dl_file, &index[i]);
    }
    printf("finish thpool_add_work!\n");
    thpool_wait(thpool);
    thpool_destroy(thpool);

    
    ufile_sdk_cleanup();
    return 0;
}