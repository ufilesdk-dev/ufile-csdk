#ifndef __UFILE_ENCODING_H_
#define __UFILE_ENCODING_H_

#define HMAC_LEN 20

void 
HMAC_SHA1(unsigned char hmac[HMAC_LEN], 
          const unsigned char *key, 
          int key_len,
          const unsigned char *message,
          int message_len
);

void SHA1(unsigned char hmac[HMAC_LEN],const unsigned char *message,
          int message_len);

void 
HMAC2HEX(unsigned char hmac[HMAC_LEN], char* out);

int 
base64decode_len(const char *bufcoded);
int 
base64decode(char *bufplain, const char *bufcoded);

int 
base64encode_len(int len);
int 
base64encode(char *encoded, const char *string, int len);

void
query_escape(char *out, const char* query, unsigned int query_len);
#endif
