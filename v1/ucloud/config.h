#ifndef _UFILESDK_C_UCLOUD_CONFIG_
#define _UFILESDK_C_UCLOUD_CONFIG_

#include "error.h"
#include <string.h>

//请在/etc/ufilesdk.conf 或者运行程序的目录下./ufilesdk.conf 配置下列参数.
//参数配置使用 json 格式,模板如下
/*
{
    "public_key": "请把您账户的 API 公钥粘贴于此",
    "private_key": "请把您账户的 API 私钥粘贴于此", 
    "proxy_host": ".ufile.ucloud.cn"
}
*/

//用户公钥
extern char ucloud_public_key[128];
//用户私钥
extern char ucloud_private_key[64];

//上传域名的后缀, 外网上传默认为 ".ufile.ucloud.cn" ,不需要改动
//内网上传需要使用各 region 对应的域名.
extern char ucloud_host_suffix[64];

ret_status_t *init_global_config();

#define USERAGENT ("UFile CSDK/1.0.0")

#endif