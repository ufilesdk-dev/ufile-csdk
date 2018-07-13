#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <json-c/json.h>
#include <unistd.h>
#include <errno.h>

#include "api.h"
#include "common.h"
#include "error.h"
#include "digest.h"
#include "http.h"
#include "json_util.h"
#include "mimetype.h"

#define UPLOADID_KEY    ("UploadId")
#define BLKSIZE_KEY     ("BlkSize")
#define PART_NUMBER_KEY ("PartNumber")
#define FILESIZE_KEY    ("FileSize")

static int parse_init_result(const char *data, char *upload_id, size_t *blksize) {

    int ret = get_string_from_data(data, UPLOADID_KEY, upload_id);
    if (ret) {
        return ret;
    }
    ret = get_sizet_from_data(data, BLKSIZE_KEY, blksize);
    return ret;
}

static int parse_mupload_result(const char *body, const char *header, etag_t *etag) {

    size_t blk_idx;
    int ret = 0;
    char data[64];
    char *pdata = NULL;
    int i = 0;
    memset(data, 0, 64);
    ret = get_sizet_from_data(body, PART_NUMBER_KEY, &blk_idx);
    if (ret) {
        return ret;
    }

    etag->blk_idx = blk_idx;
    snprintf(data, 64, "%s", stristr(header, "ETag:"));
    pdata = strchr(data, ':') + 1;
    while ( *(pdata + i++) != '\0') {
        if ( *(pdata + i) == '\r' || *(pdata + i) == '\n') {
            pdata[i] = '\0';
            break;
        }
    }
    sdk_strip_space(&pdata);
    sdk_unquote_str(&pdata);
    snprintf(etag->etag, 64, "%s", pdata);
    return ret;
}

static int parse_mfinish_result(const char *body, const char *header, char *etag) {

    int ret = 0;
    char data[64];
    char *pdata = NULL;
    int i = 0;
	
	if (stristr(header,"ETag:") == NULL) 
	{
		return ret;
	}
//	printf("###---###:stristr:%s\n", stristr(header,"ETag:"));
//	printf("###---###:len(stristr):%d:%d\n", sizeof(stristr(header, "ETag:")), strlen(stristr(header,"ETag:")));
    snprintf(data, 64, "%s", stristr(header, "ETag:"));
//	printf("###---###:====================");	
    pdata = strchr(data, ':') + 1;
//	printf("pdata:%s, len=%d\n", pdata, sizeof(pdata));
    while ( *(pdata + i++) != '\0') {
        if ( *(pdata + i) == '\r' || *(pdata + i) == '\n') {
            pdata[i] = '\0';
            break;
        }
    }
    sdk_strip_space(&pdata);
    sdk_unquote_str(&pdata);
    snprintf(etag, 64, "%s", pdata);
    return ret;
}


static ret_status_t *open_file(FILE **pf, const char *file_path) {

    ret_status_t *ret_status = NULL;
    if (access(file_path, R_OK)) {
        UFILE_SET_ERROR2(ERR_CSDK_INVALID_PARAM, strerror(errno));
        return ret_status;
    }
    *pf = fopen(file_path, "r");
    if (*pf == NULL) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        return ret_status;
    }
    return ret_status;
}

static ret_status_t * get_stream_point(FILE *pf, const ssize_t blk_idx, const size_t blksize, size_t *bsize) {

    ret_status_t *ret_status = NULL;
    long length = 0;
    int fd = -1;
    if (fetch_content_length(pf, &length)) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        return ret_status;
    }
    
    if (length - blk_idx*blksize > blksize) {
        *bsize = blksize;
    } else {
        *bsize = length - blk_idx*blksize;
    }

    if (fseek(pf, blk_idx*blksize, SEEK_SET)) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        return ret_status;
    }

    if(fd == -1) fd = fileno(pf);
    lseek(fd, blk_idx*blksize, SEEK_SET);
    return ret_status;
}

