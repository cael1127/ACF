#include "agent/tools.h"

#include "tools/file_tools.h"
#include "tools/command_tools.h"

#include <string.h>

#define MAX_TOOLS 64

static ToolSpec g_tools[MAX_TOOLS];
static int g_num_tools = 0;

int tools_register(const ToolSpec *tool) {
    if (!tool || g_num_tools >= MAX_TOOLS) return -1;
    g_tools[g_num_tools++] = *tool;
    return 0;
}

const ToolSpec *tools_find(const char *name) {
    for (int i = 0; i < g_num_tools; ++i) {
        if (strcmp(g_tools[i].name, name) == 0) {
            return &g_tools[i];
        }
    }
    return NULL;
}

void tools_init_defaults(void) {
    ToolSpec read_tool = {
        "read_file",
        "Read a file with line numbers.",
        0,
        tool_read_file
    };
    tools_register(&read_tool);

    ToolSpec edit_tool = {
        "edit_file",
        "Edit a file using fuzzy matching of old/new snippets.",
        1,
        tool_edit_file
    };
    tools_register(&edit_tool);

    ToolSpec cmd_tool = {
        "run_command",
        "Run a shell command with optional backgrounding.",
        1,
        tool_run_command
    };
    tools_register(&cmd_tool);
}

int tools_get_all(const ToolSpec **tools, int *count) {
    if (!tools || !count) return -1;
    *tools = g_tools;
    *count = g_num_tools;
    return 0;
}

