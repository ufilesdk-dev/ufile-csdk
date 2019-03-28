#include "auth.h"
#include "string_util.h"
#include "encoding.h"

#include <string.h>
#include <stdlib.h>

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

char * ufile_download_authorization(
    const char* private_key,
    const char* bucket,
    const char* key,
    const char* method,
    const char* unix_expires,
    const char* md5,
    const char* mime_type
){
    char *sig_data = ufile_strconcat(method,"\n",
                                    md5, "\n",
                                    mime_type, "\n",
                                    unix_expires, "\n",
                                     "/",bucket, "/", key,
                                    NULL);
    unsigned char HMAC_str[HMAC_LEN];
    HMAC_SHA1(HMAC_str, (unsigned char*)private_key, strlen(private_key),
              (unsigned char*)sig_data, strlen(sig_data));

    memset(sig_data, 0, strlen(sig_data));
    base64encode(sig_data, (const char*)HMAC_str, 20);
    return sig_data;
}