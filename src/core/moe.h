#ifndef CORE_MOE_H
#define CORE_MOE_H

#include "core/tensor.h"

typedef struct {
    int num_experts;
    int top_k;
} MoEConfig;

int moe_route(const MoEConfig *cfg, const Tensor *inputs, Tensor *outputs);

#endif /* CORE_MOE_H */

