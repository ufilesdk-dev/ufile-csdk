#include "api.h"
#include <curl/curl.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "http.h"
#include "cJSON.h"
#include "string_util.h"

struct etag_slist{
    char *etag_buf;
    size_t pos;
    size_t cap;
    pthread_mutex_t mutex;
};

static void
append_etage(struct etag_slist *list, const char *etag, size_t etag_len){
    if(etag_len + list->pos >= list->cap){ //etag_len 总是要多几个字符，因为有 /r/n
        list->cap = (list->cap+etag_len)*2;
        list->etag_buf = (char*)realloc(list->etag_buf, list->cap);
        assert(list->etag_buf != NULL);
    }
    char *write_start=list->etag_buf+list->pos;
    char *p = write_start;
    const char *etag_p = etag;
    while(etag_p != etag+etag_len){
        if(*etag_p != '"' && !isspace(*etag_p)){
            *p++ = *etag_p;
        }
        etag_p++;
    }
    *p++ = ','; // 为后续传给后台的etag列表增加分隔符。
    list->pos += p-write_start;
}

#define ETAG_HEADER "Etag: "
static size_t 
header_cb(char *buffer, size_t size, size_t nitems, void *userdata){
    buffer[nitems] = '\0';
    char *p = NULL;
    if(strstr(buffer, ETAG_HEADER) != NULL){
        size_t buffer_size = nitems * size;
        struct etag_slist *list = (struct etag_slist*)userdata;
        size_t header_len = strlen(ETAG_HEADER);
        pthread_mutex_lock(&list->mutex);
        append_etage(list, buffer+header_len, buffer_size-header_len);
        pthread_mutex_unlock(&list->mutex);
    }

    return nitems * size;
}

static void
free_state(struct ufile_mutipart_state *state){
    free((char*)state->key);
    free((char*)state->bucket);
    free((char*)state->upload_id);

    struct etag_slist *list = state->etags;
    free((char*)list->etag_buf);
    free(list);
}

struct ufile_error
ufile_multiple_upload_init(struct ufile_mutipart_state *self, const char *bucket, const char *key, const char* mime_type){
    struct ufile_error error=NO_ERROR;
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    struct http_options opt;
    error = set_http_options(&opt, "POST", mime_type, bucket, key, "uploads");
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, &opt);
        return error;
    }

    set_curl_options(curl, &opt);

    struct http_body response_body;
    memset(&response_body, 0, sizeof(struct http_body));
    char tmp[1024] = {0};
    response_body.buffer = tmp;
    response_body.buffer_size = 1024;
    response_body.pos_n = 0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

    error = curl_do(curl);
    http_cleanup(curl, &opt);
    if(UFILE_HAS_ERROR(error.code)){
        return error;
    }
    struct cJSON *json=cJSON_Parse(tmp);
    if(json==NULL){
        error.code = UFILE_MULTIPLE_INIT_ERROR_CODE;
        error.message = cJSON_GetErrorPtr();
        return error;
    }
    struct cJSON *item = json->child;
    while(item!=NULL){
        if (strcmp(item->string, "BlkSize") == 0){
            self->part_size = item->valueint;
        }else if(strcmp(item->string, "UploadId") == 0){
            self->upload_id = ufile_strconcat(item->valuestring, NULL);
        }
        item=item->next;
    }
    self->bucket = ufile_strconcat(bucket, NULL);
    self->key = ufile_strconcat(key, NULL);
    self->etags = malloc(sizeof(struct etag_slist));
    self->etags->cap = 0;
    self->etags->pos = 0;
    self->etags->etag_buf = NULL;
    //self->etags->mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&self->etags->mutex, NULL);
    return error;
}

struct ufile_error
ufile_multiple_upload_part(struct ufile_mutipart_state *self, char *buffer, size_t buf_len, int part_number){
    struct ufile_error error=NO_ERROR;
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    struct http_options opt;
    char query[64]={0};
    sprintf(query, "partNumber=%d&uploadId=%s", part_number, self->upload_id);
    error = set_http_options(&opt, "PUT", "", self->bucket, self->key, query);
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, &opt);
        return error;
    }

    set_content_length(&opt, buf_len);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)self->etags);

    struct http_body body;
    memset(&body, 0, sizeof(struct http_body));
    body.buffer = buffer;
    body.buffer_size = buf_len;
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, http_read_cb);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, &body);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)buf_len);
    opt.header = curl_slist_append(opt.header, "Expect: ");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    set_curl_options(curl, &opt);
    error = curl_do(curl);
    http_cleanup(curl, &opt);
    return error;
}

struct ufile_error
ufile_multiple_upload_finish(struct ufile_mutipart_state *self){
    struct ufile_error error=NO_ERROR;
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    struct http_options opt;
    char query[64]={0};
    sprintf(query, "uploadId=%s",self->upload_id);
    error = set_http_options(&opt, "POST", "", self->bucket, self->key, query);
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, &opt);
        return error;
    }

    struct http_body body;
    memset(&body, 0, sizeof(struct http_body));
    struct etag_slist *list = self->etags;
    body.buffer = list->etag_buf;
    body.buffer_size = list->pos;
    set_content_length(&opt, body.buffer_size);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, http_read_cb);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, &body);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)body.buffer_size);
    opt.header = curl_slist_append(opt.header, "Expect: ");

    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    set_curl_options(curl, &opt);

    error = curl_do(curl);
    http_cleanup(curl, &opt);
    free_state(self);
    return error;
}


struct ufile_error
ufile_multiple_upload_abort(struct ufile_mutipart_state *self){
    struct ufile_error error=NO_ERROR;
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        error.code = CURL_ERROR_CODE;
        error.message = CURL_INIT_ERROR_MSG;
        return error;
    }

    struct http_options opt;
    char query[64]={0};
    sprintf(query, "uploadId=%s",self->upload_id);
    error = set_http_options(&opt, "DELETE", "", self->bucket, self->key, query);
    if(UFILE_HAS_ERROR(error.code)){
        http_cleanup(curl, &opt);
        return error;
    }

    set_curl_options(curl, &opt);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    error = curl_do(curl);
    http_cleanup(curl, &opt);
    free_state(self);

    return error;
}
