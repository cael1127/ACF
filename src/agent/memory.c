#include "agent/memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *memory_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *p = (char *)malloc(len + 1);
    if (!p) return NULL;
    memcpy(p, s, len);
    p[len] = '\0';
    return p;
}

void memory_init(AgentMemory *m) {
    if (!m) return;
    m->working = NULL;
    m->working_count = 0;
    m->working_capacity = 0;
    m->episodic_summary = NULL;
}

void memory_free(AgentMemory *m) {
    if (!m) return;
    for (int i = 0; i < m->working_count; ++i) {
        free(m->working[i].role);
        free(m->working[i].content);
    }
    free(m->working);
    free(m->episodic_summary);
    m->working = NULL;
    m->episodic_summary = NULL;
    m->working_count = 0;
    m->working_capacity = 0;
}

int memory_add_exchange(AgentMemory *m, const char *role, const char *content) {
    if (!m || !role || !content) return -1;
    if (m->working_count == m->working_capacity) {
        int new_cap = m->working_capacity == 0 ? 8 : m->working_capacity * 2;
        MemoryTurn *new_buf = (MemoryTurn *)realloc(m->working, sizeof(MemoryTurn) * (size_t)new_cap);
        if (!new_buf) return -1;
        m->working = new_buf;
        m->working_capacity = new_cap;
    }
    MemoryTurn *t = &m->working[m->working_count++];
    t->role = memory_strdup(role);
    t->content = memory_strdup(content);
    return 0;
}

int memory_get_context(const AgentMemory *m, char *out, int out_capacity) {
    if (!m || !out || out_capacity <= 0) return -1;
    out[0] = '\0';
    int len = 0;
    if (m->episodic_summary) {
        int written = snprintf(out, (size_t)out_capacity, "%s", m->episodic_summary);
        if (written < 0) return -1;
        len = written;
    }
    for (int i = 0; i < m->working_count; ++i) {
        const MemoryTurn *t = &m->working[i];
        int written = snprintf(out + len, (size_t)(out_capacity - len), "\n[%s] %s", t->role, t->content);
        if (written < 0) break;
        len += written;
        if (len >= out_capacity) break;
    }
    return 0;
}

int memory_compact_if_needed(AgentMemory *m, int token_budget) {
    if (!m || token_budget <= 0) return -1;

    int episodic_chars = 0;
    if (m->episodic_summary) {
        episodic_chars = (int)strlen(m->episodic_summary);
    }
    int working_chars = 0;
    for (int i = 0; i < m->working_count; ++i) {
        working_chars += (int)strlen(m->working[i].role);
        working_chars += (int)strlen(m->working[i].content);
    }

    int approx_tokens = (episodic_chars + working_chars) / 4;
    if (approx_tokens <= token_budget) {
        return 0;
    }

    int keep_recent = 8;
    if (m->working_count <= keep_recent) {
        /* Summarize everything into episodic and clear working. */
        int buf_size = episodic_chars + working_chars + 64;
        char *summary = (char *)malloc((size_t)buf_size);
        if (!summary) return -1;
        summary[0] = '\0';
        if (m->episodic_summary) {
            snprintf(summary, (size_t)buf_size, "%s\n---\n", m->episodic_summary);
        }
        for (int i = 0; i < m->working_count; ++i) {
            int used = (int)strlen(summary);
            int remaining = buf_size - used;
            if (remaining <= 1) break;
            snprintf(summary + used, (size_t)remaining, "[%s] %s\n",
                     m->working[i].role, m->working[i].content);
        }
        free(m->episodic_summary);
        m->episodic_summary = summary;
        for (int i = 0; i < m->working_count; ++i) {
            free(m->working[i].role);
            free(m->working[i].content);
        }
        free(m->working);
        m->working = NULL;
        m->working_count = 0;
        m->working_capacity = 0;
        return 0;
    }

    int cutoff = m->working_count - keep_recent;
    int buf_size = episodic_chars + working_chars + 64;
    char *summary = (char *)malloc((size_t)buf_size);
    if (!summary) return -1;
    summary[0] = '\0';
    if (m->episodic_summary) {
        snprintf(summary, (size_t)buf_size, "%s\n---\n", m->episodic_summary);
    }
    for (int i = 0; i < cutoff; ++i) {
        int used = (int)strlen(summary);
        int remaining = buf_size - used;
        if (remaining <= 1) break;
        snprintf(summary + used, (size_t)remaining, "[%s] %s\n",
                 m->working[i].role, m->working[i].content);
        free(m->working[i].role);
        free(m->working[i].content);
    }

    free(m->episodic_summary);
    m->episodic_summary = summary;

    int new_count = m->working_count - cutoff;
    for (int i = 0; i < new_count; ++i) {
        m->working[i] = m->working[i + cutoff];
    }
    m->working_count = new_count;
    return 0;
}

