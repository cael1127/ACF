#include "tools/file_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *path;
    long offset;
    long limit;
} ReadFileArgs;

static int parse_read_args(const char *args_json, ReadFileArgs *out) {
    if (!args_json || !out) return -1;
    const char *p_path = strstr(args_json, "\"path\"");
    if (!p_path) return -1;
    const char *quote1 = strchr(p_path, '\"');
    if (!quote1) return -1;
    const char *quote2 = strchr(quote1 + 1, '\"');
    if (!quote2) return -1;
    const char *quote3 = strchr(quote2 + 1, '\"');
    if (!quote3) return -1;
    const char *quote4 = strchr(quote3 + 1, '\"');
    if (!quote4) return -1;
    size_t len = (size_t)(quote4 - quote3 - 1);
    char *path = (char *)malloc(len + 1);
    if (!path) return -1;
    memcpy(path, quote3 + 1, len);
    path[len] = '\0';
    out->path = path;
    out->offset = 1;
    out->limit = -1;

    const char *p_off = strstr(args_json, "\"offset\"");
    if (p_off) {
        long off = 0;
        if (sscanf(p_off, "\"offset\"%*[^0-9-]%ld", &off) == 1 && off > 0) {
            out->offset = off;
        }
    }
    const char *p_lim = strstr(args_json, "\"limit\"");
    if (p_lim) {
        long lim = 0;
        if (sscanf(p_lim, "\"limit\"%*[^0-9-]%ld", &lim) == 1 && lim > 0) {
            out->limit = lim;
        }
    }
    return 0;
}

int tool_read_file(const char *args_json, char *result, int result_capacity) {
    if (!result || result_capacity <= 0) return -1;

    ReadFileArgs args;
    if (parse_read_args(args_json, &args) != 0) {
        snprintf(result, (size_t)result_capacity, "Failed to parse arguments.");
        return -1;
    }

    FILE *f = fopen(args.path, "r");
    if (!f) {
        snprintf(result, (size_t)result_capacity, "Failed to open file: %s", args.path);
        free((void *)args.path);
        return -1;
    }

    char line[1024];
    int line_no = 1;
    int written = 0;
    int emitted = 0;
    result[0] = '\0';

    while (fgets(line, sizeof(line), f)) {
        if (line_no >= args.offset &&
            (args.limit < 0 || emitted < args.limit)) {
            int n = snprintf(result + written, (size_t)(result_capacity - written),
                             "%d:%s", line_no, line);
            if (n < 0 || written + n >= result_capacity - 16) {
                snprintf(result + written, (size_t)(result_capacity - written), "\n[TRUNCATED]\n");
                break;
            }
            written += n;
            ++emitted;
        }
        ++line_no;
        if (written >= result_capacity) {
            break;
        }
    }

    fclose(f);
    free((void *)args.path);
    return 0;
}

static int tool_edit_file_exact(const char *path,
                                const char *old_snippet,
                                const char *new_snippet,
                                char *result,
                                int result_capacity) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        snprintf(result, (size_t)result_capacity, "Failed to open file: %s", path);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        snprintf(result, (size_t)result_capacity, "Out of memory.");
        return -1;
    }

    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);

    char *pos = strstr(buf, old_snippet);
    if (!pos) {
        free(buf);
        snprintf(result, (size_t)result_capacity, "Old snippet not found.");
        return -1;
    }

    size_t prefix_len = (size_t)(pos - buf);
    size_t old_len = strlen(old_snippet);
    size_t new_len = strlen(new_snippet);
    size_t suffix_len = strlen(pos + old_len);

    size_t new_size = prefix_len + new_len + suffix_len;
    char *out = (char *)malloc(new_size + 1);
    if (!out) {
        free(buf);
        snprintf(result, (size_t)result_capacity, "Out of memory.");
        return -1;
    }

    memcpy(out, buf, prefix_len);
    memcpy(out + prefix_len, new_snippet, new_len);
    memcpy(out + prefix_len + new_len, pos + old_len, suffix_len);
    out[new_size] = '\0';

    free(buf);

    FILE *fw = fopen(path, "wb");
    if (!fw) {
        free(out);
        snprintf(result, (size_t)result_capacity, "Failed to open file for writing: %s", path);
        return -1;
    }

    fwrite(out, 1, new_size, fw);
    fclose(fw);
    free(out);

    snprintf(result, (size_t)result_capacity, "Edit applied.");
    return 0;
}

