#include "agent/harness.h"

#include "agent/memory.h"
#include "agent/tools.h"
#include "safety/guardrails.h"
#include "safety/loop_detector.h"
#include "safety/approval.h"

#include <stdio.h>
#include <string.h>

static void agent_set_role(Agent *agent, const char *role) {
    if (!agent) return;
    const char *r = role && role[0] ? role : "executor";
    strncpy_s(agent->role, sizeof(agent->role), r, _TRUNCATE);
}

void agent_init(Agent *agent, ModelInterface *model, const char *role) {
    if (!agent) return;
    memory_init(&agent->memory);
    agent->model = model;
    agent_set_role(agent, role);
}

void agent_free(Agent *agent) {
    if (!agent) return;
    memory_free(&agent->memory);
}

static void hash_args(const char *s, char *out, int out_capacity) {
    unsigned long hash = 5381UL;
    if (s) {
        while (*s) {
            hash = ((hash << 5) + hash) + (unsigned char)(*s);
            ++s;
        }
    }
    snprintf(out, (size_t)out_capacity, "%lx", hash);
}

static int parse_action(const char *model_output, char *tool_name, int tool_name_cap,
                        char *args_json, int args_cap) {
    const char *p = strstr(model_output, "Action:");
    if (!p) return 0;
    p += 7;
    while (*p == ' ' || *p == '\t') ++p;

    int i = 0;
    while (*p && *p != ' ' && *p != '\t' && *p != '{' && i < tool_name_cap - 1) {
        tool_name[i++] = *p++;
    }
    tool_name[i] = '\0';

    while (*p && *p != '{') ++p;
    if (*p != '{') return 0;

    const char *start = p;
    int depth = 0;
    do {
        if (*p == '{') depth++;
        else if (*p == '}') depth--;
        p++;
    } while (*p && depth > 0);

    int len = (int)(p - start);
    if (len >= args_cap) len = args_cap - 1;
    memcpy(args_json, start, (size_t)len);
    args_json[len] = '\0';
    return 1;
}

static int parse_final(const char *model_output, char *final, int final_cap) {
    const char *p = strstr(model_output, "Final:");
    if (!p) return 0;
    p += 6;
    while (*p == ' ' || *p == '\t') ++p;
    strncpy_s(final, (size_t)final_cap, p, _TRUNCATE);
    return 1;
}

int agent_run_turn(Agent *agent, const char *user_input, char *response, int response_capacity) {
    if (!agent || !user_input || !response || response_capacity <= 0) return -1;

    if (!agent->model) {
        strncpy_s(response, (size_t)response_capacity, "No model configured.", _TRUNCATE);
        return 0;
    }

    memory_add_exchange(&agent->memory, "user", user_input);

    const char *system_prompt =
        "You are a compound C-based AI agent.\n"
        "Use a ReAct style protocol:\n"
        "Thought: ...\n"
        "Action: TOOL_NAME {\"arg\": \"value\"}\n"
        "Observation: ... (will be provided)\n"
        "When ready to answer the user, output:\n"
        "Final: <your answer>";

    char model_output[4096];
    for (int step = 0; step < 4; ++step) {
        memory_compact_if_needed(&agent->memory, 2048);

        char context[4096];
        memory_get_context(&agent->memory, context, (int)sizeof(context));

        int rc = model_interface_infer(agent->model, system_prompt, context,
                                       model_output, (int)sizeof(model_output));
        if (rc != 0) {
            strncpy_s(response, (size_t)response_capacity, "Model error.", _TRUNCATE);
            return rc;
        }

        char final_buf[1024];
        if (parse_final(model_output, final_buf, (int)sizeof(final_buf))) {
            strncpy_s(response, (size_t)response_capacity, final_buf, _TRUNCATE);
            memory_add_exchange(&agent->memory, "assistant", response);
            return 0;
        }

        char tool_name[64];
        char args_json[512];
        if (!parse_action(model_output, tool_name, (int)sizeof(tool_name),
                          args_json, (int)sizeof(args_json))) {
            strncpy_s(response, (size_t)response_capacity, model_output, _TRUNCATE);
            memory_add_exchange(&agent->memory, "assistant", response);
            return 0;
        }

        const ToolSpec *tool = tools_find(tool_name);
        char obs[1024];
        if (!tool) {
            snprintf(obs, sizeof(obs), "Unknown tool: %s", tool_name);
        } else {
            char args_hash[64];
            hash_args(args_json, args_hash, (int)sizeof(args_hash));
            loop_detector_record(tool_name, args_hash);
            if (loop_detector_is_stuck()) {
                char seq[256];
                loop_detector_last_sequence(seq, (int)sizeof(seq));
                snprintf(obs, sizeof(obs),
                         "Aborting: repeated calls detected. Sequence: %s", seq);
            } else if (!guardrails_is_tool_allowed(agent->role, tool_name, tool->is_write)) {
                snprintf(obs, sizeof(obs),
                         "Tool %s is not allowed for role %s.", tool_name, agent->role);
            } else if (approval_required(tool_name, args_json)) {
                snprintf(obs, sizeof(obs),
                         "Tool %s requires external approval before execution.", tool_name);
            } else {
                int rc_tool = tool->invoke(args_json, obs, (int)sizeof(obs));
                if (rc_tool != 0) {
                    snprintf(obs, sizeof(obs),
                             "Tool %s failed with code %d.", tool_name, rc_tool);
                }
            }
        }

        memory_add_exchange(&agent->memory, "tool", tool_name);
        memory_add_exchange(&agent->memory, "observation", obs);
    }

    strncpy_s(response, (size_t)response_capacity,
              "Max ReAct steps reached without Final.", _TRUNCATE);
    memory_add_exchange(&agent->memory, "assistant", response);
    return 0;
}

