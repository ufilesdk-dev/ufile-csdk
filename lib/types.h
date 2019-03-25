#ifndef __UFILE_TYPES_H_
#define __UFILE_TYPES_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct ufile_error{
    int code;
	const char* message;
};

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define UFILE_OK  0
#define UFILE_ERROR_CODE -1
#define UFILE_CONFIG_ERROR_CODE -2
#define CURL_ERROR_CODE -20

#define NO_ERROR {0, ""} 
#define CONFIG_ERROR {UFILE_ERROR_CODE, "config error, please check your configuration has been initialization."} 

#define HTTP_ERROR_MSG "http is not OK, check the code as HTTP code."
#define CURL_RESPONSE_CODE_ERROR_MSG "Get curl response code failed."
#define CURL_INIT_ERROR_MSG "init curl failed."

#define HTTP_IS_OK(CODE) ((CODE)/100 == 2)
#define UFILE_HAS_ERROR(CODE) ((CODE) != 0 && (CODE)/100 != 2 )

struct ufile_config{
    const char *public_key;   //json: public_key
    const char *private_key;  //json: private_key
    const char *file_host;    //json: file_host
    const char *bucket_host;  //json: bucket_host
};

#ifdef __cplusplus
} //extern "C"
#endif


#endif
