#ifndef MODEL_CHECKPOINT_H
#define MODEL_CHECKPOINT_H

#include "model/transformer.h"

int checkpoint_load(const char *path, Transformer *m);

#endif /* MODEL_CHECKPOINT_H */

