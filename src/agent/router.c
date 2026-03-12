#include "agent/router.h"

#include <string.h>

void router_init(RouterContext *ctx) {
    if (!ctx) return;
    ctx->count = 0;
}

int router_add_model(RouterContext *ctx, const ModelProfile *profile, ModelInterface *iface) {
    if (!ctx || !profile || !iface) return -1;
    if (ctx->count >= ROUTER_MAX_MODELS) return -1;
    ctx->profiles[ctx->count] = *profile;
    ctx->models[ctx->count] = iface;
    ctx->count++;
    return 0;
}

static int is_complex_task(const char *task_description, int context_tokens) {
    if (context_tokens > 512) return 1;
    if (!task_description) return 0;
    if (strstr(task_description, "plan") ||
        strstr(task_description, "analyze") ||
        strstr(task_description, "refactor") ||
        strstr(task_description, "debug") ||
        strstr(task_description, "design")) {
        return 1;
    }
    return 0;
}

ModelInterface *router_select(const RouterContext *ctx,
                              const char *task_description,
                              int context_tokens) {
    if (!ctx || ctx->count == 0) return NULL;

    int wants_heavy = is_complex_task(task_description, context_tokens);

    int best_index = -1;
    for (int i = 0; i < ctx->count; ++i) {
        const ModelProfile *p = &ctx->profiles[i];
        if (wants_heavy && !p->is_heavy) {
            continue;
        }
        if (best_index == -1) {
            best_index = i;
            continue;
        }
        const ModelProfile *best = &ctx->profiles[best_index];
        if (wants_heavy) {
            if (p->max_context_tokens > best->max_context_tokens) {
                best_index = i;
            }
        } else {
            if (p->approximate_latency_ms < best->approximate_latency_ms) {
                best_index = i;
            }
        }
    }

    if (best_index == -1) {
    /* Fallback: choose model with lowest latency. */
        best_index = 0;
        for (int i = 1; i < ctx->count; ++i) {
            if (ctx->profiles[i].approximate_latency_ms <
                ctx->profiles[best_index].approximate_latency_ms) {
                best_index = i;
            }
        }
    }

    return ctx->models[best_index];
}

