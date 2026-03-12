#ifndef MODEL_INTERFACE_H
#define MODEL_INTERFACE_H

typedef struct {
    const char *name;
    int max_context_tokens;
    int is_external;
} ModelInterfaceInfo;

typedef struct ModelInterface ModelInterface;

typedef int (*ModelInferenceFn)(ModelInterface *iface,
                                const char *system_prompt,
                                const char *input,
                                char *output,
                                int output_capacity);

struct ModelInterface {
    ModelInterfaceInfo info;
    void *impl;
    ModelInferenceFn infer;
};

int model_interface_infer(ModelInterface *iface,
                          const char *system_prompt,
                          const char *input,
                          char *output,
                          int output_capacity);

#endif /* MODEL_INTERFACE_H */

