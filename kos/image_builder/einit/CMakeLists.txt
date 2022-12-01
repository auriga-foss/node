project (einit)

# Toolchain for work with NK-parser.
include (platform/image)

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

# Add target for klog's edl translation.
nk_build_edl_files (klog_edl_files NK_MODULE "klog" EDL
                    "${CMAKE_SOURCE_DIR}/resources/edl/KlogEntity.edl")
# HACK: make edl depends on storage because they are needed much earlier
# (i.e on KSS build stage) and we do not want to depend on KOS internal targets.
add_dependencies (KlogStorageEntity klog_edl_files)

# Make Klog targets by platforms, Klog hardly connected with KSS.
# Klog built and included in entites list as KSS, by image build scripts,
# and that is the reason why klog src must be in einit/src.
add_executable (KlogEntityQemu "src/klog_entity.c")
add_executable (KlogEntityHw "src/klog_entity.c")

add_dependencies (KlogEntityQemu klog_edl_files)
add_dependencies (KlogEntityHw klog_edl_files)

target_link_libraries (KlogEntityQemu ${klog_SYSTEM_AUDIT_LIB})
target_link_libraries (KlogEntityHw ${klog_SYSTEM_AUDIT_LIB})

# Define an ENTITIES variable with a list of entities.
set (ENTITIES
        KlogStorageEntity
        Node
        ${precompiled_vfsVfsNet}
        Env
        ${tls_ENTITY}
        ${rump_DHCPCD_ENTITY}
        ${ns_ENTITY}
        ${certificate_storage_ENTITY})

set (SECURITY_PSL_FILE "src/security.psl")

if (ROOTFS_SDCARD)
    set (ROOTFS_ENTITY "kl.VfsSdCardFs")
    list (APPEND ENTITIES ${precompiled_vfsVfsSdCardFs})
    set (FS_BACKEND ${precompiled_vfsVfsSdCardFs})
else()
    set (ROOTFS_ENTITY "kl.VfsRamFs")
    list (APPEND ENTITIES ${precompiled_vfsVfsRamFs})
    set (FS_BACKEND ${precompiled_vfsVfsRamFs})
endif()

set (FILES $ENV{RAMDISK0})

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

add_custom_command (OUTPUT $ENV{RAMDISK0}
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different client.crt $ENV{ROOTFS_SOURCE}/ramdisk0/certs/
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different client.key $ENV{ROOTFS_SOURCE}/ramdisk0/certs/
                    DEPENDS rootCA.crt client.crt client.key)

set (INIT_KlogEntity_PATH "KlogEntityHw")
# Building Kaspersky OS solution image for a hardware platform.
build_kos_hw_image (kos-image
                    EINIT_ENTITY EinitHw
                    CONNECTIONS_CFG "src/init.yaml.in"
                    SECURITY_PSL ${SECURITY_PSL_FILE}
                    KLOG_ENTITY KlogEntityHw
                    IMAGE_FILES ${ENTITIES}
                                ${FILES})

set (INIT_KlogEntity_PATH "KlogEntityQemu")
# Building Kaspersky OS solution image for a QEMU platform.
build_kos_qemu_image (kos-qemu-image
                      EINIT_ENTITY EinitQemu
                      CONNECTIONS_CFG "src/init.yaml.in"
                      SECURITY_PSL ${SECURITY_PSL_FILE}
                      KLOG_ENTITY KlogEntityQemu
                      IMAGE_FILES ${ENTITIES}
                                  ${FILES})
