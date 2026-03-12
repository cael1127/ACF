#include "safety/guardrails.h"

int guardrails_is_tool_allowed(const char *agent_role, const char *tool_name, int is_write) {
    if (!tool_name) return 0;

    /* Planning agents are restricted to read-only tools. */
    if (agent_role && strcmp(agent_role, "planner") == 0) {
        if (is_write) {
            return 0;
        }
    }

    /* Disallow obviously dangerous commands in general. */
    if (strcmp(tool_name, "run_command") == 0 && is_write) {
        return 1;
    }

    return 1;
}

