#ifndef _H_UFILESDK_C_UCLOUD_API_
#define _H_UFILESDK_C_UCLOUD_API_

#include "types.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

//*********************************config
//从 json string 里面解析出一个配置文件，使用本接口你必须要使用 ufile_free_config 释放内存。 
struct ufile_error
ufile_load_config_from_json(const char* json_buf, struct ufile_config *cfg);
void 
//释放从 ufile_load_config_from_json 分片的内存，与 ufile_load_config_from_json 接口成对使用。
ufile_free_config(struct ufile_config cfg);
//*********************************end config

//全局 sdk 初始化接口
//open_verbose 表示打印 curl 输出的相关 http 请求信息到 stdout。1 表示打印，0 表示不打印。
//每当不再用到本 sdk 相关接口后，必须要调用 ufile_sdk_cleanup 接口进行清理。
struct ufile_error
ufile_sdk_initialize(const struct ufile_config cfg, int open_verbose);

//全局 sdk 清理接口，和 ufile_sdk_initialize 成对使用。 
void
ufile_sdk_cleanup();

//********************************************** file info
//远程文件信息数据结构。
struct ufile_file_info{
    long long bytes_len;
    const char *etag;
    const char *mime_type;
};

//获取一个文件的信息，使用此接口必须要调用 ufile_free_file_info 释放堆上的内存。
struct ufile_error
ufile_head(const char* bucket, const char *key, struct ufile_file_info *info);

//清理从 ufile_head 中分配的内存，与 ufile_head 接口成对使用。
void 
ufile_free_file_info(struct ufile_file_info info);
//***************************************************

//把 buf 的数据使用简单上传(put)的方式上传到ufile，上传成功后远端ufile会保存成一个文件。
//mime_type可以为空。
struct ufile_error
ufile_put_buf(const char* bucket, const char *key, const char *mime_type, char *buffer, size_t buf_len);

//使用简单上传(put)上传一个文件到远端 ufile 中。
//mime_type可以为空。
struct ufile_error
ufile_put_file(const char* bucket, const char *key, const char *mime_type, FILE *file);

//分片上传相关状态数据
struct ufile_mutipart_state{
    //远端的 bucket
    const char *bucket;
    //远端的 key
    const char *key;
    //分片大小，由 ufile_multiple_upload_init 返回。
    size_t part_size;
    //分片上传状态 ID，由 ufile_multiple_upload_init 返回。
    const char *upload_id;
    //分片的 etag 列表。内部数据结构，不可访问。
    struct etag_slist *etags;
};

//初始化一个分片上传任务。 mime_type 可以为空。
//初始化成功后会返回一个任务 state。
struct ufile_error
ufile_multiple_upload_init(struct ufile_mutipart_state *self, const char *bucket, const char *key, const char* mime_type);

//上传一个分片， buf_len 的大小始终是 state.part_size, 除了最后一片可以为其他值以外。
//part_number 是分片在文件中的位置，从 0 开始。
struct ufile_error
ufile_multiple_upload_part(struct ufile_mutipart_state *self, char *buffer, size_t buf_len, int part_number);
//完成分片上传
struct ufile_error
ufile_multiple_upload_finish(struct ufile_mutipart_state *self);

//如果调用 ufile_multiple_upload_part 出现错误，可调用此接口取消分片上传任务。
struct ufile_error
ufile_multiple_upload_abort(struct ufile_mutipart_state *self);

//下载远端文件到本地文件， return_size 返回实际下载的大小，以 byte 为单位。
struct ufile_error
ufile_download(const char *bucket, const char *key, FILE *file, size_t *return_size);

//分片下载远端文件中的一片到 buf 中， return_size 返回实际下载的大小，以 byte 为单位。
//return_size 通常会和 buf_len 是一样的大小，除非下载最后一片。
//您可以据此判断一个文件是否已经下载到了最后一个分片。
struct ufile_error
ufile_download_piece(const char *bucket, const char *key, size_t start_position, char *buf, size_t buf_len, size_t *return_size);

#ifdef __cplusplus
}
#endif

#endif //_H_UFILESDK_C_UCLOUD_API_
