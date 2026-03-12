cmake_minimum_required(VERSION 3.16)
project(compound_ai_c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(ENABLE_GPU "Enable GPU kernels (CUDA/OpenCL) if available" OFF)

add_library(core STATIC
    src/core/tensor.c
    src/core/attention.c
    src/core/rope.c
    src/core/moe.c
    src/core/gpu.c
)

target_include_directories(core PUBLIC src)

add_library(model STATIC
    src/model/config.c
    src/model/transformer.c
    src/model/checkpoint.c
    src/model/interface.c
    src/model/external.c
)

target_link_libraries(model PUBLIC core)
target_include_directories(model PUBLIC src)

add_library(agent STATIC
    src/agent/harness.c
    src/agent/tools.c
    src/agent/memory.c
    src/agent/router.c
    src/agent/mcts.c
)

target_link_libraries(agent PUBLIC model)
target_include_directories(agent PUBLIC src)

add_library(tools STATIC
    src/tools/file_tools.c
    src/tools/command_tools.c
)

target_include_directories(tools PUBLIC src)

add_library(safety STATIC
    src/safety/guardrails.c
    src/safety/approval.c
    src/safety/loop_detector.c
)

target_include_directories(safety PUBLIC src)

add_library(runtime STATIC
    src/runtime/log.c
    src/runtime/config.c
    src/runtime/http.c
)

target_include_directories(runtime PUBLIC src)

add_executable(compound_ai_cli src/main/cli.c)
target_link_libraries(compound_ai_cli PRIVATE agent tools safety runtime)

add_executable(compound_ai_server src/main/server.c)
target_link_libraries(compound_ai_server PRIVATE agent tools safety runtime)

add_executable(compound_ai_tests
    src/tests/test_main.c
)
target_link_libraries(compound_ai_tests PRIVATE core model agent tools safety runtime)

