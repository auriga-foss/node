project (node)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

nk_build_edl_files (node_edl_files
                    NK_MODULE "node"
                    EDL "${CMAKE_SOURCE_DIR}/resources/edl/Node.edl")

add_executable (Node IMPORTED GLOBAL)
set_property (TARGET Node PROPERTY IMPORTED_LOCATION $ENV{ROOTFS_SOURCE}/Node)
add_dependencies (Node node_edl_files)
target_link_libraries (Node)
