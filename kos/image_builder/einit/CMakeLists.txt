project (einit)

# Toolchain for work with NK-parser.
include (platform/image)

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

# Define an ENTITIES variable with a list of entities.
set (ENTITIES Node Env NetVfs ${tls_ENTITY})

set (SECURITY_PSL_FILE "src/security.psl")

set (FILES
     $ENV{RAMDISK0}
     $ENV{BUILD_ROOT}/image_builder/certs/server.crt
     $ENV{BUILD_ROOT}/image_builder/certs/server.key
     $ENV{BUILD_ROOT}/image_builder/certs/rootCA.crt)

set (EXTRA_CONNECTIONS "\
    - target: env.Env
      id: {var: ENV_SERVICE_NAME, include: env/env.h}
    - target: vfs.NetVfs
      id: vfs.NetVfs")

set_target_properties (${tls_ENTITY} PROPERTIES ${vfs_ENTITY}_REPLACEMENT "")
set_target_properties (${tls_ENTITY}
                       PROPERTIES EXTRA_CONNECTIONS ${EXTRA_CONNECTIONS})


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
