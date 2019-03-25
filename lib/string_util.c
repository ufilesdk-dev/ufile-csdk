#include "string_util.h"
#include <stdlib.h>
#include <string.h>

char* ufile_strconcat(const char* s1, ...)
{
	va_list ap; char* p;
	const char* s;
	size_t len, slen, len1 = strlen(s1);

	va_start(ap, s1);
	len = len1;
	for (;;) {
		s = va_arg(ap, const char*);
		if (s == NULL) {
			break;
		}
		len += strlen(s);
	}

	p = (char*)malloc(len + 1);

	va_start(ap, s1);
	memcpy(p, s1, len1);
	len = len1;
	for (;;) {
		s = va_arg(ap, const char*);
		if (s == NULL) {
			break;
		}
		slen = strlen(s);
		memcpy(p + len, s, slen);
		len += slen;
	}
	p[len] = '\0';
	return p;
}