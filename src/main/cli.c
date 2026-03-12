#include "agent/harness.h"
#include "agent/memory.h"
#include "agent/tools.h"
#include "model/external.h"
#include "runtime/log.h"

#include <stdio.h>
#include <string.h>

static int run_eval_script(const char *path, Agent *agent) {
    FILE *f = fopen(path, "r");
    if (!f) {
        log_error("Failed to open eval script");
        return 1;
    }
    char line[2048];
    char response[4096];
    int cases = 0;
    int passed = 0;
    while (fgets(line, sizeof(line), f)) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        if (line[0] == '\0') continue;
        char *sep = strchr(line, '|');
        if (!sep) continue;
        *sep = '\0';
        const char *prompt = line;
        const char *expected = sep + 1;
        if (agent_run_turn(agent, prompt, response, (int)sizeof(response)) == 0) {
            ++cases;
            if (strstr(response, expected) != NULL) {
                ++passed;
                printf("[OK] %s\n", prompt);
            } else {
                printf("[FAIL] %s\nExpected substring: %s\nGot: %s\n", prompt, expected, response);
            }
        }
    }
    fclose(f);
    char msg[128];
    snprintf(msg, sizeof(msg), "Eval finished: %d/%d passed", passed, cases);
    log_info(msg);
    return (cases == passed) ? 0 : 1;
}

int main(int argc, char **argv) {
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

    if (argc >= 3 && strcmp(argv[1], "--eval") == 0) {
        int rc = run_eval_script(argv[2], &agent);
        agent_free(&agent);
        return rc;
    }

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

