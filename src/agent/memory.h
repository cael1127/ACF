#ifndef AGENT_MEMORY_H
#define AGENT_MEMORY_H

typedef struct {
    char *role;
    char *content;
} MemoryTurn;

typedef struct {
    MemoryTurn *working;
    int working_count;
    int working_capacity;
    char *episodic_summary;
} AgentMemory;

void memory_init(AgentMemory *m);
void memory_free(AgentMemory *m);

int memory_add_exchange(AgentMemory *m, const char *role, const char *content);
int memory_get_context(const AgentMemory *m, char *out, int out_capacity);

int memory_compact_if_needed(AgentMemory *m, int token_budget);

#endif /* AGENT_MEMORY_H */

