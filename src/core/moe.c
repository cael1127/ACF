#include "core/moe.h"

int moe_route(const MoEConfig *cfg, const Tensor *inputs, Tensor *outputs) {
    if (!cfg || !inputs || !outputs) return -1;
    if (inputs->dtype != TENSOR_F32 || outputs->dtype != TENSOR_F32) return -1;

    int num_tokens = (int)inputs->shape[0];
    int dim = (int)inputs->shape[1];

    float *in_data = (float *)inputs->data;
    float *out_data = (float *)outputs->data;

    /* Placeholder: identity routing (no-op), copies inputs to outputs. */
    for (int t = 0; t < num_tokens; ++t) {
        for (int d = 0; d < dim; ++d) {
            out_data[t * dim + d] = in_data[t * dim + d];
        }
    }

    (void)cfg;
    return 0;
}

