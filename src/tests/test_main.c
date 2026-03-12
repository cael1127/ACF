#include "core/tensor.h"
#include "core/attention.h"
#include "core/rope.h"
#include "agent/memory.h"
#include "agent/router.h"
#include "runtime/http.h"
#include "tools/file_tools.h"
#include "tools/command_tools.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

static void assert_true(int cond, const char *name) {
    ++tests_run;
    if (!cond) {
        ++tests_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[ OK ] %s\n", name);
    }
}

static void test_attention_simple(void) {
    size_t shape[2] = {1, 2};
    Tensor q = tensor_create(2, shape, TENSOR_F32);
    Tensor k = tensor_create(2, shape, TENSOR_F32);
    Tensor v = tensor_create(2, shape, TENSOR_F32);
    Tensor out = tensor_create(2, shape, TENSOR_F32);

    float *qd = (float *)q.data;
    float *kd = (float *)k.data;
    float *vd = (float *)v.data;

    qd[0] = 1.0f; qd[1] = 0.0f;
    kd[0] = 1.0f; kd[1] = 0.0f;
    vd[0] = 0.5f; vd[1] = -0.5f;

    int rc = attention_scaled_dot_product(&q, &k, &v, &out);
    float *od = (float *)out.data;

    assert_true(rc == 0 && od[0] > 0.49f && od[0] < 0.51f, "attention_scaled_dot_product returns expected value");

    tensor_free(&q);
    tensor_free(&k);
    tensor_free(&v);
    tensor_free(&out);
}

static void test_rope_norm_preserved(void) {
    size_t shape[2] = {1, 4};
    Tensor q = tensor_create(2, shape, TENSOR_F32);
    Tensor k = tensor_create(2, shape, TENSOR_F32);

    float *qd = (float *)q.data;
    float *kd = (float *)k.data;
    for (int i = 0; i < 4; ++i) {
        qd[i] = 1.0f;
        kd[i] = 1.0f;
    }

    float before = 0.0f;
    for (int i = 0; i < 4; ++i) before += qd[i] * qd[i];

    RoPEConfig cfg;
    cfg.head_dim = 4;
    cfg.base = 10000.0f;
    rope_apply(&cfg, &q, &k, 0);

    float after = 0.0f;
    for (int i = 0; i < 4; ++i) after += qd[i] * qd[i];

    assert_true(after > 0.99f * before && after < 1.01f * before, "rope_apply approximately preserves norm");

    tensor_free(&q);
    tensor_free(&k);
}

static void test_file_tools_basic(void) {
    const char *path = "test_tmp.txt";
    FILE *f = fopen(path, "w");
    if (!f) {
        assert_true(0, "create test_tmp.txt");
        return;
    }
    fputs("line1\nline2\nline3\n", f);
    fclose(f);

    char args[128];
    snprintf(args, sizeof(args), "{\"path\":\"%s\",\"offset\":2,\"limit\":1}", path);
    char out[256];
    int rc = tool_read_file(args, out, (int)sizeof(out));
    assert_true(rc == 0 && strstr(out, "2:line2") != NULL, "tool_read_file reads expected line");

    snprintf(args, sizeof(args), "{\"path\":\"%s\",\"old\":\"line2\",\"new\":\"LINE_TWO\"}", path);
    rc = tool_edit_file(args, out, (int)sizeof(out));
    assert_true(rc == 0, "tool_edit_file applies edit");
}

static void test_run_command_echo(void) {
    char args[128];
    snprintf(args, sizeof(args), "{\"cmd\":\"echo hello\",\"background\":false,\"timeout_ms\":1000}");
    char out[512];
    int rc = tool_run_command(args, out, (int)sizeof(out));
    assert_true(rc == 0 && strstr(out, "hello") != NULL, "tool_run_command runs echo");
}

static void test_memory_compaction(void) {
    AgentMemory m;
    memory_init(&m);
    for (int i = 0; i < 20; ++i) {
        char msg[64];
        snprintf(msg, sizeof(msg), "message-%d", i);
        memory_add_exchange(&m, "user", msg);
    }
    int before = m.working_count;
    memory_compact_if_needed(&m, 50); /* small budget to force compaction */
    int after = m.working_count;
    int has_summary = (m.episodic_summary && m.episodic_summary[0] != '\0');
    assert_true(after < before && has_summary, "memory_compact_if_needed compacts and summarizes");
    memory_free(&m);
}

static void test_router_selection(void) {
    RouterContext ctx;
    router_init(&ctx);

    ModelInterface m1, m2;
    memset(&m1, 0, sizeof(m1));
    memset(&m2, 0, sizeof(m2));

    ModelProfile p_fast = {"fast", 4096, 50, 0};
    ModelProfile p_heavy = {"heavy", 16384, 200, 1};

    router_add_model(&ctx, &p_fast, &m1);
    router_add_model(&ctx, &p_heavy, &m2);

    ModelInterface *sel_simple = router_select(&ctx, "summarize this", 100);
    ModelInterface *sel_complex = router_select(&ctx, "plan and analyze this complex task", 2000);

    assert_true(sel_simple == &m1, "router_select chooses fast model for simple task");
    assert_true(sel_complex == &m2, "router_select chooses heavy model for complex task");
}

static int dummy_http_handler(const char *method,
                              const char *path,
                              const char *body,
                              char *response,
                              int response_cap) {
    (void)body;
    snprintf(response, (size_t)response_cap,
             "{\"method\":\"%s\",\"path\":\"%s\"}", method, path);
    return 0;
}

static void test_http_handle_request(void) {
    const char *req =
        "POST /v1/complete HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 18\r\n"
        "\r\n"
        "{\"prompt\":\"hi\"}";
    char resp[256];
    int rc = http_handle_request(dummy_http_handler, req, resp, (int)sizeof(resp));
    assert_true(rc == 0 && strstr(resp, "\"path\":\"/v1/complete\"") != NULL,
                "http_handle_request parses method and path");
}

int main(void) {
    printf("Running compound_ai_tests...\n");

    test_attention_simple();
    test_rope_norm_preserved();
    test_file_tools_basic();
    test_run_command_echo();
    test_memory_compaction();
    test_router_selection();
    test_http_handle_request();

    printf("Tests run: %d, failed: %d\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}

