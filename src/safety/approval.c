#include "safety/approval.h"

#include <string.h>

int approval_required(const char *tool_name, const char *args_json) {
    if (!tool_name || !args_json) return 0;

    if (strcmp(tool_name, "run_command") == 0) {
        if (strstr(args_json, "rm ") || strstr(args_json, "rmdir") ||
            strstr(args_json, "format") || strstr(args_json, "del ")) {
            return 1;
        }
    }
    return 0;
}

