#ifndef __UFILE_ENCODING_H_
#define __UFILE_ENCODING_H_

void 
HMAC_SHA1(unsigned char hmac[20]
    , const unsigned char *key
    , int key_len
    , const unsigned char *message
    , int message_len
);



int 
base64decode_len(const char *bufcoded);
int 
base64decode(char *bufplain, const char *bufcoded);

int 
base64encode_len(int len);
int 
base64encode(char *encoded, const char *string, int len);
#endif
