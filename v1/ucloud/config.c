#include "config.h"
#include "error.h"
#include "json_util.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

char ucloud_public_key[128];
char ucloud_private_key[64];
char ucloud_host_suffix[64];

ret_status_t *init_global_config() {

    ret_status_t *ret_status = NULL;
    FILE *pf = NULL;
    char conf[20];
    char buffer[1024];
    static int inited = 0;
    char ch;
    int i = 0;
    do{
        if (inited) break;
        if (access("./ufilesdk.conf", R_OK) == 0) {
            sprintf(conf, "%s", "./ufilesdk.conf");
        } else if ( access("/etc/ufilesdk.conf", R_OK) == 0) {
            sprintf(conf, "%s", "/etc/ufilesdk.conf");
        } else {
            UFILE_SET_ERROR(ERR_CSDK_NO_CONFIG);
            break;
        }
        
        if ((pf = fopen(conf, "r")) == NULL) {
            UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
           break;
        }

        while(((ch = fgetc(pf)) != EOF) && i < 1024) {
            buffer[i] = ch;
            i++;
        }

        if(get_string_from_data(buffer, "public_key", ucloud_public_key)) {
            UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
            break;
        }

        if(get_string_from_data(buffer, "private_key", ucloud_private_key)) {
            UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
            break;
        }

        if(get_string_from_data(buffer, "proxy_host", ucloud_host_suffix)) {
            UFILE_SET_ERROR(ERR_CSDK_CLIENT_INTERNAL);
            break;
        }
    } while(0);

    if (pf != NULL) {
        inited = 1;
        fclose(pf);
        pf = NULL;
    }
    return ret_status;
}