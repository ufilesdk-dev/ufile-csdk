#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "thpool.h" //线程池模块，仅做为测试用，不包含在sdk模块中。

struct part_data{
    struct ufile_mutipart_state *state;
    char *buf;
    size_t buf_len;
    int pos;
};

void upload_task(void *param){
    struct part_data *part = (struct part_data*)param;
    struct ufile_error error;
    error = ufile_multiple_upload_part(part->state, part->buf, part->buf_len, part->pos);
    if(UFILE_HAS_ERROR(error.code)){
        printf("调用 upload 失败，错误信息为：%s\n", error.message);
        ufile_multiple_upload_abort(part->state);
        exit(1); //失败了就直接退出程序。
    }
    free(part->buf);
    free(part);
}


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
    error = ufile_sdk_initialize(cfg, 1);
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

    threadpool thpool = thpool_init(4); //初始化 4 线程的线程池。

    FILE *fp = fopen(argv[1], "rb");
    int i;
    for(i=0; ; i++){
        struct part_data *part = malloc(sizeof(struct part_data));
        part->state = &state;
        part->buf = malloc(state.part_size);
        part->buf_len = fread(part->buf, 1, state.part_size, fp);
        part->pos = i;
        if(part->buf_len == 0){
            free(part->buf);
            free(part);
            break;
        }
        thpool_add_work(thpool, &upload_task, part);
    }
    thpool_wait(thpool);
    thpool_destroy(thpool);

    error = ufile_multiple_upload_finish(&state);
    if(UFILE_HAS_ERROR(error.code)){
        printf("调用 ufile_multiple_upload_part 失败，错误信息为：%s\n", error.message);
    }else{
        printf("调用 (mput) 分片上传成功\n");
    }
    ufile_sdk_cleanup();
    return 0;
}