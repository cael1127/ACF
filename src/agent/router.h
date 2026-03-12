#ifndef AGENT_ROUTER_H
#define AGENT_ROUTER_H

#include "model/interface.h"

typedef struct {
    const char *name;
    int max_context_tokens;
    int approximate_latency_ms;
    int is_heavy;
} ModelProfile;

#define ROUTER_MAX_MODELS 4

typedef struct {
    ModelProfile profiles[ROUTER_MAX_MODELS];
    ModelInterface *models[ROUTER_MAX_MODELS];
    int count;
} RouterContext;

void router_init(RouterContext *ctx);
int router_add_model(RouterContext *ctx, const ModelProfile *profile, ModelInterface *iface);
ModelInterface *router_select(const RouterContext *ctx,
                              const char *task_description,
                              int context_tokens);

#endif /* AGENT_ROUTER_H */

