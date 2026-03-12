#ifndef MODEL_TRANSFORMER_H
#define MODEL_TRANSFORMER_H

#include "model/config.h"

typedef struct Transformer Transformer;

Transformer *transformer_create(const ModelConfig *cfg);
void         transformer_free(Transformer *m);

int transformer_forward(Transformer *m, const int *tokens, int num_tokens, int *next_token);

#endif /* MODEL_TRANSFORMER_H */

