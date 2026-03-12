#include "core/rope.h"

int rope_apply(const RoPEConfig *cfg, Tensor *q, Tensor *k, int position_offset) {
    if (!cfg || !q || !k) return -1;
    if (q->dtype != TENSOR_F32 || k->dtype != TENSOR_F32) return -1;

    int head_dim = cfg->head_dim;
    float base = cfg->base;

    float *q_data = (float *)q->data;
    float *k_data = (float *)k->data;

    int seq = (int)q->shape[0];

    for (int pos = 0; pos < seq; ++pos) {
        int idx = position_offset + pos;
        for (int i = 0; i < head_dim; i += 2) {
            float theta = powf(base, -((float)i) / (float)head_dim) * (float)idx;
            float c = cosf(theta);
            float s = sinf(theta);

            float q1 = q_data[pos * head_dim + i];
            float q2 = q_data[pos * head_dim + i + 1];
            float k1 = k_data[pos * head_dim + i];
            float k2 = k_data[pos * head_dim + i + 1];

            q_data[pos * head_dim + i]     = q1 * c - q2 * s;
            q_data[pos * head_dim + i + 1] = q1 * s + q2 * c;
            k_data[pos * head_dim + i]     = k1 * c - k2 * s;
            k_data[pos * head_dim + i + 1] = k1 * s + k2 * c;
        }
    }

    return 0;
}

