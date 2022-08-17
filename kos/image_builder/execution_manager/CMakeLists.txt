project (execution_manager)

include (platform/nk)
include (execution_manager/create_execution_manager_entity)

find_package (entity_launcher_client REQUIRED)

create_execution_manager_entity(
    ENTITY ExecMgrEntity
    NK_MODULE_NAME "execution_manager"
    MAIN_CONN_NAME "ExecMgrEntity"
    ROOT_PATH "/root"
    ROMFS_PATH "/mnt/romfs"
    RAMFS_PATH "/mnt/ramfs"
    VFS_CLIENT_LIB ${vfs_CLIENT_LIB})
