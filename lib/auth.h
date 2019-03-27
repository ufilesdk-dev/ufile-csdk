#ifndef __UFILE_AUTH_H_
#define __UFILE_AUTH_H_


//If you thought this sdk code as shit
//you can use this authorization module alone for you own http request.

//文件相关操作的签名，mime_type, date, md5 可以为空。
//如果你加入了 date 请在 http 的 header 里面加入 Date: md5 头
//如果你加入了 md5 请在 http 的 header 里面加入 Content-MD5: md5 头
//调用本接口后需要调用 free 释放返回的字符串指针。
char * ufile_file_authorization(const char *public_key, const char *private_key,
                                const char *method, const char *bucket, 
                                const char *key, const char *mime_type,
                                const char *date, const char *md5);


//文件下载签名，mime_type, date, md5 可以为空。
//如果你加入了 date 请在 http 的 header 里面加入 Date: md5 头
//如果你加入了 md5 请在 http 的 header 里面加入 Content-MD5: md5 头
//调用本接口后需要调用 free 释放返回的字符串指针
char * ufile_download_authorization(
    const char* private_key,
    const char* bucket,
    const char* key,
    const char* method,
    const char* unix_expires,
    const char* md5,
    const char* mime_type
);
#endif