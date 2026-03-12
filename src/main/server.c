#include "agent/harness.h"
#include "agent/tools.h"
#include "agent/router.h"
#include "model/external.h"
#include "runtime/http.h"
#include "runtime/log.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    char session_id[64];
    Agent agent;
    int in_use;
} SessionEntry;

#define MAX_SESSIONS 16

static SessionEntry g_sessions[MAX_SESSIONS];
static RouterContext g_router;
static ModelInterface g_external_model;

static Agent *get_or_create_agent(const char *session_id) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use &&
            strcmp(g_sessions[i].session_id, session_id) == 0) {
            return &g_sessions[i].agent;
        }
    }
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!g_sessions[i].in_use) {
            g_sessions[i].in_use = 1;
            strncpy_s(g_sessions[i].session_id, sizeof(g_sessions[i].session_id),
                      session_id, _TRUNCATE);
            agent_init(&g_sessions[i].agent, &g_external_model, "executor");
            return &g_sessions[i].agent;
        }
    }
    return NULL;
}

static void json_extract_string(const char *body, const char *key,
                                char *out, int out_cap) {
    out[0] = '\0';
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(body, pattern);
    if (!p) return;
    const char *q = strchr(p + strlen(pattern), '\"');
    if (!q) return;
    const char *r = strchr(q + 1, '\"');
    if (!r) return;
    int len = (int)(r - (q + 1));
    if (len >= out_cap) len = out_cap - 1;
    memcpy(out, q + 1, (size_t)len);
    out[len] = '\0';
}

static int api_handler(const char *method,
                       const char *path,
                       const char *body,
                       char *response,
                       int response_cap) {
    if (strcmp(method, "POST") == 0 && strcmp(path, "/v1/complete") == 0) {
        char prompt[1024];
        json_extract_string(body, "prompt", prompt, (int)sizeof(prompt));
        char output[2048];
        int rc = model_interface_infer(&g_external_model,
                                       "You are a completion model.",
                                       prompt,
                                       output,
                                       (int)sizeof(output));
        if (rc != 0) {
            snprintf(response, (size_t)response_cap,
                     "{\"error\":\"model error\"}");
        } else {
            snprintf(response, (size_t)response_cap,
                     "{\"completion\":\"%s\"}", output);
        }
        return 0;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/v1/agent") == 0) {
        char session_id[64];
        char input[1024];
        json_extract_string(body, "session_id", session_id, (int)sizeof(session_id));
        json_extract_string(body, "input", input, (int)sizeof(input));
        Agent *agent = get_or_create_agent(session_id);
        if (!agent) {
            snprintf(response, (size_t)response_cap,
                     "{\"error\":\"no session available\"}");
            return 0;
        }
        char answer[2048];
        int rc = agent_run_turn(agent, input, answer, (int)sizeof(answer));
        if (rc != 0) {
            snprintf(response, (size_t)response_cap,
                     "{\"error\":\"agent error\"}");
        } else {
            snprintf(response, (size_t)response_cap,
                     "{\"output\":\"%s\"}", answer);
        }
        return 0;
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/v1/tools") == 0) {
        const ToolSpec *tools;
        int count;
        tools_get_all(&tools, &count);
        int written = snprintf(response, (size_t)response_cap, "{\"tools\":[");
        for (int i = 0; i < count; ++i) {
            const ToolSpec *t = &tools[i];
            written += snprintf(response + written, (size_t)(response_cap - written),
                                "%s{\"name\":\"%s\",\"description\":\"%s\",\"is_write\":%d}",
                                (i == 0 ? "" : ","),
                                t->name,
                                t->description,
                                t->is_write);
            if (written >= response_cap - 2) break;
        }
        snprintf(response + written, (size_t)(response_cap - written), "]}");
        return 0;
    }

    snprintf(response, (size_t)response_cap, "{\"error\":\"not found\"}");
    return 0;
}

int main(void) {
    log_info("Starting compound_ai_server");

    tools_init_defaults();

    ExternalModelConfig cfg;
    cfg.endpoint = "http://localhost:8000/v1/complete";
    cfg.api_key = "";
    if (external_model_init(&g_external_model, &cfg) != 0) {
        log_error("Failed to initialize external model interface");
        return 1;
    }

    router_init(&g_router);
    ModelProfile p;
    p.name = "external-default";
    p.max_context_tokens = 8192;
    p.approximate_latency_ms = 200;
    p.is_heavy = 1;
    router_add_model(&g_router, &p, &g_external_model);

    http_server_start(8080, api_handler);
    return 0;
}

