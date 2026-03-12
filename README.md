## Compound AI System in C

This repository implements a compound AI system in C, designed as a birthplace for a new AI with:

- **Core model math**: attention, RoPE, FlashAttention-style kernels, and sparse Mixture-of-Experts.
- **Agentic harness**: ReAct-style loop, multi-model routing, dual-memory, and MCTS-based slow thinking.
- **Tooling layer**: file and command tools suitable for autonomous code and research agents.

### Layout

- `src/core`: Core math primitives (tensors, attention, RoPE, MoE, GPU abstraction).
- `src/model`: Transformer definition, configuration, checkpoints, and model interface.
- `src/agent`: Agent harness, tools registry, memory, routing, and MCTS.
- `src/tools`: File and command tools.
- `src/safety`: Guardrails, approvals, and doom-loop detection.
- `src/runtime`: Logging, configuration, and HTTP server.
- `src/main`: CLI and server entry points.

### Building

This project uses CMake.

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Testing

After building, you can run a small test suite:

```bash
cd build
./compound_ai_tests  # run the test binary directly
```

The tests exercise core math (attention, RoPE), the tool layer (`read_file`, `edit_file`, `run_command`), memory compaction, router selection, and HTTP request parsing.

