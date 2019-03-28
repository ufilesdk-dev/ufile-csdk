#include "api.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "encoding.h"
#include "string_util.h"
#include "http.h"
#include "cJSON.h"

extern struct ufile_config *_global_config;
extern int _g_debug_open;

struct bucket_resp{
    const char* msg;
    int code; 
}; 

static void
free_br(struct bucket_resp *br){
    if(br->code != 0){
        free((char*)br->msg);
    }
}

static struct ufile_error
parse_error(struct bucket_resp *br){
    struct ufile_error error = NO_ERROR;
    if(br->code == 0){
        return error;
    }
    printf("RetCode is=%d, message=%s\n", br->code, br->msg);
    error.code = UFILE_BUCKET_REQ_ERROR_CODE; 
    if(br->code >= 15005 &&  br->code <= 15007){
        error.message="操作失败，请稍后重试。";
    }
    switch(br->code){
    case 150:
        error.message = "服务目前不可用，我们正在努力恢复中，请稍后重试。";
        break;
    case 152:
        error.message = "API 错误或后端不可用";
        break;
    case 171:
        error.message = "签名错误。";
        break;
    case 172:
        error.message = "该账号不存在。";
        break;
    case 173:
        error.message = "账户限制";
        break;
    case 15001:
        error.message = "非法的 bucket name";
        break;
    case 15004:
        error.message="服务目前不可用，我们正在努力恢复中，请稍后重试。";
        break;
    case 15023:
        error.message="空间不为空，不可删除。";
        break;
    case 15030:
        error.message="您的CDN配额已满。";
        break;
    case 15037:
        error.message="没有操作权限";
        break;
    case 15041:
        error.message="用户未实名认证，无法创建bucket。";
        break;
    case 15051:
        error.message="空间创建失败，配额不足。";
        break;
    case 15052:
        error.message="未解绑token,无法删除空间。";
        break;
    default:
        error.message="内部错误，请联系技术支持或提交工单。";
        break;
    }
    return error;
}

static struct ufile_error
bucket_request_do(CURL *curl, const char* url, struct bucket_resp *br){
    struct ufile_error error = NO_ERROR;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    if(_g_debug_open != 0){
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    struct http_body response_body;
    memset(&response_body, 0, sizeof(response_body));
    char tmp[2048] = {0}; 
    response_body.buffer_size = 2048;
    response_body.buffer = tmp;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &http_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

    error = curl_do(curl);
    cJSON *json = cJSON_Parse(response_body.buffer);
    if(json==NULL){
        error.code = UFILE_MULTIPLE_INIT_ERROR_CODE;
        error.message = cJSON_GetErrorPtr();
        return error;
    }
    struct cJSON *item = json->child;
    while(item!=NULL){
        if (strcmp(item->string, "RetCode") == 0){
            br->code = item->valueint;
        }else if(strcmp(item->string, "Message") == 0){
            br->msg = ufile_strconcat(item->valuestring, NULL);
        }
        item=item->next;
    }
    if(_g_debug_open != 0 && br->code !=0){
        printf("RetCode=%d;\nmessage=%s\n", br->code, br->msg);
    }

    cJSON_Delete(json);
    return parse_error(br);
}

static struct ufile_error
ufile_bucket_cfg_validation(){
    struct ufile_error error = NO_ERROR;
    if(_global_config == NULL){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "global configuration has not been initialization yet.";
        return error;
    }

    if(strstr(_global_config->public_key, "TOKEN") != NULL){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "bucket 创建必须要公私钥，不支持 token。";
        return error;
    }
    return error;
}

struct ufile_error
ufile_bucket_create(const char *bucket_name, const char* region, const char* bucket_type){
    struct ufile_error error = NO_ERROR;
    error = ufile_bucket_cfg_validation();
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }

    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    if(strcmp(bucket_type, "public") != 0 &&
       strcmp(bucket_type, "private") != 0)
    {
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "bucket_type 必须为 public 或 private";
        return error;
    }

    if(strlen(region) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "region 不能为空";
        return error;
    }

    if(strlen(bucket_name) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "bucket_name 不能为空";
        return error;
    }
    char query[512];
    //这里的签名字符的 query key 必须要按照字母顺序来排。
    char *signature = ufile_strconcat(
        "ActionCreateBucket",//把 action 的key value 组合在了一起。
        "BucketName", bucket_name,
        "PublicKey", _global_config->public_key,
        "Region", region,
        "Type", bucket_type,
        _global_config->private_key, //最后混入私钥
        NULL
    );
    unsigned char hmac[HMAC_LEN]={0};
    SHA1(hmac, (const unsigned char *)signature, strlen(signature));
    char hmac_str[HMAC_LEN*2+1] = {0};
    HMAC2HEX(hmac, hmac_str);
    char escaped_pubkey[100]={0};
    query_escape(escaped_pubkey, _global_config->public_key, 0);
    char url[512]={0};
    sprintf(url, "%s/?Action=CreateBucket&BucketName=%s&Region=%s&Type=%s&Signature=%s&PublicKey=%s",
            _global_config->bucket_host,bucket_name, region, bucket_type, hmac_str, escaped_pubkey);
    free(signature);

    struct bucket_resp br;
    error = bucket_request_do(curl, url, &br);

    free_br(&br);
    curl_easy_cleanup(curl);
    return error;
}

struct ufile_error
ufile_bucket_delete(const char *bucket_name){
    struct ufile_error error = NO_ERROR;
    error = ufile_bucket_cfg_validation();
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }

    if(strlen(bucket_name) == 0){
        error.code = UFILE_CONFIG_ERROR_CODE;
        error.message = "bucket_name 不能为空";
        return error;
    }

    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    char query[512];
    //这里的签名字符的 query key 必须要按照字母顺序来排。
    char *signature = ufile_strconcat(
        "ActionDeleteBucket",//把 action 的key value 组合在了一起。
        "BucketName", bucket_name,
        "PublicKey", _global_config->public_key,
        _global_config->private_key, //最后混入私钥
        NULL
    );

    unsigned char hmac[HMAC_LEN]={0};
    SHA1(hmac, (const unsigned char *)signature, strlen(signature));
    char hex_str[HMAC_LEN*2+1]={0}; 
    HMAC2HEX(hmac, hex_str);

    char escaped_pubkey[100]={0};
    query_escape(escaped_pubkey, _global_config->public_key, 0);

    char url[512]={0};
    sprintf(url, "%s/?Action=DeleteBucket&BucketName=%s&Signature=%s&PublicKey=%s",
            _global_config->bucket_host, bucket_name, hex_str, escaped_pubkey);
    free(signature);

    struct bucket_resp br;
    error = bucket_request_do(curl, url, &br);

    free_br(&br);
    curl_easy_cleanup(curl);
    return error;
}