#include "agent/harness.h"
#include "agent/memory.h"
#include "agent/tools.h"
#include "model/external.h"
#include "runtime/log.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    log_info("Starting compound_ai_cli");

    ExternalModelConfig cfg;
    cfg.endpoint = "http://localhost:8000/v1/complete";
    cfg.api_key = "";

    ModelInterface iface;
    if (external_model_init(&iface, &cfg) != 0) {
        log_error("Failed to initialize external model interface");
        return 1;
    }

    tools_init_defaults();

    Agent agent;
    agent_init(&agent, &iface, "executor");

    char line[1024];
    char response[4096];

    puts("Compound AI CLI. Type 'exit' to quit.");
    while (1) {
        fputs("> ", stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        if (strncmp(line, "exit", 4) == 0) {
            break;
        }
        if (agent_run_turn(&agent, line, response, (int)sizeof(response)) == 0) {
            puts(response);
        } else {
            log_error("agent_run_turn failed");
        }
    }

    agent_free(&agent);
    return 0;
}

