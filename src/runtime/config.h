#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

typedef struct {
    const char *model_name;
} RuntimeConfig;

int runtime_config_load(const char *path, RuntimeConfig *cfg);

#endif /* RUNTIME_CONFIG_H */

