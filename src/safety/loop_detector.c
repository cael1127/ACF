#include "safety/loop_detector.h"

#include <string.h>

typedef struct {
    char tool[64];
    char args[64];
} LoopEntry;

static LoopEntry g_history[3];
static int g_count = 0;

int loop_detector_record(const char *tool_name, const char *args_hash) {
    if (!tool_name || !args_hash) return -1;
    if (g_count < 3) {
        g_count++;
    }
    for (int i = g_count - 1; i > 0; --i) {
        g_history[i] = g_history[i - 1];
    }
    strncpy_s(g_history[0].tool, sizeof(g_history[0].tool), tool_name, _TRUNCATE);
    strncpy_s(g_history[0].args, sizeof(g_history[0].args), args_hash, _TRUNCATE);
    return 0;
}

int loop_detector_is_stuck(void) {
    if (g_count < 3) return 0;
    if (strcmp(g_history[0].tool, g_history[1].tool) == 0 &&
        strcmp(g_history[1].tool, g_history[2].tool) == 0 &&
        strcmp(g_history[0].args, g_history[1].args) == 0 &&
        strcmp(g_history[1].args, g_history[2].args) == 0) {
        return 1;
    }
    return 0;
}

