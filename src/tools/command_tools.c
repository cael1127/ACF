#include "tools/command_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>

int tool_run_command(const char *args_json, char *result, int result_capacity) {
    if (!args_json || !result || result_capacity <= 0) return -1;

    const char *p_cmd = strstr(args_json, "\"cmd\"");
    if (!p_cmd) {
        snprintf(result, (size_t)result_capacity, "Missing cmd field.");
        return -1;
    }

    char cmd[512];
    sscanf(p_cmd, "\"cmd\"%*[^\"\n]\"%511[^\"]", cmd);

    int background = 0;
    int timeout_ms = 0;
    const char *p_bg = strstr(args_json, "\"background\"");
    if (p_bg && strstr(p_bg, "true")) {
        background = 1;
    }
    const char *p_to = strstr(args_json, "\"timeout_ms\"");
    if (p_to) {
        sscanf(p_to, "\"timeout_ms\"%*[^0-9]%d", &timeout_ms);
    }

    if (background) {
        FILE *pipe_bg = _popen(cmd, "r");
        if (!pipe_bg) {
            snprintf(result, (size_t)result_capacity, "{\"status\":\"error\",\"message\":\"Failed to start command\"}");
            return -1;
        }
        _pclose(pipe_bg);
        snprintf(result, (size_t)result_capacity,
                 "{\"status\":\"ok\",\"mode\":\"background\",\"cmd\":\"%s\"}", cmd);
        return 0;
    }

    FILE *pipe = _popen(cmd, "r");
    if (!pipe) {
        snprintf(result, (size_t)result_capacity, "{\"status\":\"error\",\"message\":\"Failed to start command\"}");
        return -1;
    }

    int written = 0;
    int cap = result_capacity;
    char cwd[512];
    if (!_getcwd(cwd, (int)sizeof(cwd))) {
        strcpy_s(cwd, sizeof(cwd), "");
    }
    snprintf(result, (size_t)cap, "{\"status\":\"ok\",\"mode\":\"foreground\",\"cwd\":\"%s\",\"output\":\"", cwd);
    written = (int)strlen(result);

    char buffer[256];
    clock_t start = clock();
    while (fgets(buffer, sizeof(buffer), pipe)) {
        if (timeout_ms > 0) {
            clock_t now = clock();
            double elapsed_ms = (double)(now - start) * 1000.0 / (double)CLOCKS_PER_SEC;
            if (elapsed_ms > (double)timeout_ms) {
                break;
            }
        }
        for (char *p = buffer; *p; ++p) {
            char c = *p;
            if (c == '\"' || c == '\\') {
                if (written + 2 >= cap - 3) {
                    goto done;
                }
                result[written++] = '\\';
                result[written++] = c;
            } else if (c == '\n') {
                if (written + 2 >= cap - 3) goto done;
                result[written++] = '\\';
                result[written++] = 'n';
            } else {
                if (written + 1 >= cap - 3) goto done;
                result[written++] = c;
            }
        }
        if (written >= cap - 3) {
            break;
        }
    }

done:
    result[written++] = '\"';
    int exit_code = _pclose(pipe);
    int n = snprintf(result + written, (size_t)(cap - written),
                     ",\"exit_code\":%d}", exit_code);
    if (n < 0) {
        result[written] = '\0';
    }

    return 0;
}

