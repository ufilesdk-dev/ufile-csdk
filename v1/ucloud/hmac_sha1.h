#ifndef _UFILESDK_C_UCLOUD_HMAC_
#define _UFILESDK_C_UCLOUD_HMAC_


void HMAC_SHA1(unsigned char hmac[20]
    , const unsigned char *key
    , int key_len
    , const unsigned char *message
    , int message_len
);

#endif
