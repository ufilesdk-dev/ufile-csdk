#include "auth.h"
#include "string_util.h"
#include "encoding.h"

#include <string.h>

const int HMAC_LEN = 21;
//文件相关操作的签名，mime_type, date, md5 可以为空。
//如果你加入了 date 请在 http 的 header 里面加入 Date: md5 头
//如果你加入了 md5 请在 http 的 header 里面加入 Content-MD5: md5 头
//调用本接口后需要调用 free 释放返回的字符串指针。
char * ufile_file_authorization(const char *public_key, const char *private_key,
                                const char *method, const char *bucket, 
                                const char *key, const char *mime_type,
                                const char *date, const char *md5)
{
    char *sig_data = ufile_strconcat(method,"\n",
                                     md5, "\n",
                                     mime_type, "\n",
                                     date, "\n",
                                     "/",bucket, "/", key,
                                     NULL);
    
    unsigned char HMAC_str[HMAC_LEN];
    HMAC_SHA1(HMAC_str, (unsigned char*)private_key, strlen(private_key),
              (unsigned char*)sig_data, strlen(sig_data));

    char signature[20];
    base64encode(signature, (const char*)HMAC_str, 20);

    free(sig_data);
    return ufile_strconcat("UCloud ", public_key, ":", signature, NULL);
}