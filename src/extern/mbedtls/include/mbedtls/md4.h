/**
 * \file md4.h
 *
 * \brief MD4 message digest algorithm (hash function)
 *
 * \warning MD4 is considered a weak message digest and its use constitutes a
 *          security risk. We recommend considering stronger message digests
 *          instead.
 */
/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 *
 */
#ifndef MBEDTLS_MD4_H
#define MBEDTLS_MD4_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "extern/mbedtls/include/mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <stddef.h>
#include <stdint.h>

/* MBEDTLS_ERR_MD4_HW_ACCEL_FAILED is deprecated and should not be used. */
/** MD4 hardware accelerator failed */
#define MBEDTLS_ERR_MD4_HW_ACCEL_FAILED                   -0x002D

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(MBEDTLS_MD4_ALT)
// Regular implementation
//

/**
 * \brief          MD4 context structure
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
typedef struct mbedtls_md4_context {
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[4];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
}
mbedtls_md4_context;

#else  /* MBEDTLS_MD4_ALT */
#include "md4_alt.h"
#endif /* MBEDTLS_MD4_ALT */

/**
 * \brief          Initialize MD4 context
 *
 * \param ctx      MD4 context to be initialized
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
void mbedtls_md4_init(mbedtls_md4_context *ctx);

/**
 * \brief          Clear MD4 context
 *
 * \param ctx      MD4 context to be cleared
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
void mbedtls_md4_free(mbedtls_md4_context *ctx);

/**
 * \brief          Clone (the state of) an MD4 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
void mbedtls_md4_clone(mbedtls_md4_context *dst,
                       const mbedtls_md4_context *src);

/**
 * \brief          MD4 context setup
 *
 * \param ctx      context to be initialized
 *
 * \return         0 if successful
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 */
int mbedtls_md4_starts_ret(mbedtls_md4_context *ctx);

/**
 * \brief          MD4 process buffer
 *
 * \param ctx      MD4 context
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 *
 * \return         0 if successful
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
int mbedtls_md4_update_ret(mbedtls_md4_context *ctx,
                           const unsigned char *input,
                           size_t ilen);

/**
 * \brief          MD4 final digest
 *
 * \param ctx      MD4 context
 * \param output   MD4 checksum result
 *
 * \return         0 if successful
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
int mbedtls_md4_finish_ret(mbedtls_md4_context *ctx,
                           unsigned char output[16]);

/**
 * \brief          MD4 process data block (internal use only)
 *
 * \param ctx      MD4 context
 * \param data     buffer holding one block of data
 *
 * \return         0 if successful
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
int mbedtls_internal_md4_process(mbedtls_md4_context *ctx,
                                 const unsigned char data[64]);

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
#if defined(MBEDTLS_DEPRECATED_WARNING)
#define MBEDTLS_DEPRECATED      __attribute__((deprecated))
#else
#define MBEDTLS_DEPRECATED
#endif
/**
 * \brief          MD4 context setup
 *
 * \deprecated     Superseded by mbedtls_md4_starts_ret() in 2.7.0
 *
 * \param ctx      context to be initialized
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
MBEDTLS_DEPRECATED void mbedtls_md4_starts(mbedtls_md4_context *ctx);

/**
 * \brief          MD4 process buffer
 *
 * \deprecated     Superseded by mbedtls_md4_update_ret() in 2.7.0
 *
 * \param ctx      MD4 context
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
MBEDTLS_DEPRECATED void mbedtls_md4_update(mbedtls_md4_context *ctx,
                                           const unsigned char *input,
                                           size_t ilen);

/**
 * \brief          MD4 final digest
 *
 * \deprecated     Superseded by mbedtls_md4_finish_ret() in 2.7.0
 *
 * \param ctx      MD4 context
 * \param output   MD4 checksum result
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
MBEDTLS_DEPRECATED void mbedtls_md4_finish(mbedtls_md4_context *ctx,
                                           unsigned char output[16]);

/**
 * \brief          MD4 process data block (internal use only)
 *
 * \deprecated     Superseded by mbedtls_internal_md4_process() in 2.7.0
 *
 * \param ctx      MD4 context
 * \param data     buffer holding one block of data
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
MBEDTLS_DEPRECATED void mbedtls_md4_process(mbedtls_md4_context *ctx,
                                            const unsigned char data[64]);

#undef MBEDTLS_DEPRECATED
#endif /* !MBEDTLS_DEPRECATED_REMOVED */

/**
 * \brief          Output = MD4( input buffer )
 *
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 * \param output   MD4 checksum result
 *
 * \return         0 if successful
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
int mbedtls_md4_ret(const unsigned char *input,
                    size_t ilen,
                    unsigned char output[16]);

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
#if defined(MBEDTLS_DEPRECATED_WARNING)
#define MBEDTLS_DEPRECATED      __attribute__((deprecated))
#else
#define MBEDTLS_DEPRECATED
#endif
/**
 * \brief          Output = MD4( input buffer )
 *
 * \deprecated     Superseded by mbedtls_md4_ret() in 2.7.0
 *
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 * \param output   MD4 checksum result
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
MBEDTLS_DEPRECATED void mbedtls_md4(const unsigned char *input,
                                    size_t ilen,
                                    unsigned char output[16]);

#undef MBEDTLS_DEPRECATED
#endif /* !MBEDTLS_DEPRECATED_REMOVED */

#if defined(MBEDTLS_SELF_TEST)

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 *
 * \warning        MD4 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
int mbedtls_md4_self_test(int verbose);

#endif /* MBEDTLS_SELF_TEST */

#ifdef __cplusplus
}
#endif

#endif /* mbedtls_md4.h */