static void normalize_no_space(const char *src, char *dst, size_t dst_cap, int *map, int *out_len) {
    int j = 0;
    for (int i = 0; src[i] != '\0' && j < (int)dst_cap - 1; ++i) {
        char c = src[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            continue;
        }
        dst[j] = c;
        if (map) {
            map[j] = i;
        }
        ++j;
    }
    dst[j] = '\0';
    if (out_len) *out_len = j;
}

static int tool_edit_file_whitespace_insensitive(const char *path,
                                                 const char *old_snippet,
                                                 const char *new_snippet,
                                                 char *result,
                                                 int result_capacity) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return -1;
    }
    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);

    int *map = (int *)malloc(((size_t)size + 1) * sizeof(int));
    if (!map) {
        free(buf);
        return -1;
    }

    char *norm = (char *)malloc((size_t)size + 1);
    if (!norm) {
        free(buf);
        free(map);
        return -1;
    }
    int norm_len = 0;
    normalize_no_space(buf, norm, (size_t)size + 1, map, &norm_len);

    char old_norm[512];
    int old_norm_len = 0;
    normalize_no_space(old_snippet, old_norm, sizeof(old_norm), NULL, &old_norm_len);

    char *pos = strstr(norm, old_norm);
    if (!pos) {
        free(buf);
        free(map);
        free(norm);
        return -1;
    }

    int norm_index = (int)(pos - norm);
    int start_index = map[norm_index];

    size_t old_len = strlen(old_snippet);
    size_t prefix_len = (size_t)start_index;
    size_t suffix_len = strlen(buf + start_index + (int)old_len);
    size_t new_len = strlen(new_snippet);

    size_t new_size = prefix_len + new_len + suffix_len;
    char *out = (char *)malloc(new_size + 1);
    if (!out) {
        free(buf);
        free(map);
        free(norm);
        return -1;
    }

    memcpy(out, buf, prefix_len);
    memcpy(out + prefix_len, new_snippet, new_len);
    memcpy(out + prefix_len + new_len, buf + start_index + (int)old_len, suffix_len);
    out[new_size] = '\0';

    FILE *fw = fopen(path, "wb");
    if (!fw) {
        free(buf);
        free(map);
        free(norm);
        free(out);
        return -1;
    }
    fwrite(out, 1, new_size, fw);
    fclose(fw);

    free(buf);
    free(map);
    free(norm);
    free(out);

    snprintf(result, (size_t)result_capacity, "Edit applied (whitespace-insensitive).");
    return 0;
}

int tool_edit_file(const char *args_json, char *result, int result_capacity) {
    if (!args_json || !result || result_capacity <= 0) return -1;

    const char *p_path = strstr(args_json, "\"path\"");
    const char *p_old = strstr(args_json, "\"old\"");
    const char *p_new = strstr(args_json, "\"new\"");
    if (!p_path || !p_old || !p_new) {
        snprintf(result, (size_t)result_capacity, "Missing required fields.");
        return -1;
    }

    /* Very simple string extraction assuming JSON like {"path":"...","old":"...","new":"..."} */
    char path[512];
    char old_snippet[512];
    char new_snippet[512];

    sscanf(p_path, "\"path\"%*[^\"\n]\"%511[^\"]", path);
    sscanf(p_old, "\"old\"%*[^\"\n]\"%511[^\"]", old_snippet);
    sscanf(p_new, "\"new\"%*[^\"\n]\"%511[^\"]", new_snippet);

    if (tool_edit_file_exact(path, old_snippet, new_snippet, result, result_capacity) == 0) {
        return 0;
    }

    if (tool_edit_file_whitespace_insensitive(path, old_snippet, new_snippet,
                                              result, result_capacity) == 0) {
        return 0;
    }

    snprintf(result, (size_t)result_capacity, "Edit failed: snippet not found.");
    return -1;
}

