#ifndef RUNTIME_HTTP_H
#define RUNTIME_HTTP_H

typedef int (*HttpHandlerFn)(const char *method,
                             const char *path,
                             const char *body,
                             char *response,
                             int response_cap);

int http_server_start(int port, HttpHandlerFn handler);
int http_handle_request(HttpHandlerFn handler,
                        const char *raw_request,
                        char *response,
                        int response_cap);

#endif /* RUNTIME_HTTP_H */

