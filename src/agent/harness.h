#ifndef AGENT_HARNESS_H
#define AGENT_HARNESS_H

#include "agent/memory.h"
#include "model/interface.h"

typedef struct {
    AgentMemory memory;
    ModelInterface *model;
    char role[16];
} Agent;

void agent_init(Agent *agent, ModelInterface *model, const char *role);
void agent_free(Agent *agent);

int agent_run_turn(Agent *agent, const char *user_input, char *response, int response_capacity);

#endif /* AGENT_HARNESS_H */

