project (einit)

# Toolchain for work with NK-parser.
include (platform/image)

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

if (USE_KLOG)
	# Make Klog targets by platforms, Klog hardly connected with KSS.
	# Klog built and included in entites list as KSS, by image build scripts,
	# and that is the reason why klog src must be in einit/src.
	add_executable (KlogEntityQemu "src/klog_entity.c")
	add_executable (KlogEntityHw "src/klog_entity.c")

	add_dependencies (KlogEntityQemu klog_edl_files)
	add_dependencies (KlogEntityHw klog_edl_files)

	target_link_libraries (KlogEntityQemu ${klog_SYSTEM_AUDIT_LIB})
	target_link_libraries (KlogEntityHw ${klog_SYSTEM_AUDIT_LIB})
endif()

# Define an ENTITIES variable with a list of entities.
set (ENTITIES
        Node
        ${precompiled_vfsVfsNet}
        Env
        ${tls_ENTITY}
        ${rump_DHCPCD_ENTITY}
        ${ns_ENTITY}
        ${certificate_storage_ENTITY})

if (USE_KLOG)
    list (APPEND ENTITIES KlogStorageEntity)
endif()

if (USE_EXECMGR)
    list (APPEND ENTITIES ExecMgrEntity ${blob_container_ENTITY} ${romfs_storage_ENTITY})
endif()

set (SECURITY_PSL_FILE "src/psl/security.psl")

if (ROOTFS_SDCARD)
    set (ROOTFS_ENTITY "kl.VfsSdCardFs")
    list (APPEND ENTITIES ${precompiled_vfsVfsSdCardFs})
    set (FS_BACKEND ${precompiled_vfsVfsSdCardFs})
else()
    set (ROOTFS_ENTITY "kl.VfsRamFs")
    list (APPEND ENTITIES ${precompiled_vfsVfsRamFs})
    set (FS_BACKEND ${precompiled_vfsVfsRamFs})
    set (FILES $ENV{ROOTFS_IMAGE})
endif()

if (USE_TLS)
    set (NODE_VFS_NETWORK_BACKEND "client:kl.TlsEntity")
else()
    set (NODE_VFS_NETWORK_BACKEND "client:kl.VfsNet")
endif()

if (ADD_TEST)
    list (APPEND FILES $ENV{BUILD_ROOT}/image_builder/application/application)
endif()

if (NOT "$ENV{NODE_ARG}" STREQUAL "")
    set (PROG_ARGS "args:")
    list (APPEND PROG_PARAMS $ENV{NODE_ARG})
    foreach (ARG IN LISTS PROG_PARAMS)
      __append_string_on_new_line(PROG_ARGS "    - ${ARG}")
    endforeach ()
    set (NODE_ARGS ${PROG_ARGS})
endif()

