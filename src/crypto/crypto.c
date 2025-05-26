//  Copyright 2025 Ross Kinard

#include "crypto/crypto.h"

#include "global/config.h"
#include "hal/hal_storage.h"
#include <string.h>

#if defined(CRYPTO_BACKEND_MBEDTLS)
#include "extern/mbedtls/include/mbedtls/md.h"
#include "extern/mbedtls/include/mbedtls/pkcs5.h"
#include "extern/mbedtls/include/mbedtls/platform_util.h"
#elif defined(CRYPTO_BACKEND_TINYCRYPT)
#include "extern/tinycrypt/include/tinycrypt/constants.h"
#include "extern/tinycrypt/include/tinycrypt/hmac.h"
#include "extern/tinycrypt/include/tinycrypt/sha256.h"
#else
#error "No supported crypto backend defined"
#endif

/**
 * secure_zero() wraps mbedtls_platform_zeroize() to securely erase memory.
 * This prevents the compiler from optimizing away memory clearing of sensitive
 * data, such as PINs or cryptographic keys.
 */
status_t
secure_zero(void* data, size_t len)
{
#if defined(CRYPTO_BACKEND_MBEDTLS)
    mbedtls_platform_zeroize(data, len);
#elif defined(CRYPTO_BACKEND_TINYCRYPT)
    volatile uint8_t* p = (volatile uint8_t*) data;
    while (len--)
        *p++ = 0;
#else
    return STATUS_ERR_INTERNAL;
#endif
    return STATUS_OK;
}

status_t
get_hmac_sha256(const uint8_t* key, size_t key_len, const uint8_t* input, size_t input_len,
                uint8_t* output)
{
#if defined(CRYPTO_BACKEND_MBEDTLS)
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md_info)
    {
        return STATUS_ERR_INTERNAL;
    }
    if (mbedtls_md_hmac(md_info, key, key_len, input, input_len, output) != 0)
    {
        return STATUS_ERR_INTERNAL;
    }
    return STATUS_OK;

#elif defined(CRYPTO_BACKEND_TINYCRYPT)
    struct tc_hmac_state_struct state;

    if (tc_hmac_set_key(&state, key, key_len) != TC_CRYPTO_SUCCESS)
    {
        secure_zero(&state, sizeof(state));
        return STATUS_ERR_INTERNAL;
    }

    if (tc_hmac_init(&state) != TC_CRYPTO_SUCCESS)
    {
        secure_zero(&state, sizeof(state));
        return STATUS_ERR_INTERNAL;
    }

    if (tc_hmac_update(&state, input, input_len) != TC_CRYPTO_SUCCESS)
    {
        secure_zero(&state, sizeof(state));
        return STATUS_ERR_INTERNAL;
    }

    if (tc_hmac_final(output, TC_SHA256_DIGEST_SIZE, &state) != TC_CRYPTO_SUCCESS)
    {
        secure_zero(&state, sizeof(state));
        return STATUS_ERR_INTERNAL;
    }

    secure_zero(&state, sizeof(state));
    return STATUS_OK;
#else
    return STATUS_ERR_INTERNAL;
#endif
}

status_t
secure_compare(const uint8_t* a, const uint8_t* b, size_t len)
{
    status_t status = STATUS_ERR_UNINITIALIZED;
    uint8_t  diff   = 0;
    for (size_t i = 0; i < len; i++)
    {
        diff |= a[i] ^ b[i];
    }

    if (0 == diff)
    {
        status = STATUS_OK;
    }
    else
    {
        status = STATUS_ERR_AUTH;
    }
    return status;
}

status_t
compute_internal_hmac(const uint8_t* data, size_t data_len, uint8_t* out_mac, size_t out_len)
{
    status_t status = STATUS_ERR_UNINITIALIZED;
    uint8_t  device_key[DEVICE_KEY_LEN];

    if (out_len < LOCKSYS_HASH_SIZE)
    {
        status = STATUS_ERR_INPUT;
    }
    else
    {
        status = hal_load_device_key(device_key, sizeof(device_key));
    }

    if (STATUS_OK == status)
    {
        status = get_hmac_sha256(device_key, DEVICE_KEY_LEN, data, data_len, out_mac);
    }

    secure_zero(device_key, sizeof(device_key));
    if (STATUS_OK != status)
    {
        memset(out_mac, 0, out_len);
    }

    return status;
}
