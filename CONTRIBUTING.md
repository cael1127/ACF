## Contributing to ACF

Thanks for helping build this C-based compound AI system.

### Code layout

- Core math and model: `src/core`, `src/model`
- Agent harness and tools: `src/agent`, `src/tools`
- Safety and runtime: `src/safety`, `src/runtime`
- Training scaffolding: `src/train`
- Tests: `src/tests`

Code is written in **C11** and built with **CMake**.

### Adding a new tool

1. Implement the tool function in `src/tools/your_tool.c` with signature:
   `int your_tool(const char *args_json, char *result, int result_capacity);`
2. Declare it in `src/tools/your_tool.h`.
3. Register it in `src/agent/tools.c` by adding a `ToolSpec` entry in `tools_init_defaults`.

### Adding a new model backend

1. Implement a backend that fills a `ModelInterface` (see `src/model/interface.h`).
2. Initialize it in your entry point (CLI or server).
3. Register it with the `RouterContext` in `src/agent/router.c` so that the router can select it.

### Tests and CI

- After building, run `compound_ai_tests` to exercise core math, tools, router, memory, and HTTP parsing.
- GitHub Actions (`.github/workflows/ci.yml`) runs the same commands on each push/PR.

