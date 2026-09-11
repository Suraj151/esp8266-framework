/******************************** hmac_sha256 ********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 29th Jul 2025
******************************************************************************/

#ifndef _HMAC_SHA256_H_
#define _HMAC_SHA256_H_

#include <utility/crypto/hash/sha256.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Running HMAC-SHA256 state, so a mac can span data that is not held in
 *        memory all at once.
 */
typedef struct {
    sha256_context m_inner;
    uint8_t m_opad[64];
} hmac_sha256_context;

/**
 * Starts an HMAC-SHA256
 * @param ctx Context to start
 * @param key Pointer to the HMAC key
 * @param key_len Length of the HMAC key
 */
inline void hmac_sha256_init(hmac_sha256_context *ctx, const uint8_t *key, unsigned int key_len){
    uint8_t k_ipad[64] = {0};
    uint8_t tk[32];
    size_t i;

    // If key is longer than block size, hash it
    if (key_len > 64) {
        sha256(key, key_len, tk);
        key = tk;
        key_len = 32;
    }

    memset(ctx->m_opad, 0, 64);
    memcpy(k_ipad, key, key_len);
    memcpy(ctx->m_opad, key, key_len);

    for (i = 0; i < 64; i++) {
        k_ipad[i] ^= 0x36;
        ctx->m_opad[i] ^= 0x5c;
    }

    sha256_init(&ctx->m_inner);
    sha256_update(&ctx->m_inner, k_ipad, 64);
}

/**
 * Adds data to a running HMAC-SHA256
 * @param ctx Context to add to
 * @param data Pointer to the data to be hashed
 * @param data_len Length of the data to be hashed
 */
inline void hmac_sha256_update(hmac_sha256_context *ctx, const uint8_t *data, unsigned int data_len){
    sha256_update(&ctx->m_inner, data, data_len);
}

/**
 * Finishes a running HMAC-SHA256
 * @param ctx Context to finish
 * @param output Pointer to the output buffer for the HMAC (32 bytes)
 */
inline void hmac_sha256_final(hmac_sha256_context *ctx, uint8_t *output){
    uint8_t temp[32];
    sha256_context outer;

    sha256_final(&ctx->m_inner, temp);

    sha256_init(&outer);
    sha256_update(&outer, ctx->m_opad, 64);
    sha256_update(&outer, temp, 32);
    sha256_final(&outer, output);
}

/**
 * Computes HMAC-SHA256
 * @param key Pointer to the HMAC key
 * @param key_len Length of the HMAC key
 * @param data Pointer to the data to be hashed
 * @param data_len Length of the data to be hashed
 * @param output Pointer to the output buffer for the HMAC (32 bytes)
 */
inline void hmac_sha256(const uint8_t *key, unsigned int key_len, const uint8_t *data, unsigned int data_len, uint8_t *output){
    hmac_sha256_context ctx;

    hmac_sha256_init(&ctx, key, key_len);
    hmac_sha256_update(&ctx, data, data_len);
    hmac_sha256_final(&ctx, output);
}

#endif // _HMAC_SHA256_H_
