#ifndef __UCLOUD_UFILE_HELPLER__
#define __UCLOUD_UFILE_HELPLER__


#include <stdio.h>
#include "../lib/api.h"

static long helper_get_file_size(FILE *fp){
    long fsize;
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return fsize;
}

#endif