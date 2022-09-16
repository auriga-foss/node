nk_build_edl_files (klog_storage_edl_files
                    NK_MODULE "klog_storage"
                    EDL "${CMAKE_SOURCE_DIR}/resources/edl/KlogStorageEntity.edl")

add_executable (KlogStorageEntity "src/klog_storage_entity.c")
target_link_libraries (KlogStorageEntity ${klog_storage_SERVER_LIB})
add_dependencies (KlogStorageEntity klog_storage_edl_files)
