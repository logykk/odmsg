// main.c
// Format: "ODZ\VERSION" | raw_size(u32 LE) | [ groups ... ]
// Each group: flags(u8), then up to 8 tokens (LSB-first):
//   bit=0 → literal: [u8]
//   bit=1 → match:   [len_minus_MIN_MATCH u8][dist u16 LE], with len in [MIN_MATCH..MAX_MATCH]
//
// Build: gcc -std=c17 -O2 -Wall -Wextra -o odz main.c compress.c decompress.c lz_hashchain.c odz_util.c

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "odz.h"


enum { HASH_BITS = 15, HASH_SIZE = 1 << HASH_BITS };



char* join_argv(char *argv[], int start, int total) {
    // If string wrapped in quotes
    if (total == 1) {
        char *str = argv[start];
        size_t len = strlen(str);

        if (((str[0] == '"' && str[len-1] == '"') || (str[0] == '\'' && str[len-1] == '\''))) {
            // Allocate new string without first and last character
            char* stripped = malloc(len - 1);
            if (!stripped) return NULL;

            // Copy middle part, excluding first and last character
            strncpy(stripped, str + 1, len - 2);
            stripped[len - 2] = '\0';
            
            return stripped;
        } else {
            return str;
        }
    }

    // Calculate total length
    size_t total_length = 0;
    for (int i = 0; i < total; i++) {
        total_length += strlen(argv[i]) + 1;  // +1 for space or null terminator
    }

    // Allocate memory
    char* result = malloc(total_length);
    if (!result) return NULL;

    // First argument
    strcpy(result, argv[start]);

    // Concatenate remaining arguments
    for (int i = start + 1; i < total; i++) {
        strcat(result, " ");
        strcat(result, argv[i]);
    }

    return result;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage:\n  %s c <in>\n  %s d <in>\n", argv[0], argv[0]);
        return 2;
    }

    char mode = argv[1][0];
    char *in = join_argv(argv, 2, argc);
    size_t lin = strlen(in);
    uint8_t *bout = NULL; size_t nout = 0;
    char *msg;

    if (mode == 'c') {
        compress_simple((uint8_t *)in, lin, &bout, &nout);
        msg = encode((char *)bout, nout);
        puts(msg);
    } else if (mode == 'd') {
        msg = decode(in, strlen(in));
        decompress_simple((uint8_t *)msg, lin, &bout, &nout);
        puts((char *)bout);
    } else {
        die("mode must be c or d");
    }

    free(msg); free(bout);
    return 0;
}
