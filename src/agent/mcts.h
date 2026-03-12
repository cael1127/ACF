#ifndef AGENT_MCTS_H
#define AGENT_MCTS_H

typedef struct {
    int dummy;
} MCTSConfig;

int mcts_run(const MCTSConfig *cfg, const char *root_state, char *improved_plan, int capacity);

#endif /* AGENT_MCTS_H */

