#include "runtime/config.h"

int runtime_config_load(const char *path, RuntimeConfig *cfg) {
    (void)path;
    if (!cfg) return -1;
    cfg->model_name = "external-default";
    return 0;
}

