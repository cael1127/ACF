#include "model/vocab.h"

int vocab_encode_ascii(const char *text, int *tokens, int max_tokens, int *out_len) {
    if (!text || !tokens || max_tokens <= 0) return -1;
    int n = 0;
    while (*text && n < max_tokens) {
        tokens[n++] = (unsigned char)(*text++);
    }
    if (out_len) *out_len = n;
    return 0;
}

int vocab_decode_ascii(const int *tokens, int len, char *text, int text_cap) {
    if (!tokens || !text || text_cap <= 0) return -1;
    int n = len;
    if (n >= text_cap) n = text_cap - 1;
    for (int i = 0; i < n; ++i) {
        text[i] = (char)tokens[i];
    }
    text[n] = '\0';
    return 0;
}

