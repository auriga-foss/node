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
