/********************************************************************************************
* Functions for AES
*
* If USE_OPENSSL flag is defined it uses OpenSSL's AES implementation. 
* Otherwise, it uses a standalone implementation
*********************************************************************************************/

#include <assert.h>
#include <string.h>
#include "aes.h"
#include "aes_local.h"


#ifndef USE_OPENSSL

void AES128_load_schedule(const uint8_t *key, uint8_t *schedule) {
#ifdef AES_ENABLE_NI
    aes128_load_schedule_ni(key, schedule);
#else
    aes128_load_schedule_c(key, schedule);
#endif
}


void aes128_enc(const uint8_t plaintext[16], const uint8_t schedule[176], uint8_t ciphertext[16]) {
#pragma HLS inline
#ifdef AES_ENABLE_NI
    aes128_enc_ni(plaintext, schedule, ciphertext);
#else
    aes128_enc_c(plaintext, schedule, ciphertext);
#endif
}
     
#ifndef DISABLE_NEW_TOP
#include "aes_hw.h"
#ifndef DISABLE_SDS_INTERFACE
void AES128_enc_hw(const uint8_t *plaintext, const size_t plaintext_len, const uint8_t schedule_p[176], uint8_t *ciphertext) {
#else
void AES128_enc_hw(const uint8_t plaintext[BLOCK_BYTES], const size_t plaintext_len, const uint8_t schedule_p[176], uint8_t ciphertext[BLOCK_BYTES]) {
#endif

    aes128_enc_hw(plaintext, plaintext_len, schedule_p, ciphertext);
}
#endif

void AES128_ECB_enc_sch(const uint8_t *plaintext, const size_t plaintext_len, const uint8_t *schedule, uint8_t *ciphertext) {
    assert(plaintext_len % 16 == 0);
#ifndef DISABLE_NEW_TOP
#ifndef DISABLE_SDS_INTERFACE
    AES128_enc_hw(plaintext, plaintext_len, schedule, ciphertext);
#else
    size_t block = 0;
    for (; block < plaintext_len - BLOCK_BYTES; block += BLOCK_BYTES) {
        AES128_enc_hw(plaintext + block, BLOCK_BYTES, schedule, ciphertext + block);
    }
    if( block < plaintext_len) {
        AES128_enc_hw(plaintext + block, plaintext_len - block, schedule, ciphertext + block);
    }
#endif
#else
    for (size_t block = 0; block < plaintext_len / 16; block++) {
        aes128_enc(plaintext + (16 * block), schedule, ciphertext + (16 * block));
    }
#endif
}


void AES128_free_schedule(uint8_t *schedule) {
    memset(schedule, 0, 16*11);
}


void AES256_load_schedule(const uint8_t *key, uint8_t *schedule) {
#ifdef AES_ENABLE_NI
    aes256_load_schedule_ni(key, schedule);
#else
    aes256_load_schedule_c(key, schedule);
#endif
}


static inline void aes256_enc(const uint8_t *plaintext, const uint8_t *schedule, uint8_t *ciphertext) {
#ifdef AES_ENABLE_NI
    aes256_enc_ni(plaintext, schedule, ciphertext);
#else
    aes256_enc_c(plaintext, schedule, ciphertext);
#endif
}
     

void AES256_ECB_enc_sch(const uint8_t *plaintext, const size_t plaintext_len, const uint8_t *schedule, uint8_t *ciphertext) {
    assert(plaintext_len % 16 == 0);
    for (size_t block = 0; block < plaintext_len / 16; block++) {
        aes256_enc(plaintext + (16 * block), schedule, ciphertext + (16 * block));
    }
}


void AES256_free_schedule(uint8_t *schedule) {
    memset(schedule, 0, 16*15);
}

#else  // Use OpenSSL's AES implementation

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}


void AES128_free_schedule(EVP_CIPHER_CTX *schedule) {
    EVP_CIPHER_CTX_free(schedule);
}


void AES256_free_schedule(EVP_CIPHER_CTX *schedule) {
    EVP_CIPHER_CTX_free(schedule);
}


#endif
