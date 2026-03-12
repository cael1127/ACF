#ifndef MODEL_EXTERNAL_H
#define MODEL_EXTERNAL_H

#include "model/interface.h"

typedef struct {
    const char *endpoint;
    const char *api_key;
} ExternalModelConfig;

int external_model_init(ModelInterface *iface, const ExternalModelConfig *cfg);

#endif /* MODEL_EXTERNAL_H */

