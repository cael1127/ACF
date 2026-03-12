#ifndef TRAIN_LOOP_H
#define TRAIN_LOOP_H

#include "model/transformer.h"

typedef struct Dataset Dataset;

struct Dataset {
    int (*next_batch)(Dataset *ds, int *tokens, int max_tokens, int *out_len);
    void (*reset)(Dataset *ds);
    void *impl;
};

typedef struct {
    int batch_size;
    int seq_len;
    double learning_rate;
} TrainConfig;

int train_step(const TrainConfig *cfg, Transformer *model, Dataset *ds);
double train_epoch(const TrainConfig *cfg, Transformer *model, Dataset *ds, int steps);
void toy_dataset_init(Dataset *ds);

#endif /* TRAIN_LOOP_H */

