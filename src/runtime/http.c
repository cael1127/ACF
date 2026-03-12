#include "runtime/http.h"

#include <stdio.h>
#include <string.h>

int http_handle_request(HttpHandlerFn handler,
                        const char *raw_request,
                        char *response,
                        int response_cap) {
    if (!handler || !raw_request || !response || response_cap <= 0) return -1;

    const char *line_end = strstr(raw_request, "\r\n");
    if (!line_end) line_end = strstr(raw_request, "\n");
    if (!line_end) {
        snprintf(response, (size_t)response_cap, "{\"error\":\"bad request\"}");
        return -1;
    }

    char method[8];
    char path[128];
    if (sscanf(raw_request, "%7s %127s", method, path) != 2) {
        snprintf(response, (size_t)response_cap, "{\"error\":\"bad request\"}");
        return -1;
    }

    const char *body = strstr(raw_request, "\r\n\r\n");
    if (body) {
        body += 4;
    } else {
        body = strstr(raw_request, "\n\n");
        if (body) {
            body += 2;
        } else {
            body = "";
        }
    }

    return handler(method, path, body, response, response_cap);
}

int http_server_start(int port, HttpHandlerFn handler) {
    (void)port;
    (void)handler;
    /* Networking not implemented; http_handle_request can be used for testing. */
    return 0;
}

