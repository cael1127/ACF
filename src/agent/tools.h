#ifndef AGENT_TOOLS_H
#define AGENT_TOOLS_H

typedef struct {
    const char *name;
    const char *description;
    int is_write;
    int (*invoke)(const char *args_json, char *result, int result_capacity);
} ToolSpec;

int tools_register(const ToolSpec *tool);
const ToolSpec *tools_find(const char *name);

void tools_init_defaults(void);
int tools_get_all(const ToolSpec **tools, int *count);

#endif /* AGENT_TOOLS_H */

