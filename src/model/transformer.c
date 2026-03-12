#include "model/transformer.h"

#include <stdlib.h>

struct Transformer {
    ModelConfig cfg;
};

Transformer *transformer_create(const ModelConfig *cfg) {
    Transformer *m = (Transformer *)malloc(sizeof(Transformer));
    if (!m) return NULL;
    m->cfg = *cfg;
    return m;
}

void transformer_free(Transformer *m) {
    if (!m) return;
    free(m);
}

int transformer_forward(Transformer *m, const int *tokens, int num_tokens, int *next_token) {
    (void)m;
    (void)tokens;
    (void)num_tokens;
    if (!next_token) return -1;
    *next_token = 0;
    return 0;
}

