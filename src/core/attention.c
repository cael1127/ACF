#include "core/attention.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

int attention_scaled_dot_product(const Tensor *q,
                                 const Tensor *k,
                                 const Tensor *v,
                                 Tensor *out) {
    if (!q || !k || !v || !out) return -1;
    if (q->dtype != TENSOR_F32 || k->dtype != TENSOR_F32 || v->dtype != TENSOR_F32 || out->dtype != TENSOR_F32) {
        return -1;
    }

    /* Assume shapes: [seq_q, d_k], [seq_k, d_k], [seq_k, d_v], [seq_q, d_v] */
    int seq_q = (int)q->shape[0];
    int seq_k = (int)k->shape[0];
    int d_k = (int)q->shape[1];
    int d_v = (int)v->shape[1];

    float *q_data = (float *)q->data;
    float *k_data = (float *)k->data;
    float *v_data = (float *)v->data;
    float *o_data = (float *)out->data;

    float scale = 1.0f / sqrtf((float)d_k);

    for (int i = 0; i < seq_q; ++i) {
        float *scores = (float *)malloc((size_t)seq_k * sizeof(float));
        if (!scores) return -1;

        /* Compute scaled dot products q_i * k_j */
        for (int j = 0; j < seq_k; ++j) {
            float sum = 0.0f;
            for (int d = 0; d < d_k; ++d) {
                float qv = q_data[i * d_k + d];
                float kv = k_data[j * d_k + d];
                sum += qv * kv;
            }
            scores[j] = sum * scale;
        }

        /* Numerically stable softmax over scores */
        float max_score = scores[0];
        for (int j = 1; j < seq_k; ++j) {
            if (scores[j] > max_score) max_score = scores[j];
        }
        float denom = 0.0f;
        for (int j = 0; j < seq_k; ++j) {
            scores[j] = expf(scores[j] - max_score);
            denom += scores[j];
        }
        if (denom == 0.0f) denom = 1.0f;
        for (int j = 0; j < seq_k; ++j) {
            scores[j] /= denom;
        }

        /* Weighted sum over V */
        for (int dv = 0; dv < d_v; ++dv) {
            float acc = 0.0f;
            for (int j = 0; j < seq_k; ++j) {
                acc += scores[j] * v_data[j * d_v + dv];
            }
            o_data[i * d_v + dv] = acc;
        }

        free(scores);
    }

    return 0;
}

int attention_flash_blockwise(const Tensor *q,
                              const Tensor *k,
                              const Tensor *v,
                              Tensor *out,
                              int block_q) {
    if (!q || !k || !v || !out) return -1;
    if (block_q <= 0) {
        /* Fallback to full attention. */
        return attention_scaled_dot_product(q, k, v, out);
    }
    if (q->dtype != TENSOR_F32 || k->dtype != TENSOR_F32 || v->dtype != TENSOR_F32 || out->dtype != TENSOR_F32) {
        return -1;
    }

    int seq_q = (int)q->shape[0];
    int seq_k = (int)k->shape[0];
    int d_k = (int)q->shape[1];
    int d_v = (int)v->shape[1];

    float *q_data = (float *)q->data;
    float *k_data = (float *)k->data;
    float *v_data = (float *)v->data;
    float *o_data = (float *)out->data;

    float scale = 1.0f / sqrtf((float)d_k);

    for (int q_start = 0; q_start < seq_q; q_start += block_q) {
        int q_end = q_start + block_q;
        if (q_end > seq_q) q_end = seq_q;
        int cur_q = q_end - q_start;

        for (int i = 0; i < cur_q; ++i) {
            int qi = q_start + i;
            float m_i = -1e30f;
            float l_i = 0.0f;
            float *o_row = &o_data[qi * d_v];
            for (int dv = 0; dv < d_v; ++dv) {
                o_row[dv] = 0.0f;
            }

            for (int kb = 0; kb < seq_k; ++kb) {
                float score = 0.0f;
                for (int d = 0; d < d_k; ++d) {
                    float qv = q_data[qi * d_k + d];
                    float kv = k_data[kb * d_k + d];
                    score += qv * kv;
                }
                score *= scale;

                float m_new = m_i > score ? m_i : score;
                float exp_m_i = expf(m_i - m_new);
                float exp_s = expf(score - m_new);
                float l_new = l_i * exp_m_i + exp_s;

                for (int dv = 0; dv < d_v; ++dv) {
                    float vj = v_data[kb * d_v + dv];
                    o_row[dv] = o_row[dv] * exp_m_i * (l_i / (l_new + 1e-9f)) + vj * (exp_s / (l_new + 1e-9f));
                }

                m_i = m_new;
                l_i = l_new;
            }
        }
    }

    return 0;
}

