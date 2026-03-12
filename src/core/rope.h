#ifndef CORE_ROPE_H
#define CORE_ROPE_H

#include "core/tensor.h"

typedef struct {
    int head_dim;
    float base;
} RoPEConfig;

int rope_apply(const RoPEConfig *cfg, Tensor *q, Tensor *k, int position_offset);

#endif /* CORE_ROPE_H */

