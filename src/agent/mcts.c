#include "agent/mcts.h"

int mcts_run(const MCTSConfig *cfg, const char *root_state, char *improved_plan, int capacity) {
    (void)cfg;
    if (!root_state || !improved_plan || capacity <= 0) return -1;
    /* Placeholder: echo back the root state as the "improved" plan. */
    snprintf(improved_plan, (size_t)capacity, "%s", root_state);
    return 0;
}

