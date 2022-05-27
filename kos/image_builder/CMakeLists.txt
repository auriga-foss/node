cmake_minimum_required (VERSION 3.12)
project (node)

# Initialize CMake library for Kaspersky OS SDK.
include (platform)
initialize_platform ()

# Add functions for installing artefacts.
include (platform/install)

# Toolchain for work with NK-parser.
include (platform/nk)

# Attach a package with the dhcpcd entity implementation.
find_package (rump REQUIRED COMPONENTS DHCPCD_ENTITY)
include_directories (${rump_INCLUDE})

# Attach a package with VFS entities implementation
find_package (precompiled_vfs REQUIRED)
include_directories (${precompiled_vfs_INCLUDE})

# Attach a package with TLS entity and client lib implementations
find_package (tls REQUIRED)
include_directories (${tls_INCLUDE})

find_package (uart REQUIRED)
include_directories (${uart_INCLUDE})


# Payload
add_subdirectory (node)
add_subdirectory (env)

# Binary image of KasperskyOS.
add_subdirectory (einit)
