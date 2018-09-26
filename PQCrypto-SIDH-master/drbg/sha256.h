/**
 * \file sha256.h
 *
 * \brief SHA-224 and SHA-256 cryptographic hash function
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 */
#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-256 context structure
 */
typedef struct {
	uint32_t total[2]; /*!< number of bytes processed  */
	uint32_t state[8]; /*!< intermediate digest state  */
	unsigned char buffer[64]; /*!< data block being processed */
	int is224; /*!< 0 => SHA-256, else SHA-224 */
} sha256_context;

/**
 * \brief          Initialize SHA-256 context
 *
 * \param ctx      SHA-256 context to be initialized
 */
void sha256_init(sha256_context *ctx);

/**
 * \brief          Clear SHA-256 context
 *
 * \param ctx      SHA-256 context to be cleared
 */
void sha256_free(sha256_context *ctx);

/**
 * \brief          Clone (the state of) a SHA-256 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void sha256_clone(sha256_context *dst,
		const sha256_context *src);

/**
 * \brief          SHA-256 context setup
 *
 * \param ctx      context to be initialized
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha256_starts(sha256_context *ctx, int is224);

/**
 * \brief          SHA-256 process buffer
 *
 * \param ctx      SHA-256 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha256_update(sha256_context *ctx,
		const unsigned char *input, size_t ilen);

/**
 * \brief          SHA-256 final digest
 *
 * \param ctx      SHA-256 context
 * \param output   SHA-224/256 checksum result
 */
void sha256_finish(sha256_context *ctx,
		unsigned char output[32]);

/* Internal use */
void sha256_process(sha256_context *ctx,
		const unsigned char data[64]);

/**
 * \brief          Output = SHA-256( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-224/256 checksum result
 */
void sha256(const unsigned char *input, size_t ilen,
		unsigned char output[32]);

#ifdef __cplusplus
}
#endif

#endif /* mbedtls_sha256.h */
