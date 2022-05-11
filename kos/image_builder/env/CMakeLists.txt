project (env)

# Toolchain for work with NK-parser.
include (platform/nk)

# Use TLS terminator.
add_compile_definitions (USE_TLS=$ENV{USE_TLS})

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

nk_build_edl_files (env_edl_files
                    NK_MODULE "env"
                    EDL "${CMAKE_SOURCE_DIR}/resources/edl/Env.edl")

add_executable (Env "src/env.c")
add_dependencies (Env env_edl_files)
target_link_libraries (Env ${env_SERVER_LIB})

if (NOT $ENV{UART_OPTION} STREQUAL "")
    target_compile_definitions (Env PRIVATE "-DENABLE_UART")
    target_compile_definitions (Env PRIVATE "-DPORT_NAME=\"${PORT_QEMU}\"")
    target_link_libraries (Env ${uart_CLIENT_LIB})
endif ()
