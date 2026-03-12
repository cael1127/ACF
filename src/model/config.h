#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

typedef struct {
    int d_model;
    int num_layers;
    int num_heads;
    int vocab_size;
} ModelConfig;

int model_config_load(const char *path, ModelConfig *cfg);

#endif /* MODEL_CONFIG_H */

