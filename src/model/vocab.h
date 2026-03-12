#ifndef MODEL_VOCAB_H
#define MODEL_VOCAB_H

/* Simple toy vocabulary for experimentation and tests.
   Encodes ASCII characters as integer IDs and decodes them back. */

int vocab_encode_ascii(const char *text, int *tokens, int max_tokens, int *out_len);
int vocab_decode_ascii(const int *tokens, int len, char *text, int text_cap);

#endif /* MODEL_VOCAB_H */

