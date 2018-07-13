#include "digest.h"
#include "error.h"
#include "hmac_sha1.h"
#include "base64.h"
#include "string.h"
#include <malloc.h>
#include <json-c/json.h>

int sign_with_data(const char *utf8_string2_sign, char *signature) {

    unsigned char digest[21];
    if (utf8_string2_sign == NULL){
        return -1;
    }

    HMAC_SHA1(digest,
              (unsigned char *)ucloud_private_key,
              strlen(ucloud_private_key),
              (unsigned char *)utf8_string2_sign,
              strlen(utf8_string2_sign));

//    memcpy(signature, digest, strlen(digest));
    return base64_encode(digest, 20, BASE64_STD, signature);
}

void JsonHandle(json_object **my_object, const char *str, int type, void *value )
{

    if (value) {
        if (type == 1) //string
		{
//			printf("key[%s]:value[%s]:length[%d]\n", str, (char*)value, strlen((char*)value));
			char *ss = (char *)malloc( sizeof(char) * strlen((char*)value) + 1);
			strncpy(ss, (char*)value, strlen((char*)value));
			ss[strlen((char*)value)]='\0';
//			printf("newstr:%s:len:%d", ss, strlen(ss));
            json_object_object_add(*my_object, str, json_object_new_string(ss));
			free (ss);
        }
        else if (type == 0)//int64
        {
			int64_t *in = (int64_t*)malloc( sizeof(int64_t) );
			in = (int64_t*)value;
			json_object_object_add(*my_object, str, json_object_new_int64(*in));
			free(in);
        }
    }
}

int base64_policy(struct CallbackPolicy *policy, char *target) {

    char cstr[1024] = {0};
    
    json_object *my_object = json_object_new_object();
	
    JsonHandle(&my_object, "returnUrl", 1/*string*/, policy->ReturnUrl);
    JsonHandle(&my_object, "returnBody", 1/*string*/, policy->ReturnBody);
    JsonHandle(&my_object, "callbackUrl", 1/*string*/, policy->CallbackUrl);
    JsonHandle(&my_object, "callbackHost", 1/*string*/, policy->CallbackHost);
    JsonHandle(&my_object, "callbackBody", 1/*string*/, policy->CallbackBody);
    JsonHandle(&my_object, "callbackMethod", 1/*string*/, policy->CallbackMethod);
    JsonHandle(&my_object, "callbackBodyType", 1/*string*/, policy->CallbackBodyType);
    JsonHandle(&my_object, "autogenKeyPattern", 1/*string*/, policy->AutogenKeyPattern);
	JsonHandle(&my_object, "fsizeLimit", 0/*int64*/, policy->FsizeLimit);
    JsonHandle(&my_object, "mimeLimit", 1/*string*/, policy->MimeLimit);

    const char *str = json_object_to_json_string(my_object);
//	printf("json_string:%s, length=%d, sizeof=%d", str, strlen(str), sizeof(*str));
    memcpy(cstr, str, strlen(str));
//	printf("policy str =%s\n\n", cstr);

    return base64_encode(cstr, strlen(cstr), BASE64_STD, target);
}

int token(struct curl_slist *req_headers,
          const char *method,
          const int type,
          const char *bucket,
          const char *key,
          const char *multipartMimetype,
          char *token,
          struct CallbackPolicy *policy){

    int ret = 0;
    const char *mimetype = NULL;
    char signature[1024];
    char utf8_string2_sign[2048];
    char ucloud_headers[1024];
    char *header_md5 = NULL;
    char *header_data = NULL;
    char policy_tmp[1024]; 
    memset(policy_tmp, 0, 1024);
    memset(signature, 0, 1024);
    memset(utf8_string2_sign, 0, 2048);
    memset(ucloud_headers, 0, 1024);
    mimetype = http_request_header("Content-Type", req_headers);
    if (http_request_header("multipart/form-data", req_headers) != NULL) {
        mimetype = multipartMimetype;
    }

    canonicalized_ucloud_headers(req_headers, ucloud_headers);

    header_md5 = http_request_header("Content-MD5", req_headers);
    header_data = http_request_header("Data", req_headers);

	if (policy != NULL){
		printf("--##-- policy is NOT NULL!");
		base64_policy(policy, policy_tmp);
		snprintf(utf8_string2_sign, 2048, "%s\n%s\n%s\n%s\n%s/%s/%s%s",
				 method,
				 header_md5 ? header_md5 : "",
				 mimetype ? mimetype : "",
				 header_data ? header_data : "",
				 ucloud_headers ? ucloud_headers : "",
				 bucket,
				 key,
				 policy_tmp);
		printf("--##-- policy_tmp =%s\n\n", policy_tmp);

	}else{
		printf("--##-- policy is NULL!");
		snprintf(utf8_string2_sign, 2048, "%s\n%s\n%s\n%s\n%s/%s/%s",
				 method,
				 header_md5 ? header_md5 : "",
				 mimetype ? mimetype : "",
				 header_data ? header_data : "",
				 ucloud_headers ? ucloud_headers : "",
				 bucket,
				 key);
		
		ret = sign_with_data(utf8_string2_sign, signature);
		if (ret) return ret;
		
//		printf("It has not policy!!!");
		sprintf(token, "UCloud %s:%s", ucloud_public_key, signature);
		return 0;
	}

    ret = sign_with_data(utf8_string2_sign, signature);
    if (ret) return ret;
    sprintf(token, "UCloud %s:%s:%s", ucloud_public_key, signature, policy_tmp);
    printf("UCloud %s:%s:%s\n", ucloud_public_key, signature, policy_tmp);
    return 0;
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
