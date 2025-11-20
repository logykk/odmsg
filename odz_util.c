#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "odz.h"

const char* ENCODING_CHARS = "!@#$%^&*()_+-={}|[]:;<>,.?/~`\\";

void die(const char *m) { fprintf(stderr, "err: %s\n", m); exit(1); }

void wr_u32le(uint8_t *dst, uint32_t x) {
	dst[0]=x&0xFF; dst[1]=(x>>8)&0xFF; dst[2]=(x>>16)&0xFF; dst[3]=(x>>24)&0xFF;
}
uint32_t rd_u32le(const uint8_t *src) {
	return (uint32_t)src[0] | ((uint32_t)src[1]<<8) | ((uint32_t)src[2]<<16) | ((uint32_t)src[3]<<24);
}

char* encode(const char* input, size_t input_len) {
    // Leave the first 3 characters unencoded; encode the rest (each char becomes 2 chars)
    size_t prefix_len = input_len < 3 ? input_len : 3;
    size_t encoded_len = (input_len <= prefix_len)
                         ? input_len
                         : prefix_len + 2 * (input_len - prefix_len);

    char* encoded = malloc((encoded_len + 1) * sizeof(char));
    if (!encoded) return NULL;

    // Copy the first prefix_len bytes as-is
    for (size_t i = 0; i < prefix_len; ++i) {
        encoded[i] = input[i];
    }

    // Encode the remainder
    for (size_t i = prefix_len; i < input_len; ++i) {
        unsigned char ch = (unsigned char)input[i];

        // Split the byte into two 4-bit parts
        unsigned char high = (ch >> 4) & 0x0F;
        unsigned char low = ch & 0x0F;

        // Map each 4-bit part to a non-alphanumeric character
        size_t pos = prefix_len + (i - prefix_len) * 2;
        encoded[pos] = ENCODING_CHARS[high];
        encoded[pos + 1] = ENCODING_CHARS[low];
    }

    // Null-terminate the string
    encoded[encoded_len] = '\0';
    
    return encoded;
}

char* decode(const char* encoded, size_t encoded_len) {
    // Leave the first 3 characters unmodified, decode the remainder (2 chars -> 1 byte)
    size_t prefix_len = encoded_len < 3 ? encoded_len : 3;

    if (encoded_len <= prefix_len) {
        // Nothing to decode; just copy the prefix
        char* decoded = malloc((prefix_len + 1) * sizeof(char));
        if (!decoded) return NULL;
        memcpy(decoded, encoded, prefix_len);
        decoded[prefix_len] = '\0';
        return decoded;
    }

    size_t rem = encoded_len - prefix_len;
    if ((rem % 2) != 0) {
        // Invalid encoding length
        return NULL;
    }

    size_t decoded_len = prefix_len + rem / 2;
    char* decoded = malloc((decoded_len + 1) * sizeof(char));
    if (!decoded) return NULL;

    // Copy first prefix_len bytes unchanged
    memcpy(decoded, encoded, prefix_len);

    // Decode the remainder
    for (size_t i = 0; i < rem; i += 2) {
        char* high_ptr = strchr(ENCODING_CHARS, encoded[prefix_len + i]);
        char* low_ptr = strchr(ENCODING_CHARS, encoded[prefix_len + i + 1]);

        if (!high_ptr || !low_ptr) {
            free(decoded);
            return NULL; // Invalid encoding
        }

        // Calculate 4-bit values
        unsigned char high = (unsigned char)(high_ptr - ENCODING_CHARS);
        unsigned char low = (unsigned char)(low_ptr - ENCODING_CHARS);

        // Reconstruct original byte
        decoded[prefix_len + i / 2] = (unsigned char)((high << 4) | low);
    }

    // Null-terminate the string
    decoded[decoded_len] = '\0';
    
    return decoded;
}