static char *etag_to_string(const etag_t *etag, int etag_length) {

    char *etag_string = NULL;
    int etags_size = 0;
    int i = 0;
    for (i = 0; i < etag_length; i++) {
        etags_size += strlen(etag[i].etag);
        if (i < etag_length) {
             etags_size++;
        }
    }

    etag_string = (char*)malloc(etags_size + 1);
    memset(etag_string, 0, etags_size + 1);
    for (i = 0; i < etag_length; i++) {
        strcat(etag_string, etag[i].etag);
        if (i < etag_length) {
            strcat(etag_string, ",");
        }
    }
    return etag_string;
}

ret_status_t *init_multipart_upload(const char *bucket, 
                                    const char *key, 
                                    char *upload_id,
                                    size_t *blksize) {
                    
    ret_status_t *ret_status = NULL;
    struct curl_slist *req_headers = NULL;
    http_options_t http_options;
    char header[256];
    char header_token[256];
    char response_buf[1024];
    char error_message[128];
    CURL *curl;
    int64_t ret = 0;
    int parse_ret = 0;
    long code = 200;
    http_write_param_t http_write_param;


    memset(header, 0, 256);
    memset(header_token, 0, 256);
    memset(http_options.method, 0, sizeof(http_options.method));
    memset(http_options.url, 0, sizeof(http_options.url));
    memset(response_buf, 0, 1024);
    memset(error_message, 0, 128);

    //初始化配置文件
    ret_status = init_global_config();
    if (ret_status) return ret_status;

    curl = http_request_get();

    snprintf(header, 256, "%s:%s", "User-Agent", USERAGENT);
    http_add_header(header, &req_headers);

    sprintf(http_options.method, "%s", "POST");
    ufile_host(bucket, http_options.url);
    sprintf(http_options.url, "%s%s%s", http_options.url, key, "?uploads");
	printf("%s%s%s", http_options.url, key, "?uploads");

    memset(header, 0, 256);
    ret = token(req_headers,
                "POST",
                HEAD_FIELD_CHECK,
                bucket,
                key,
                NULL,
                header_token,
				NULL);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    snprintf(header, 256, "%s:%s", "Authorization", header_token);
    http_add_header(header, &req_headers);
    http_options.req_headers = req_headers;
    http_options.curl = curl;
    
    ret_status = http_set_options(http_options);
    if (ret_status != NULL) {
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    http_write_param.f = NULL;
    http_write_param.buffer = (char*)response_buf;
    ret_status = http_round_trip(curl, NULL, &http_write_param, NULL);
    if (ret_status) {
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    ret = http_respons_eode(curl, &code);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CURL);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    if (code != 200) {
        parse_ret = ufile_error_response(response_buf, &ret, error_message);
        if (parse_ret) {
             UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
             http_cleanup_curl(curl, req_headers);
             return ret_status;
        }
        UFILE_SET_ERROR2(ret, error_message);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }
    else {
        ret = parse_init_result(response_buf, upload_id, blksize);
        if (ret) {
             UFILE_SET_ERROR(ERR_CSDK_PARSE_JSON);
             http_cleanup_curl(curl, req_headers);
             return ret_status;
        }
    }
    http_cleanup_curl(curl, req_headers);
    return ret_status;
}

ret_status_t *upload_multipart(const char *bucket, 
                               const char *key, 
                               const char *upload_id,
                               const char *file_path,
                               etag_t *etag,
                               size_t blksize, 
                               ssize_t blk_idx) {

    ret_status_t *ret_status = NULL;
    struct curl_slist *req_headers = NULL;
    http_options_t http_options;
    char header[256];
    char header_token[256];
    char response_buf[1024];
    char http_header_buf[1024];
    char error_message[128];
    char mimetype[32];
    CURL *curl;
    FILE *pf = NULL;
    int64_t ret = 0;
    int parse_ret = 0;
    long code = 200;
    size_t bsize;

    http_write_param_t http_write_param;
    http_read_param_t http_read_param;
    http_header_param_t http_header_param;


    memset(http_options.method, 0, sizeof(http_options.method));
    memset(http_options.url, 0, sizeof(http_options.url));
    memset(response_buf, 0, 1024);
    memset(http_header_buf, 0, 1024);
    memset(error_message, 0, 128);
    memset(mimetype, 0, 32);
    memset(header, 0, 256);
    memset(header_token, 0, 256);

    //初始化配置文件
    ret_status = init_global_config();
    if (ret_status) return ret_status;

    ret_status = open_file(&pf, file_path);
    if (ret_status) return  ret_status;

    ret_status = fetch_mimetype(pf, file_path, mimetype);
    if (ret_status) {
        fclose(pf);
        return ret_status;
    }

    ret_status = get_stream_point(pf, blk_idx, blksize, &bsize);
    if (ret_status) {
        fclose(pf);
        return  ret_status;
    } 

    curl = http_request_get();

    memset(header, 0, 256);
    snprintf(header, 256, "%s:%s", "Content-Type", mimetype);
    http_add_header(header, &req_headers);

    memset(header, 0, 256);
    snprintf(header, 256, "%s:%s", "User-Agent", USERAGENT);
    http_add_header(header, &req_headers);

    sprintf(http_options.method, "%s", "PUT");
    ufile_host(bucket, http_options.url);
    sprintf(http_options.url, "%s%s%s%s%s%zd", 
             http_options.url, 
             key, 
             "?uploadId=",
             upload_id,
             "&partNumber=",
             blk_idx);

    memset(header, 0, 256);
    ret = token(req_headers,
                "PUT",
                HEAD_FIELD_CHECK,
                bucket,
                key,
                NULL,
                header_token,
				NULL);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        fclose(pf);
        return ret_status;
    }

    snprintf(header, 256, "%s:%s", "Authorization", header_token);
    http_add_header(header, &req_headers);
    http_options.req_headers = req_headers;
    http_options.curl = curl;
    
    ret_status = http_set_options(http_options);
    if (ret_status != NULL) {
        fclose(pf);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    http_write_param.f = NULL;
    http_write_param.buffer = (char*)response_buf;
    http_read_param.f = pf;
    http_read_param.buffer = NULL;
    http_read_param.fsize = bsize;
    http_read_param.need_total_n = bsize;
    http_read_param.read_total_n = 0;
    http_header_param.f = NULL;
    http_header_param.buffer = (char*)http_header_buf;
    ret_status = http_round_trip(curl, &http_read_param, &http_write_param, &http_header_param);
    if (ret_status) {
        fclose(pf);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    ret = http_respons_eode(curl, &code);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CURL);
        fclose(pf);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    if (code != 200) {
        parse_ret = ufile_error_response(response_buf, &ret, error_message);
        if (parse_ret) {
             UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
             http_cleanup_curl(curl, req_headers);
             fclose(pf);
             return ret_status;
        }
        UFILE_SET_ERROR2(ret, error_message);
        http_cleanup_curl(curl, req_headers);
        fclose(pf);
        return ret_status;
    }
    else {
        ret = parse_mupload_result(response_buf, http_header_buf, etag);
        if (ret) {
             UFILE_SET_ERROR(ERR_CSDK_PARSE_JSON);
             http_cleanup_curl(curl, req_headers);
             fclose(pf);
             return ret_status;
        }
    }
    http_cleanup_curl(curl, req_headers);
    fclose(pf);
    return ret_status;

}

ret_status_t *upload_multipart_finish(const char *bucket,
                                      const char *key,
                                      const char *upload_id,
                                      const etag_t *etags,
                                      int etags_length,
                                      char *final_etag) {
                                
    ret_status_t *ret_status = NULL;
    struct curl_slist *req_headers = NULL;
    http_options_t http_options;
    char header[256];
    char header_token[256];
    char response_buf[1024];
    char http_header_buf[1024];
    char error_message[128];
    char *etags_string = NULL;
    CURL *curl;
    int64_t ret = 0;
    int parse_ret = 0;
    long code = 200;
    FILE *ptf = NULL;
    long etags_string_length = 0;

    http_write_param_t http_write_param;
    http_read_param_t http_read_param;
    http_header_param_t http_header_param;

    memset(http_options.method, 0, sizeof(http_options.method));
    memset(http_options.url, 0, sizeof(http_options.url));
    memset(response_buf, 0, 1024);
    memset(http_header_buf, 0, 1024);
    memset(error_message, 0, 128);
    memset(header, 0, 256);

    //初始化配置文件
    ret_status = init_global_config();
    if (ret_status) return ret_status;

    curl = http_request_get();

    snprintf(header, 256, "%s:%s", "User-Agent", USERAGENT);
    http_add_header(header, &req_headers);

    sprintf(http_options.method, "%s", "POST");
    ufile_host(bucket, http_options.url);
    sprintf(http_options.url, "%s%s%s%s", 
             http_options.url, 
             key, 
             "?uploadId=",
             upload_id);

    memset(header, 0, 256);

	struct CallbackPolicy p;
	p.CallbackUrl = "http://106.75.231.229/callbackdemo/demo/world";
	p.CallbackMethod = "GET";
	p.ReturnUrl=NULL;     
	p.ReturnBody=NULL;     
	p.CallbackHost=NULL;     
	p.CallbackBody=NULL;    
	p.CallbackBodyType=NULL;
	p.AutogenKeyPattern=NULL;
	p.FsizeLimit = NULL;
	p.MimeLimit =NULL;

    ret = token(req_headers,
                "POST",
                HEAD_FIELD_CHECK,
                bucket,
                key,
                NULL,
                header_token,
				&p);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    snprintf(header, 256, "%s:%s", "Authorization", header_token);
    http_add_header(header, &req_headers);
    http_options.req_headers = req_headers;
    http_options.curl = curl;

    ret_status = http_set_options(http_options);
    if (ret_status != NULL) {
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }

    etags_string = etag_to_string(etags, etags_length);
    ptf = tmpfile();
    if (ptf == NULL) {
        UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
        free(etags_string);
        etags_string = NULL;
        http_cleanup_curl(curl, req_headers);
        return ret_status;
    }
    fprintf(ptf, "%s", etags_string);
    fseek(ptf, 0, SEEK_SET);

    if (etags_string != NULL) {
        etags_string_length = strlen(etags_string);
        free(etags_string);
        etags_string = NULL;
    }
    

    http_write_param.f = NULL;
    http_write_param.buffer = (char*)response_buf;
    http_read_param.f = ptf;
    http_read_param.buffer = NULL;
    http_read_param.fsize = etags_string_length;
    http_read_param.need_total_n = etags_string_length;
    http_read_param.read_total_n = 0;
    http_header_param.f = NULL;
    http_header_param.buffer = (char*)http_header_buf;
    ret_status = http_round_trip(curl, &http_read_param, &http_write_param, &http_header_param);
    if (ret_status) {
        http_cleanup_curl(curl, req_headers);
        fclose(ptf);
        return ret_status;
    }

    ret = http_respons_eode(curl, &code);
    if (ret) {
        UFILE_SET_ERROR(ERR_CSDK_CURL);
        http_cleanup_curl(curl, req_headers);
        fclose(ptf);
        return ret_status;
    }

	printf("response is %s\n", response_buf);
    if (code != 200) {
        parse_ret = ufile_error_response(response_buf, &ret, error_message);
        if (parse_ret) {
                UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
                http_cleanup_curl(curl, req_headers);
                fclose(ptf);
                return ret_status;
        }
        UFILE_SET_ERROR2(ret, error_message);
        http_cleanup_curl(curl, req_headers);
        fclose(ptf);
        return ret_status;
    }
    else {
        ret = parse_mfinish_result(response_buf, http_header_buf, final_etag);
        if (ret) {
             UFILE_SET_ERROR(ERR_CSDK_PARSE_JSON);
             http_cleanup_curl(curl, req_headers);
             fclose(ptf);
             return ret_status;
        }
    }
    http_cleanup_curl(curl, req_headers);
    fclose(ptf);
    return ret_status;
}
