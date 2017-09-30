#include "string_util.h"

typedef int (*sdk_is_char_pt)(char c);

static void sdk_strip_str_func(char **str, sdk_is_char_pt func);


static void sdk_strip_str_func(char **str, sdk_is_char_pt func)
{
    if (str == NULL || *str == NULL) return;
    int offset = 0;
    int len = strlen(*str);
    if (len == 0) return;
    
    while (len > 0 && func((*str)[len - 1])) {
        (*str)[len - 1] = '\0';
        len--;
    }
    
    for (; offset < len && func((*str)[offset]); ++offset) {
        (*str)++;
    }
}

void sdk_unquote_str(char **str)
{
    sdk_strip_str_func(str, sdk_is_quote);
}

void sdk_strip_space(char **str)
{
    sdk_strip_str_func(str, adk_is_space);
}

void sdk_trip_space_and_cntrl(char **str)
{
    sdk_strip_str_func(str, sdk_is_space_or_cntrl);
}

void sdk_strip_line_break(char **str)
{
    sdk_strip_str_func(str, sdk_is_line_break);
}

char* stristr(const char* pString, const char* pFind)
{
    char* char1 = NULL;
    char* char2 = NULL;
    char* char3 = NULL;
    char c1;
    char c2;
    if((pString == NULL) || (pFind == NULL) || (strlen(pString) < strlen(pFind)))
    {
        return NULL;
    }

    for(char1 = (char*)pString; (*char1) != '\0'; ++char1)
    {
        char3 = char1;
        for(char2 = (char*)pFind; (*char2) != '\0' && (*char1) != '\0'; ++char2, ++char1)
        {
            c1 = (*char1) & 0xDF;
            c2 = (*char2) & 0xDF;
            if((c1 != c2) || (((c1 > 0x5A) || (c1 < 0x41)) && (*char1 != *char2)))
                break;
        }

        if((*char2) == '\0')
            return char3;

        char1 = char3;
    }
    return NULL;
}
