cmake_minimum_required (VERSION 3.12)
project (node)

# Initialize CMake library for Kaspersky OS SDK.
include (platform)
initialize_platform ()

# Add functions for installing artefacts.
include (platform/install)

# Toolchain for work with NK-parser.
include (platform/nk)

# Include functions for file storages.
include (platform/storage)

if (USE_KLOG)
    # Add package importing components for building audit entity and connecting
    # to it.
    find_package (klog REQUIRED)
    include_directories (${klog_INCLUDE})

    # Add package importing `klog_storage_entity`.
    find_package (klog_storage REQUIRED)
    include_directories (${klog_storage_INCLUDE})
endif()

# Attach a package with the dhcpcd entity implementation.
find_package (rump REQUIRED COMPONENTS DHCPCD_ENTITY)
include_directories (${rump_INCLUDE})

# Import package with romfs_storage entity.
find_package (romfs_storage REQUIRED)
initialize_storage_generator ()

# Attach a entity_launcher_client for blob_container
find_package (entity_launcher_client REQUIRED)

# Attach a blob_container for Execution Manager
find_package (blob_container REQUIRED)

# Attach a Execution Manager package
find_package (execution_manager REQUIRED)
include_directories (${execution_manager_INCLUDE})

# Execution Manager entity
add_subdirectory (execution_manager)

# Attach a package with VFS entities implementation
find_package (precompiled_vfs REQUIRED)
include_directories (${precompiled_vfs_INCLUDE})

# Attach a package with TLS entity and client lib implementations
find_package (tls REQUIRED)
include_directories (${tls_INCLUDE})

# Attach a package with UART entity for QEMU and hw + lib implementations
find_package (uart REQUIRED)
include_directories (${uart_INCLUDE})

# Attach a Name server lib implementations
find_package (ns REQUIRED)

# Attach a package with certificate manager libs implementations
find_package (cert_mgr REQUIRED)
include_directories (${cert_mgr_INCLUDE})

# Attach a package with certificate storage entity and libs implementations
find_package (certificate_storage REQUIRED)
include_directories (${certificate_storage_INCLUDE})

# Payload
add_subdirectory (node)
add_subdirectory (env)

if (USE_KLOG)
	add_subdirectory (klog_storage)
endif()

# Test entity for spawn() in NodeJS
add_subdirectory (application)

# Binary image of KasperskyOS.
add_subdirectory (einit)
