#ifndef _H_UFILESDK_C_UCLOUD_API_
#define _H_UFILESDK_C_UCLOUD_API_

#include "http.h"
#include "common.h"
#include <stdio.h>
#include <sys/types.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

/*
* @brief: csdk初始化资源，只允许调用一次
* @return: ret_status_t, 非空表示失败
*/
ret_status_t *csdk_initialize();

/*
* @brief: csdk释放资源，对应csdk_initialize接口
* @return: ret_status_t, 非空表示失败
*/
ret_status_t *csdk_cleanup();

/*
* @brief: 分片上传初始化
* @bucket: 目标Bucket名称
* @key: 保存在Bucket上的文件对象名称
* @upload_id: 返回upload id
* @blksize: 分片大小
* @return: ret_status_t, code等于200成功，否则失败
*/
ret_status_t *init_multipart_upload(const char *bucket, 
                                    const char *key, 
                                    char *upload_id,
                                    size_t *blksize);

/*
* @brief: 上传文件分片
* @bucket: 目标Bucket名称
* @key: 保存在Bucket上的文件对象名称
* @upload_id: upload id
* @file_path: 文件路径
* @etag: 分片编号与ETag的映射
* @blksize: 分片大小
* @blk_idx: 分片编号
* @return: ret_status_t, code等于200成功，否则失败
*/
ret_status_t *upload_multipart(const char *bucket, 
                               const char *key, 
                               const char *upload_id,
                               const char *file_path, 
                               etag_t *etag,
                               size_t blksize,
                               ssize_t blk_idx);

/*
* @brief: 分片上传完成
* @bucket: 目标Bucket名称
* @key: 保存在Bucket上的文件对象名称
* @upload_id: upload id
* @etag: 分片编号与ETag的映射表
* @etags_length: 分片编号与ETag的映射表长度
* @final_etag 上传完成得到的文件ETag
* @return: ret_status_t, code等于200成功，否则失败
*/
ret_status_t *upload_multipart_finish(const char *bucket,
                                      const char *key,
                                      const char *upload_id,
                                      const etag_t *etags,
                                      int etags_length,
                                      char *final_etag);


/*
* @brief: 删除文件
* @bucket: 目标Bucket名称
* @key: 要删除的文件对象名
* @return: ret_status_t, code等于204成功，否则失败
*/
ret_status_t *delete(const char *bucket, 
                     const char *key);

/*
* @brief: 下载文件到本地指定路径
* @bucket: 目标bucket名称
* @key: 要下载的文件对象名
* @filepath: 文件本地路径
* @range: 分片下载的区间，区间为前闭后闭, [begin, end]
* @return: ret_status_t, code等于200成功，否则失败
*/
ret_status_t *download(const char *bucket, 
                               const char *key, 
                               const char *file_path, 
                               const range_t *range);


#ifdef __cplusplus
}
#endif
#endif
