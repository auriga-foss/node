cmake_minimum_required (VERSION 3.12)
project (node)

# Initialize CMake library for Kaspersky OS SDK.
include (platform)
initialize_platform ()

# Add functions for installing artefacts.
include (platform/install)

# Toolchain for work with NK-parser.
include (platform/nk)

# Attach a package with the virtual file system implementation.
find_package (vfs REQUIRED)
include_directories (${vfs_INCLUDE})

# Attach a package with the ramdisk implementation.
find_package (ramdisk REQUIRED)
include_directories (${ramdisk_INCLUDE})

# Connect a package that imports components
# to work with the ext2/ext3/ext4 file systems.
find_package (LWEXT4 REQUIRED)
include_directories (${LWEXT4_INCLUDE})

# Attach a package with the DNET file system implementation.
find_package (dnet REQUIRED COMPONENTS IMPLEMENTATION_LIB)

# Attach a package with TLS entity and client lib implementations
find_package (tls REQUIRED)
include_directories (${tls_INCLUDE})

# Payload
add_subdirectory (node)
add_subdirectory (env)
add_subdirectory (vfs)

# Binary image of KasperskyOS.
add_subdirectory (einit)
