#ifndef _UFILESDK_C_UCLOUD__BASE64_
#define _UFILESDK_C_UCLOUD__BASE64_

#include <stdio.h>
#include <string.h>
#include <ctype.h> 

enum {
    BASE64_URL = 0,
    BASE64_STD,
};


int base64_encode(unsigned char const* , unsigned int len, int type, char *signature);
int base64_decode(const char *s, int type, char *signature);

#endif
