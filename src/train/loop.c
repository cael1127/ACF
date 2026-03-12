#include "train/loop.h"
#include "train/loss.h"
#include "runtime/log.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    int *data;
    int length;
    int index;
} ToyDatasetImpl;

static int toy_next_batch(Dataset *ds, int *tokens, int max_tokens, int *out_len) {
    if (!ds || !ds->impl || !tokens || max_tokens <= 0) return -1;
    ToyDatasetImpl *impl = (ToyDatasetImpl *)ds->impl;
    int n = 0;
    for (; n < max_tokens; ++n) {
        tokens[n] = impl->data[impl->index];
        impl->index = (impl->index + 1) % impl->length;
    }
    if (out_len) *out_len = n;
    return 0;
}

static void toy_reset(Dataset *ds) {
    if (!ds || !ds->impl) return;
    ToyDatasetImpl *impl = (ToyDatasetImpl *)ds->impl;
    impl->index = 0;
}

void toy_dataset_init(Dataset *ds) {
    if (!ds) return;
    static int toy_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    ToyDatasetImpl *impl = (ToyDatasetImpl *)malloc(sizeof(ToyDatasetImpl));
    if (!impl) return;
    impl->data = toy_data;
    impl->length = (int)(sizeof(toy_data) / sizeof(toy_data[0]));
    impl->index = 0;
    ds->next_batch = toy_next_batch;
    ds->reset = toy_reset;
    ds->impl = impl;
}

int train_step(const TrainConfig *cfg, Transformer *model, Dataset *ds) {
    if (!cfg || !model || !ds || !ds->next_batch) return -1;

    int max_tokens = cfg->seq_len;
    if (max_tokens <= 1) max_tokens = 2;

    int *tokens = (int *)malloc((size_t)max_tokens * sizeof(int));
    if (!tokens) return -1;

    int len = 0;
    if (ds->next_batch(ds, tokens, max_tokens, &len) != 0 || len <= 1) {
        free(tokens);
        return -1;
    }

    /* Placeholder forward: transformer_forward currently produces only next token.
       Here we simply pretend we have logits and compute a dummy loss based on tokens. */
    float logits[16];
    int vocab_size = 10;
    for (int i = 0; i < vocab_size && i < (int)(sizeof(logits) / sizeof(logits[0])); ++i) {
        logits[i] = 0.0f;
    }

    double total_loss = 0.0;
    int count = 0;
    for (int t = 0; t < len - 1; ++t) {
        int target = tokens[t + 1] % vocab_size;
        logits[target] = 1.0f;
        total_loss += cross_entropy_loss(logits, vocab_size, target);
        logits[target] = 0.0f;
        ++count;
    }

    free(tokens);

    double avg_loss = (count > 0) ? total_loss / (double)count : 0.0;
    char msg[128];
    snprintf(msg, sizeof(msg), "train_step avg loss: %.4f", avg_loss);
    log_info(msg);

    return 0;
}

double train_epoch(const TrainConfig *cfg, Transformer *model, Dataset *ds, int steps) {
    if (!cfg || !model || !ds || steps <= 0) return 0.0;
    double sum_loss = 0.0;
    for (int i = 0; i < steps; ++i) {
        int rc = train_step(cfg, model, ds);
        (void)rc;
        /* For now we don't parse the log output; this is a placeholder. */
    }
    return sum_loss;
}

