#include "model/config.h"

int model_config_load(const char *path, ModelConfig *cfg) {
    (void)path;
    if (!cfg) return -1;
    cfg->d_model = 1024;
    cfg->num_layers = 24;
    cfg->num_heads = 16;
    cfg->vocab_size = 32000;
    return 0;
}

