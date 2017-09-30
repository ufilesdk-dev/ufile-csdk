#ifndef _UFILESDK_C_UCLOUD_STRING_UTIL_
#define _UFILESDK_C_UCLOUD_STRING_UTIL_

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define sdk_tolower(c)      (char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define sdk_toupper(c)      (char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

static void sdk_string_tolower(char *str, char c)
{
    int i = 0;
    while (i < strlen(str)) {
        if (str[i] == c) break;
        str[i] = sdk_tolower(str[i]);
        ++i;
    }
}


static int sdk_is_quote(char c)
{
    return c == '\"';
}

static int adk_is_space(char c)
{
    return ((c == ' ') || (c == '\t'));
}

static int sdk_is_space_or_cntrl(char c)
{
    return c <= ' ';
}

static int sdk_is_line_break(char c)
{
    return ((c == '\r') || (c == '\n'));
}

void sdk_strip_space(char **str);
void sdk_trip_space_and_cntrl(char **str);
void sdk_unquote_str(char **str);
void sdk_strip_line_break(char **str);

char* stristr(const char* pString, const char* pFind);
#ifdef __cplusplus
}
#endif

#endif
