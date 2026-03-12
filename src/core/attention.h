#ifndef CORE_ATTENTION_H
#define CORE_ATTENTION_H

#include "core/tensor.h"

typedef struct {
    int num_heads;
    int head_dim;
    int num_kv_heads; /* for GQA-style sharing of K/V */
} AttentionConfig;

int attention_scaled_dot_product(const Tensor *q,
                                 const Tensor *k,
                                 const Tensor *v,
                                 Tensor *out);

int attention_flash_blockwise(const Tensor *q,
                              const Tensor *k,
                              const Tensor *v,
                              Tensor *out,
                              int block_q);

#endif /* CORE_ATTENTION_H */

