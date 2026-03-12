#include "safety/guardrails.h"

#include <string.h>

int guardrails_is_tool_allowed(const char *agent_role, const char *tool_name, int is_write) {
    if (!tool_name) return 0;

    /* Planning agents are restricted to read-only tools. */
    if (agent_role && strcmp(agent_role, "planner") == 0) {
        if (is_write || strcmp(tool_name, "run_command") == 0 || strcmp(tool_name, "edit_file") == 0) {
            return 0;
        }
    }

    /* Executors can use write tools, but dangerous commands are further gated by approval. */
    return 1;
}

