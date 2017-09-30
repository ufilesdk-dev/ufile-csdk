#include "digest.h"
#include "error.h"
#include "hmac_sha1.h"
#include "base64.h"

#include <malloc.h>

int  sign_with_data(const char *utf8_string2_sign, char *signature) {

    unsigned char digest[21];
    if (utf8_string2_sign == NULL){
        return -1;
    }

    HMAC_SHA1(digest,
              (unsigned char *)ucloud_private_key,
              strlen(ucloud_private_key),
              (unsigned char *)utf8_string2_sign,
              strlen(utf8_string2_sign));
    return base64_encode(digest, 20, BASE64_STD, signature);
}

int token(struct curl_slist *req_headers,
          const char *method,
          const int type,
          const char *bucket,
          const char *key,
          const char *multipartMimetype,
          char *token){

    int ret = 0;
    const char *mimetype = NULL;
    char signature[1024];
    char utf8_string2_sign[1024];
    char ucloud_headers[1024];
    char *header_md5 = NULL;
    char *header_data = NULL;
    memset(signature, 0, 1024);
    memset(utf8_string2_sign, 0, 1024);
    memset(ucloud_headers, 0, 1024);
    mimetype = http_request_header("Content-Type", req_headers);
    if (http_request_header("multipart/form-data", req_headers) != NULL) {
        mimetype = multipartMimetype;
    }

    canonicalized_ucloud_headers(req_headers, ucloud_headers);

    header_md5 = http_request_header("Content-MD5", req_headers);
    header_data = http_request_header("Data", req_headers);

    snprintf(utf8_string2_sign, 1024, "%s\n%s\n%s\n%s\n%s/%s/%s",
             method,
             header_md5 ? header_md5 : "",
             mimetype ? mimetype : "",
             header_data ? header_data : "",
             ucloud_headers ? ucloud_headers : "",
             bucket,
             key);
    ret = sign_with_data(utf8_string2_sign, signature);
    if (ret) return ret;
    
    sprintf(token, "UCloud %s:%s", ucloud_public_key, signature);
    return ret;
}

void canonicalized_ucloud_headers(struct curl_slist *req_headers, char *ucloud_headers) {

    struct curl_slist *header = req_headers;
    char *data = (char*)malloc(64);
    for(; header ; header = header->next) {
        memset(data, 0, 64);
        strncpy(data, header->data, 64);
        if (memcmp(data, "x-ucloud-", 9) != 0) continue;
        sdk_string_tolower(data, ':');
        strcat(ucloud_headers, data);
        strcat(ucloud_headers, "\n");
    }
    if (data != NULL) {
        free(data);
    }
}