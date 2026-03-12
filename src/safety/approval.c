#include "safety/approval.h"

#include <string.h>
#include <stdlib.h>

int approval_required(const char *tool_name, const char *args_json) {
    if (!tool_name || !args_json) return 0;

    const char *allow = getenv("AI_ALLOW_DANGEROUS");
    if (allow && strcmp(allow, "1") == 0) {
        return 0;
    }

    if (strcmp(tool_name, "run_command") == 0) {
        if (strstr(args_json, "rm ") || strstr(args_json, "rm -") ||
            strstr(args_json, "rmdir") || strstr(args_json, "del ") ||
            strstr(args_json, "format") || strstr(args_json, "mkfs") ||
            strstr(args_json, "shutdown") || strstr(args_json, "poweroff")) {
            return 1;
        }
    }
    return 0;
}

