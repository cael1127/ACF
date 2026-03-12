#include "model/interface.h"

int model_interface_infer(ModelInterface *iface,
                          const char *system_prompt,
                          const char *input,
                          char *output,
                          int output_capacity) {
    if (!iface || !iface->infer) return -1;
    return iface->infer(iface, system_prompt, input, output, output_capacity);
}

