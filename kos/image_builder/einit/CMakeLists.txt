project (einit)

# Toolchain for work with NK-parser.
include (platform/image)

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

# Define an ENTITIES variable with a list of entities.
set (ENTITIES
        Node
        ${precompiled_vfsVfsNet}
        Env
        ${tls_ENTITY}
        ${rump_DHCPCD_ENTITY})

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

set (FILES
        $ENV{RAMDISK0}
        $ENV{BUILD_ROOT}/image_builder/certs/server.crt
        $ENV{BUILD_ROOT}/image_builder/certs/server.key
        $ENV{BUILD_ROOT}/image_builder/certs/rootCA.crt)

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

set_target_properties (${tls_ENTITY} PROPERTIES ${vfs_ENTITY}_REPLACEMENT "")
set_target_properties (${tls_ENTITY} PROPERTIES
                       EXTRA_CONNECTIONS ${EXTRA_CONNECTIONS}
                       EXTRA_ENV "\
    VFS_FILESYSTEM_BACKEND: client{fs->net}:${ROOTFS_ENTITY}
    VFS_NETWORK_BACKEND: client:kl.VfsNet")

set_target_properties (${rump_DHCPCD_ENTITY} PROPERTIES DEPENDS_ON_ENTITY "")
set_target_properties (${rump_DHCPCD_ENTITY} PROPERTIES
                       EXTRA_CONNECTIONS ${EXTRA_CONNECTIONS}
                       EXTRA_ENV "\
    VFS_FILESYSTEM_BACKEND: client{fs->net}:${ROOTFS_ENTITY}
    VFS_NETWORK_BACKEND: client:kl.VfsNet")

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

# Building Kaspersky OS solution image for a hardware platform.
build_kos_hw_image (kos-image
                    EINIT_ENTITY EinitHw
                    CONNECTIONS_CFG "src/init.yaml.in"
                    SECURITY_PSL ${SECURITY_PSL_FILE}
                    IMAGE_FILES ${ENTITIES}
                                ${FILES})

# Building Kaspersky OS solution image for a QEMU platform.
build_kos_qemu_image (kos-qemu-image
                      EINIT_ENTITY EinitQemu
                      CONNECTIONS_CFG "src/init.yaml.in"
                      SECURITY_PSL ${SECURITY_PSL_FILE}
                      IMAGE_FILES ${ENTITIES}
                                  ${FILES})
