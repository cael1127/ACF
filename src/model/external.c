#include "model/external.h"

#include <stdio.h>
#include <string.h>

static int external_infer(ModelInterface *iface,
                          const char *system_prompt,
                          const char *input,
                          char *output,
                          int output_capacity) {
    (void)iface;
    if (!output || output_capacity <= 0) return -1;
    /* Placeholder: echo back a simple message combining system prompt and input. */
    snprintf(output, (size_t)output_capacity, "[external-model] %s || %s", system_prompt ? system_prompt : "", input ? input : "");
    return 0;
}

int external_model_init(ModelInterface *iface, const ExternalModelConfig *cfg) {
    if (!iface || !cfg) return -1;
    iface->info.name = "external-http";
    iface->info.max_context_tokens = 8192;
    iface->info.is_external = 1;
    iface->impl = (void *)cfg;
    iface->infer = external_infer;
    return 0;
}

