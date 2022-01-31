project (vfs)

# Toolchain for work with NK-parser.
include (platform/nk)

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

nk_build_edl_files (net_vfs_edl_files
                    NK_MODULE "vfs"
                    EDL "${CMAKE_SOURCE_DIR}/resources/edl/NetVfs.edl"
                    NK_FLAGS "--extended-errors")

add_executable (NetVfs "src/net_vfs.c")
add_dependencies (NetVfs net_vfs_edl_files)
target_link_libraries (NetVfs ${vfs_SERVER_LIB}
                              ${LWEXT4_LIB}
                              ${vfs_IMPLEMENTATION_LIB}
                              ${dnet_IMPLEMENTATION_LIB})
set_target_properties (NetVfs
                       PROPERTIES ${blkdev_ENTITY}_REPLACEMENT ${ramdisk_ENTITY})
