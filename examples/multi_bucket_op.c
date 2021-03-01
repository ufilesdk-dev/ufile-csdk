#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "thpool.h" //线程池模块，仅做为测试用，不包含在sdk模块中。

#define BUCKET_NUM 10
#define TASK_NUM 5

char buckets[BUCKET_NUM][50] = {
    "csdk-for-dahua",
    "csdk-for-dahua1",
    "csdk-for-dahua2",
    "csdk-for-dahua3",
    "csdk-for-dahua4",
    "csdk-for-dahua5",
    "csdk-for-dahua6",
    "csdk-for-dahua7",
    "csdk-for-dahua8",
    "csdk-for-dahua9"
};

char ul_filepaths[BUCKET_NUM][50] = {
    "/data/pics/1.jpg",
    "/data/pics/2.jpg",
    "/data/pics/3.jpg",
    "/data/pics/4.jpg",
    "/data/pics/5.jpg",
    "/data/pics/6.jpg",
    "/data/pics/7.jpg",
    "/data/pics/8.jpg",
    "/data/pics/9.jpg",
    "/data/pics/0.jpg"
};

char dl_filepaths[BUCKET_NUM][50] = {
    "/data/pics/download/1.jpg",
    "/data/pics/download/2.jpg",
    "/data/pics/download/3.jpg",
    "/data/pics/download/4.jpg",
    "/data/pics/download/5.jpg",
    "/data/pics/download/6.jpg",
    "/data/pics/download/7.jpg",
    "/data/pics/download/8.jpg",
    "/data/pics/download/9.jpg",
    "/data/pics/download/0.jpg"
};

FILE *log_fp = NULL;

int upload_file(char* bucket, char* ul_filepath, char* keyname){
    struct timeval start, end;
    gettimeofday( &start, NULL );
    
    FILE *fp = fopen(ul_filepath, "rb");
    if (fp == NULL){
        fprintf(log_fp,"upload_file:打开文件失败");
        return -1;
    }
    struct ufile_error error;
    error = ufile_put_file(bucket, keyname, "jpg", fp);
    if(UFILE_HAS_ERROR(error.code)){
        fprintf(log_fp,"上传文件失败，错误信息为：%s\n", error.message);
        return -1;
    }
    fclose(fp);

    gettimeofday( &end, NULL );
    int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec; 
    if (timeuse/1000 > 500) {
        printf("upload_file -- time: %d, timeuse gt 500ms: %d ms, ", end.tv_sec, timeuse/1000);
        printf("bucket/keyname: %s/%s \n", bucket, keyname);
        fprintf(log_fp,"upload_file -- time: %d; timeuse gt 500ms: %d ms, bucket/keyname: %s/%s \n", end.tv_sec, timeuse/1000, bucket, keyname);
    }
    return 0;
}

int download_file(char* bucket, char* dl_filepath, char* keyname){
    struct timeval start, end;
    gettimeofday( &start, NULL );
    
    FILE *dl_fp = fopen(dl_filepath, "wb");
    if (dl_fp == NULL){
        fprintf(log_fp,"download_file:打开文件失败");
        return -1;
    }
    struct ufile_error error;
    error = ufile_download(bucket, keyname, dl_fp, NULL);
    if(UFILE_HAS_ERROR(error.code)){
        fprintf(log_fp,"下载文件失败，错误信息为：%s\n", error.message);
        return -1;
    }
    fclose(dl_fp);

    gettimeofday( &end, NULL );
    int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec; 
    if (timeuse/1000 > 500) {
        printf("download_file -- time: %d, timeuse gt 500ms: %d ms, ", end.tv_sec, timeuse/1000);
        printf("bucket/keyname: %s/%s \n", bucket, keyname);
        fprintf(log_fp, "download_file -- time: %d; timeuse gt 500ms: %d ms, bucket/keyname: %s/%s \n", end.tv_sec, timeuse/1000, bucket, keyname);
    }
    return 0;
}

int delete_file(char* bucket, char* keyname){
    struct timeval start, end;
    gettimeofday( &start, NULL );
    
    struct ufile_error error;
    error = ufile_delete(bucket, keyname);
    if(UFILE_HAS_ERROR(error.code)){
        fprintf(log_fp,"删除文件失败，错误信息为：%s\n", error.message);
        return -1;
    }

    gettimeofday( &end, NULL);
    int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec; 
    if (timeuse/1000 > 500) {
        printf("delete_file -- time: %d, timeuse gt 500ms: %d ms, ", end.tv_sec, timeuse/1000);
        printf("bucket/keyname: %s/%s \n", bucket, keyname);
        fprintf(log_fp, "delete_file -- time: %d; timeuse gt 500ms: %d ms, bucket/keyname: %s/%s \n", end.tv_sec, timeuse/1000, bucket, keyname);
    }
    return 0;
}

void thread_task(void *param) {
    int *tmp = (int*)param;
    int index = (*tmp) % BUCKET_NUM;
    int ret = 0;
    char keyname[10];
    sprintf(keyname,"%d",*tmp);
    ret = upload_file(buckets[index], ul_filepaths[index], keyname);
    if (ret != 0) {
        return;
    }
    download_file(buckets[index], dl_filepaths[index], keyname);
    delete_file(buckets[index], keyname);
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
        fprintf(log_fp, "初始化 sdk 失败，错误信息为：%s\n", error.message);
        return 1;
    }
    
    char log_file[100] = "test.log";
    if (argc > 1) {
        strcpy(log_file, argv[1]);
    }
    log_fp = fopen(log_file, "w+");
    fprintf(log_fp,"初始化 sdk 成功\n");
    fflush(log_fp);

    printf("初始化 sdk 成功\n");

    while (1) {
        struct timeval start1, end1;
        gettimeofday( &start1, NULL );
        threadpool thpool = thpool_init(TASK_NUM);
        int i = 0;
        int index[10000];
        for (; i < 10000; i++) {
            index[i]=i;
            thpool_add_work(thpool, &thread_task, &index[i]);
        }
        thpool_wait(thpool);
        thpool_destroy(thpool);
        gettimeofday( &end1, NULL );
        int timeuse1 = 1000000 * ( end1.tv_sec - start1.tv_sec ) + end1.tv_usec - start1.tv_usec; 
        fprintf(log_fp, "finish 30000 request!!! time: %d; time use: %d ms\n", end1.tv_sec, timeuse1/1000);
        fflush(log_fp);
        printf("finish 30000 request!!! time use: %d ms\n", timeuse1/1000);
        
    }
    ufile_sdk_cleanup();
    fclose(log_fp);
    return 0;
}

