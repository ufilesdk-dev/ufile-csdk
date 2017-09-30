#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include "api.h"

void help() {

    // printf("./demo put bucket key file\n");
    // printf("./demo putstr bucket key str\n");
    printf("./demo mput bucket key file\n");
    printf("./demo delete bucket key\n");
    printf("./demo download bucket key storedfile [range_begin-range_end]\n");
    // printf("./demo download2 bucket key storedfile [range_begin-range_end]\n");
    // printf("./demo showurl bucket key [expires]\n");
}

//分片上传
int mputfile(int argc, char **argv) {

    char upload_id[64];
    size_t blksize = 0;
    ret_status_t *ret_status = NULL;
    struct stat st;
    int blk_num = 0;
    int i = 0;
    size_t blk_idx = 0;
    etag_t *etags = NULL;
    char final_etag[64];

    if (argc != 3) {
        printf("./demo mput bucket key file\n");
        return -1;
    }

    memset(upload_id, 0, 64);
    memset(final_etag, 0, 64);    
    ret_status = init_multipart_upload(argv[0], argv[1], upload_id, &blksize);
    if (ret_status) {
        printf("init_multipart_upload error retcode: %d\nerror_message: %s\n", ret_status->ret_code, ret_status->error_message);
        return ret_status->ret_code;
    }

    if (stat(argv[2], &st)) {
        printf("%s\n",strerror(errno));
        return -1;
    }

    blk_num = st.st_size / blksize + 1;
    etags = (etag_t *)malloc(blk_num * sizeof(etag_t));
    for (; blk_idx < blk_num; blk_idx++) {

        for(i=0; i<20; ++i) printf("\b");
        printf("finished: %%%d\n", (int)blk_idx*100/blk_num);

        ret_status = upload_multipart(argv[0], argv[1], upload_id, argv[2], etags + blk_idx, blksize, blk_idx);
        if (ret_status) {
            printf("upload_multipart error retcode: %d\nerror_message: %s\n", ret_status->ret_code, ret_status->error_message);
            if (etags != NULL) {
                free(etags);
                etags = NULL;
            }
            return ret_status->ret_code;
        }
        printf("ETag: %zu  %s\n", etags[blk_idx].blk_idx, etags[blk_idx].etag);
    }
    for(i=0; i<20; ++i) printf("\b");
    printf("finished: %%%d\n", (int)blk_idx*100/blk_num);

    ret_status = upload_multipart_finish(argv[0], argv[1], upload_id, etags, blk_num, (char*)final_etag);
    if (ret_status) {
            printf("upload_multipart_finish error retcode: %d\nerror_message: %s\n", ret_status->ret_code, ret_status->error_message);
            if (etags != NULL) {
                free(etags);
                etags = NULL;
            }
            return ret_status->ret_code;
    }
    printf("Final ETag: %s\n",final_etag);
    printf("mput file success\n");
    if (etags != NULL) {
        free(etags);
        etags = NULL;
    }
    return 0;
}


int deletefile(int argc, char **argv) {

     ret_status_t *ret_status = NULL;
    if (argc != 2) {
        printf("./demo delete bucket key\n");
        return -1;
    }

    ret_status = delete(argv[0], argv[1]);
    if (ret_status) {
            printf("init_multipart_upload error retcode: %d\nerror_message: %s\n", ret_status->ret_code, ret_status->error_message);
            return ret_status->ret_code;
    }
    printf( "delete file success\n");
    return 0;
}

//下载到文件
int download_as_file(int argc, char **argv) {

    ret_status_t *ret_status = NULL;
    int ret = 0;
    range_t range;
    ssize_t begin = -1, end = -1;

    if (argc != 3 && argc !=4) {
        printf("./demo download bucket key storedfile [range_bagin-range_end]\n");
        return -1;
    }

    
    if (argc == 4) {
        sscanf(argv[3], "%zd-%zd", &begin, &end);
        range.begin = begin;
        range.end = end;
        ret_status = download(argv[0], argv[1], argv[2], &range);
    } else {
        ret_status = download(argv[0], argv[1], argv[2], NULL);
    }

    if (ret_status) {
            printf("download error retcode: %d\nerror_message: %s\n", ret_status->ret_code, ret_status->error_message);
            return ret_status->ret_code;
    }
    printf("download file success\n");
    return 0;
}

int dispatch(int argc, char **argv) {

    int ret = 0;
    char *cmd = argv[0];
    if (memcmp(cmd, "mput", strlen(cmd)) == 0) {
        ret = mputfile(argc-1, argv+1);
    } else if (memcmp(cmd, "delete", strlen(cmd)) == 0) {
        ret = deletefile(argc-1, argv+1);
    } else if (memcmp(cmd, "download", strlen(cmd)) == 0) {
        ret = download_as_file(argc-1, argv+1);
    }
    return ret;
}

int main(int argc, char **argv) {

    ret_status_t *ret_status = NULL;
    if (argc < 2 || memcmp(argv[1], "-h", 2) == 0 || memcmp(argv[1], "--help", 6) == 0) {
        help();
        exit(0);
    }

    ret_status = csdk_initialize();
    if (ret_status) {
            printf("csdk_initialize error retcode: %d\nerror_message: %s\n", ret_status->ret_code, ret_status->error_message);
            return ret_status->ret_code;
    }
    int ret = dispatch(argc-1, argv+1);
    csdk_cleanup();
    exit(ret);
}