set (EXTRA_CONNECTIONS "\
    - target: kl.VfsNet
      id: kl.VfsNet
    - target: ${ROOTFS_ENTITY}
      id: ${ROOTFS_ENTITY}")

set_target_properties (${tls_ENTITY} PROPERTIES
    ${vfs_ENTITY}_REPLACEMENT ""
    EXTRA_CONNECTIONS ${EXTRA_CONNECTIONS}
    EXTRA_ARGS "\
    - --cacert
    - /certs/rootCA.crt
    - --cert
    - /certs/server.crt
    - --key
    - /certs/server.key"
    EXTRA_ENV "\
    VFS_FILESYSTEM_BACKEND: client:${ROOTFS_ENTITY}
    VFS_NETWORK_BACKEND: client:kl.VfsNet")

set_target_properties (${rump_DHCPCD_ENTITY} PROPERTIES
    DEPENDS_ON_ENTITY ""
    EXTRA_CONNECTIONS ${EXTRA_CONNECTIONS}
    EXTRA_ENV "\
    VFS_FILESYSTEM_BACKEND: client{fs->net}:${ROOTFS_ENTITY}
    VFS_NETWORK_BACKEND: client:kl.VfsNet"
    EXTRA_ARGS "\
    - '-4'
    - -f
    - /etc/dhcpcd.conf")

set_target_properties (${certificate_storage_ENTITY} PROPERTIES
    ${vfs_ENTITY}_REPLACEMENT ""
    EXTRA_CONNECTIONS "\
  - target: kl.TlsEntity
    id: kl.TlsEntity
  - target: ${ROOTFS_ENTITY}
    id: ${ROOTFS_ENTITY}"
    EXTRA_ENV "\
    VFS_FILESYSTEM_BACKEND: client:${ROOTFS_ENTITY}")

set_target_properties (${precompiled_vfsVfsNet} PROPERTIES
    EXTRA_ARGS "\
    - -l
    - devfs /dev devfs 0
    - -l
    - romfs /etc romfs 0")

# add hosts file to romfs image, to be used by application
list (APPEND FILES $ENV{BUILD_ROOT}/image_builder/resources/rootfs/etc/hosts)

set_target_properties (${FS_BACKEND} PROPERTIES
    EXTRA_ARGS "\
    - -l
    - nodev /tmp ramfs 0
    - -l
    - romfs /romfs romfs 0
    - -l
    - nodev /var ramfs 0
    - -l
    - devfs /dev devfs 0")

# Create Root CA key
add_custom_command (OUTPUT rootCA.key
                    COMMAND openssl genrsa -out rootCA.key 2048)

# Create and self sign the Root Certificate
add_custom_command (OUTPUT rootCA.crt
                    COMMAND openssl req -x509 -new -nodes -key rootCA.key -sha256 -days 1024 -out rootCA.crt -subj '/CN=localhost'
                    DEPENDS rootCA.key)

# Create the client certificate key
add_custom_command (OUTPUT client.key
                    COMMAND openssl genrsa -out client.key 2048)

# Create the certificate signing request (csr)
add_custom_command (OUTPUT client.csr
                    COMMAND openssl req -new -sha256 -key client.key -subj '/C=US/ST=CA/O=MyOrg, Inc./CN=localhost' -out client.csr
                    DEPENDS client.key)

# Generate the certificate
add_custom_command (OUTPUT client.crt
                    COMMAND openssl x509 -req -in client.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -days 500 -sha256 -out client.crt
                    DEPENDS client.csr rootCA.crt rootCA.key)

add_custom_command (OUTPUT $ENV{ROOTFS_IMAGE}
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different client.crt $ENV{ROOTFS_DIR}/certs/
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different client.key $ENV{ROOTFS_DIR}/certs/
                    DEPENDS rootCA.crt client.crt client.key)

set (INIT_KlogEntity_PATH "")

if (USE_KLOG)
    set (KLOG_YAML "- name: klog.KlogEntity")
    __append_string_on_new_line (KLOG_YAML "  path: KlogEntityHw")
    __append_string_on_new_line (KLOG_YAML "  connections:")
    __append_string_on_new_line (KLOG_YAML "    - target: klog_storage.KlogStorageEntity")
    __append_string_on_new_line (KLOG_YAML "      id: {var: KLOG_STORAGE_SERVER_CONNECTION_ID, include: klog_storage/client.h}")

    set (KLOG_STORAGE_YAML "- name: klog_storage.KlogStorageEntity")

    set (INIT_KLOG_ENTITY ${KLOG_YAML})
    set (INIT_KLOG_STORAGE_ENTITY ${KLOG_STORAGE_YAML})
    set (INIT_KlogEntity_PATH KlogEntityHw)
endif()

if (USE_EXECMGR)
    set (EXECMGR_YAML "- name: execution_manager.ExecMgrEntity")
    __append_string_on_new_line (EXECMGR_YAML "  path: ExecMgrEntity")
    __append_string_on_new_line (EXECMGR_YAML "  env:")
    __append_string_on_new_line (EXECMGR_YAML "    VFS_FILESYSTEM_BACKEND: client:task:${ROOTFS_ENTITY}")
    __append_string_on_new_line (EXECMGR_YAML "    LOG_LEVEL: LOG_DEBUG")
    __append_string_on_new_line (EXECMGR_YAML "  connections:")
    __append_string_on_new_line (EXECMGR_YAML "  - target: ${ROOTFS_ENTITY}")
    __append_string_on_new_line (EXECMGR_YAML "    id: ${ROOTFS_ENTITY}")

    set (NODE_EXEC_MGR_CONNECTION "  - target: execution_manager.ExecMgrEntity")
    __append_string_on_new_line (NODE_EXEC_MGR_CONNECTION "    id: ExecMgrEntity")
    set (INIT_NODE_EXEC_MGR_CONNECTION ${NODE_EXEC_MGR_CONNECTION})

    set (INIT_EXECMGR_ENTITY ${EXECMGR_YAML})

    set_target_properties (ExecMgrEntity PROPERTIES
        ${vfs_ENTITY}_REPLACEMENT "")

    set_target_properties (${blob_container_ENTITY} PROPERTIES
    ${vfs_ENTITY}_REPLACEMENT ""
    EXTRA_CONNECTIONS "\
  - target: ${ROOTFS_ENTITY}
    id: ${ROOTFS_ENTITY}"
        EXTRA_ENV "\
    VFS_FILESYSTEM_BACKEND: client:task:${ROOTFS_ENTITY}")
endif()

# Building Kaspersky OS solution image for a hardware platform.
build_kos_hw_image (kos-image
                    EINIT_ENTITY EinitHw
                    CONNECTIONS_CFG "src/init.yaml.in"
                    SECURITY_PSL ${SECURITY_PSL_FILE}
                    KLOG_ENTITY ${INIT_KlogEntity_PATH}
                    IMAGE_FILES ${ENTITIES}
                                ${FILES})

set (INIT_KlogEntity_PATH "")

if (USE_KLOG)
    set (KLOG_YAML "- name: klog.KlogEntity")
    __append_string_on_new_line (KLOG_YAML "  path: KlogEntityQemu")
    __append_string_on_new_line (KLOG_YAML "  connections:")
    __append_string_on_new_line (KLOG_YAML "    - target: klog_storage.KlogStorageEntity")
    __append_string_on_new_line (KLOG_YAML "      id: {var: KLOG_STORAGE_SERVER_CONNECTION_ID, include: klog_storage/client.h}")

    set (KLOG_STORAGE_YAML "- name: klog_storage.KlogStorageEntity")

    set (INIT_KLOG_ENTITY ${KLOG_YAML})
    set (INIT_KLOG_STORAGE_ENTITY ${KLOG_STORAGE_YAML})
    set (INIT_KlogEntity_PATH KlogEntityQemu)
endif()

# Building Kaspersky OS solution image for a QEMU platform.
build_kos_qemu_image (kos-qemu-image
                      EINIT_ENTITY EinitQemu
                      CONNECTIONS_CFG "src/init.yaml.in"
                      SECURITY_PSL ${SECURITY_PSL_FILE}
                      KLOG_ENTITY ${INIT_KlogEntity_PATH}
                      IMAGE_FILES ${ENTITIES}
                                  ${FILES})
