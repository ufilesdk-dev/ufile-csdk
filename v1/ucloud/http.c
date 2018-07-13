#include "http.h"
#include "error.h"

#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include<stdlib.h>
#include<unistd.h>

ret_status_t *http_initialize() {
    ret_status_t *ret_status = NULL;
    CURLcode ret_code;
    if (CURLE_OK != (ret_code = curl_global_init(CURL_GLOBAL_ALL))) {
        UFILE_SET_ERROR2(ret_code, curl_easy_strerror(ret_code));
    }
    return ret_status;
}

void http_cleanup() {
    curl_global_cleanup();
}

CURL *http_request_get() {
    return curl_easy_init();
}

ret_status_t *http_set_options(const http_options_t http_options) {
    CURLcode ret_code;
    ret_status_t *ret_status = NULL;                                                                 
    curl_easy_setopt_safe(http_options.curl, CURLOPT_URL, http_options.url);
    if (strlen(http_options.method) > 0) {
        curl_easy_setopt_safe(http_options.curl, CURLOPT_CUSTOMREQUEST, http_options.method);
    }

    curl_easy_setopt_safe(http_options.curl, CURLOPT_HTTPHEADER, http_options.req_headers);
    curl_easy_setopt_safe(http_options.curl, CURLOPT_NOSIGNAL, 1);
    return ret_status;
}

void http_add_header(const char *key_value, struct curl_slist **req_headers) {
    *req_headers = curl_slist_append(*req_headers, key_value);
}

char *http_request_header(const char *k, struct curl_slist *req_headers) {
    struct curl_slist *header = req_headers;
    const char *data = NULL;
    char *pos = NULL;
    for (; header != NULL; header = header->next) {
        data = header->data;
        if (strlen(data) < strlen(k)) continue;
        if (memcmp(k, data, strlen(k)) == 0) {
            pos = (char *)strchr(data, ':');
            if (!pos) return NULL;
            pos++;
            sdk_strip_space(&pos);
            return pos;
        }
    }
    return NULL;
}

ret_status_t *http_round_trip(CURL *curl,
                              const http_read_param_t *read_param, 
                              const http_write_param_t *write_param, 
                              const http_header_param_t *header_param) {
    ret_status_t *ret_status = NULL;
    CURLcode ret_code = CURLE_OK;
    if (read_param) {
        curl_easy_setopt_safe(curl, CURLOPT_READFUNCTION, read_cb);
        curl_easy_setopt_safe(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt_safe(curl, CURLOPT_READDATA, read_param);
        curl_easy_setopt_safe(curl, CURLOPT_INFILESIZE_LARGE, read_param->fsize);
    }

    if (write_param) {
        curl_easy_setopt_safe(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt_safe(curl, CURLOPT_WRITEDATA, write_param);
    }

    if (header_param) {
        curl_easy_setopt_safe(curl, CURLOPT_HEADERFUNCTION, write_header_cb);
        curl_easy_setopt_safe(curl, CURLOPT_HEADERDATA, header_param);
    }

    ret_code = curl_easy_perform(curl);
    if (ret_code != CURLE_OK) {
        UFILE_SET_ERROR2(ERR_CSDK_CURL_PERFORM, curl_easy_strerror(ret_code));
        return ret_status;
    }
    return ret_status;
}

int http_respons_eode(CURL *curl, long *code) {
    if (!code || curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, code) != CURLE_OK) {
        return -1;
    }
    return 0;
}

void http_cleanup_curl(CURL *curl, struct curl_slist *req_headers) {
    if (curl) {
        curl_easy_cleanup(curl);
    }

    if (req_headers) {
        curl_slist_free_all(req_headers);
    }
}

size_t writen(FILE *f, char *buffer, char *ptr, const size_t total) {

    int    fd = -1;
    size_t left = total;
    int nw = 0;
    while(left > 0) {
        //优先使用文件对象
        if (f) {
            if (fd == -1) fd = fileno(f);
            nw = write(fd, ptr+total-left, left);
        } else {
			strcat(buffer, ptr+total-left);
            left = 0;
        }

        if (nw <= 0) return total-left;
        left -= nw;
    }
    return total-left;
}

size_t write_cb(char *ptr, size_t size, size_t nmemb, void *user_data) {

    http_write_param_t *param = (http_write_param_t *)user_data;
    if (!param) {
        return 0;
    }
    return writen(param->f, param->buffer, (char *)ptr, size*nmemb);
}

size_t write_header_cb(void *ptr, size_t size, size_t nmemb, void *user_data) {

    http_header_param_t *param = (http_header_param_t*)user_data;
    if (!param) {
        return 0;
    }
    return writen(param->f, param->buffer, (char *)ptr, size*nmemb);
}

size_t read_cb(char *buffer, size_t size, size_t nitems, void *user_data) {

    int     fd = -1;
    int nr = 0;
    size_t  total = size*nitems;
    size_t  need = 0;
    http_read_param_t *param = (http_read_param_t*)user_data;
    if (!param) {
        return 0;
    }

    if (param->need_total_n != 0 && total > param->need_total_n - param->read_total_n) {
        total = param->need_total_n - param->read_total_n;
    }

    need = total;
    while(need > 0) {
        //优先使用文件对象
        if (param->f) {
            if (fd == -1) fd = fileno(param->f);
            nr = read(fd, buffer+total-need, need);
        } else {
            strncpy(buffer+total-need, param->buffer, need);
            nr = 0;
        }

        if (nr <= 0) break;
        need -= nr;
        param->read_total_n += nr;
    }
    return total-need;
}


