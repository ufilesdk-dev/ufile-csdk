#ifndef _UFILESDK_C_UCLOUD_ERRNO_
#define _UFILESDK_C_UCLOUD_ERRNO_

#include <stdio.h>
#include <string.h>

enum {
    ERR_CSDK_BASE = -1,
    ERR_CSDK_MISS_AKSK = ERR_CSDK_BASE - 2,
    ERR_CSDK_FILE_READ = ERR_CSDK_BASE - 3,
    ERR_CSDK_SEEKABLE  = ERR_CSDK_BASE - 4,
    ERR_CSDK_CONNECT_HOST = ERR_CSDK_BASE - 5,
    ERR_CSDK_INVALID_HOST = ERR_CSDK_BASE - 6,
    ERR_CSDK_RESOLVE_DOMAIN = ERR_CSDK_BASE - 7,
    ERR_CSDK_SOCKET = ERR_CSDK_BASE - 8,
    ERR_CSDK_CONNECT = ERR_CSDK_BASE - 9,
    ERR_CSDK_SEND_HTTP = ERR_CSDK_BASE - 10,
    ERR_CSDK_SET_CURLOPT = ERR_CSDK_BASE - 11,
    ERR_CSDK_CURL_PERFORM = ERR_CSDK_BASE - 12,
    ERR_CSDK_CURL = ERR_CSDK_BASE - 13,
    ERR_CSDK_CLIENT_INTERNAL = ERR_CSDK_BASE - 14,
    ERR_CSDK_PARSE_JSON = ERR_CSDK_BASE - 15,
    ERR_CSDK_INVALID_PARAM = ERR_CSDK_BASE - 16,
    ERR_CSDK_INVALID_ETAG = ERR_CSDK_BASE - 17,
    ERR_CSDK_NO_CONFIG = ERR_CSDK_BASE - 18,
    ERR_CSDK_INVALID_CONFIG = ERR_CSDK_BASE - 19,
};

typedef struct {
    int ret_code;
    char error_message[128];
}ret_status_t;

static ret_status_t ret_status[] = {
    {ERR_CSDK_MISS_AKSK, "miss publickey/privatekey config"},
    {ERR_CSDK_FILE_READ, "file read error"},
    {ERR_CSDK_SEEKABLE, "stream is not seekable"},
    {ERR_CSDK_CONNECT_HOST, "connect to host failed"},
    {ERR_CSDK_INVALID_HOST, "invalid host"},
    {ERR_CSDK_RESOLVE_DOMAIN, "resolve domain failed"},
    {ERR_CSDK_SOCKET, "socket error"},
    {ERR_CSDK_CONNECT, "socket connect failed"},
    {ERR_CSDK_SEND_HTTP, "send http failed"},
    {ERR_CSDK_SET_CURLOPT, "set curl opt failed"},
    {ERR_CSDK_CURL_PERFORM, "curl preform failed"},
    {ERR_CSDK_CURL, "curl error"},
    {ERR_CSDK_CLIENT_INTERNAL, "sdk client internal error"},
    {ERR_CSDK_PARSE_JSON, "parse json error"},
    {ERR_CSDK_INVALID_PARAM, "invalid input param"},
    {ERR_CSDK_INVALID_ETAG, "invalid etag"},
    {ERR_CSDK_NO_CONFIG, "no config found"},
    {ERR_CSDK_INVALID_CONFIG, "invalid config"},
    {0, ""}
};

static ret_status_t ret_http_status;

ret_status_t *error_desc(const int ret_code);
ret_status_t *error_desc_http(int retcode, const char *error_message);



#define UFILE_ERROR_DESC(retcode) error_desc(retcode)

#define UFILE_SET_ERROR(retcode)  ret_status = UFILE_ERROR_DESC(retcode)

#define UFILE_SET_ERROR2(retcode, error_message) ret_status = error_desc_http(retcode, error_message);
#endif