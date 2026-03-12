#include "train/loss.h"

#include <math.h>

double cross_entropy_loss(const float *logits, int vocab_size, int target_index) {
    if (!logits || vocab_size <= 0 || target_index < 0 || target_index >= vocab_size) {
        return 0.0;
    }
    float max_logit = logits[0];
    for (int i = 1; i < vocab_size; ++i) {
        if (logits[i] > max_logit) max_logit = logits[i];
    }
    double denom = 0.0;
    for (int i = 0; i < vocab_size; ++i) {
        denom += exp((double)(logits[i] - max_logit));
    }
    double log_prob = (double)(logits[target_index] - max_logit) - log(denom);
    return -log_prob;
}

double dpo_loss(const char *prompt, const char *chosen, const char *rejected) {
    (void)prompt;
    (void)chosen;
    (void)rejected;
    /* Placeholder for future Direct Preference Optimization loss. */
    return 0.0;
}

