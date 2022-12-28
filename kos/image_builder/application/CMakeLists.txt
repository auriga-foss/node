project (application)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

nk_build_edl_files (application_edl_files
                    NK_MODULE "application"
                    EDL "${CMAKE_SOURCE_DIR}/resources/edl/Application.edl")

add_executable (application IMPORTED GLOBAL)

set_property(TARGET application PROPERTY IMPORTED_LOCATION $ENV{ROOTFS_SOURCE}/application)
add_dependencies (application application_edl_files)

