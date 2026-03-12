#include "train/loop.h"
#include "train/loss.h"

int train_step(const TrainConfig *cfg, Transformer *model, Dataset *ds) {
    (void)cfg;
    (void)model;
    if (!ds || !ds->next_batch) return -1;

    int tokens[1024];
    int len = 0;
    if (ds->next_batch(ds, tokens, (int)(sizeof(tokens) / sizeof(tokens[0])), &len) != 0) {
        return -1;
    }

    /* Placeholder: a real implementation would run forward/backward and update weights.
       Here we simply pretend to compute loss for the last token. */
    (void)tokens;
    (void)len;
    (void)cross_entropy_loss;

    return 0;
}

