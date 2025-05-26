#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#define MBEDTLS_PKCS5_C
#define MBEDTLS_SHA256_C

/* Enable only what's needed for HMAC-SHA256 */
#define MBEDTLS_SHA256_C
#define MBEDTLS_MD_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY

/* Include built-in config checks */
#include "extern/mbedtls/include/mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_H */
