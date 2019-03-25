#ifndef __UFILE_AUTH_H_
#define __UFILE_AUTH_H_


char * ufile_file_authorization(const char *public_key, const char *private_key,
                                const char *method, const char *bucket, 
                                const char *key, const char *mime_type,
                                const char *date, const char *md5);
#endif