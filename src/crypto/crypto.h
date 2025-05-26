//  Copyright 2025 Ross Kinard

#ifndef INCLUDE_CRYPTO_H_
#define INCLUDE_CRYPTO_H_

#include "global/common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Securely zero memory to remove secrets.
 */
status_t
secure_zero(void* data, size_t len);

/**
 * Compute HMAC-SHA256 using a key.
 */
status_t
get_hmac_sha256(const uint8_t* key, size_t key_len, const uint8_t* input, size_t input_len,
                uint8_t* output);

status_t
compute_internal_hmac(const uint8_t* input, size_t input_len, uint8_t* output, size_t out_len);

/**
 * Compare two memory regions in constant time.
 */
status_t
secure_compare(const uint8_t* a, const uint8_t* b, size_t len);

#endif //  INCLUDE_CRYPTO_H_
