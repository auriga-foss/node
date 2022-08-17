
TARGET_ARCH  = arm64
NODE_PORT   ?= 0
QEMU_NET    ?= 0

DEBUG_ENABLED ?= n
GDB_SUPPORT   ?= y

QEMU_SERIAL_PORT ?= 12345

USE_TLS          ?= 0
USE_KLOG         ?= 0

ROOTFS_SDCARD    ?= 0

ifeq ($(strip $(ROOTFS_SDCARD)),1)
ROOTFS_IMAGE = $(SD_CARD0)
ROOTFS_PSL   = sdcard.psl
else
ROOTFS_IMAGE = $(RAMDISK0)
ROOTFS_PSL   = ramfs.psl
endif

# simple helper to enable/disable commands echoing
ifeq ($(strip $(V)),1)
Q=
else
Q=@
MAKEFLAGS += -s --no-print-directory
endif

TARGET       := aarch64-kos
HOST          = x86_64-linux-gnu
SDK_VERSION  ?= 1.1.0.438
DEST_CPU      = arm64
ARCH_CFG_ARGS =
QEMU          = qemu-system-aarch64
QEMU_OPTS     = -machine vexpress-a15,secure=on -cpu cortex-a72
QEMU_OPTS    += -smp 4
QEMU_OPTS    += -m 2048
QEMU_OPTS    += -serial stdio
QEMU_OPTS    += -nographic
QEMU_OPTS    += -monitor none

ifeq ($(strip $(QEMU_NET)),1)
QEMU_OPTS += -nic user,hostfwd=tcp:127.0.0.1:${NODE_PORT}-10.0.2.10:8080
endif

SDK_PREFIX     = /opt/KasperskyOS-Community-Edition-$(SDK_VERSION)
# system varibale, will be exported among the others later
PATH          :=/usr/sbin:$(SDK_PREFIX)/toolchain/bin:${PATH}
BUILD          = $(BUILD_ROOT)/image_builder/build-$(TARGET_ARCH)
ROOTFS_SOURCE  = $(BUILD)/rootfs
ROOTFS_DIR     = $(ROOTFS_SOURCE)/root
RAMDISK0       = $(BUILD)/ramdisk0.img
SD_CARD0       = $(BUILD)/sdcard0.img
INSTALL_PREFIX = $(BUILD)/../install

ifeq ($(strip $(ROOTFS_SDCARD)),1)
QEMU_OPTS += -drive file=$(SD_CARD0),if=sd,format=raw
endif

CC=$(TARGET)-gcc
CXX=$(TARGET)-g++

CC_HOST=$(HOST)-gcc
CCX_HOST=$(HOST)-g++

# In order to statically link addons we need to supply proper path to C++ addons
# build folder.
# By default we have it at the <nodejs root>/kos/addons/build/Release.
NODE_ADDONS_BUILD_PATH ?= $(BUILD_ROOT)/addons/build/Release
NODE_ADDONS_LIB_ENABLED ?= 0

# NodeJS configure arguments list
CONFIG_ARGS = --dest-cpu=$(DEST_CPU) --cross-compiling --dest-os=kos \
              --fully-static --without-ssl -C $(ARCH_CFG_ARGS)

# default qemu gdb port
GDB_SERVER_PORT = 1234

# test if we want to have debug-enabled build
ifeq ($(strip $(DEBUG_ENABLED)),y)
CONFIG_ARGS += --debug --debug-node --v8-lite-mode
# enable GDB support
ifeq ($(strip $(GDB_SUPPORT)),y)
# So far the '--gdb'-enabled build is not supported by v8 for aarch64 target.
# https://github.com/nodejs/node/issues/40697
# Thus we may have it only for arm32 build only.
ifeq ($(strip $(TARGET_ARCH)),arm64)
$(error The gdb-enabled build for NodeJS is not supported by V8 (--gdb option \
causes build failure)! If you really want to do debug build without GDB \
support set GDB_SUPPORT variable to 'n')
endif
CONFIG_ARGS += --gdb
endif
# redirecting serial device
QEMU_OPTS += -serial tcp::$(QEMU_SERIAL_PORT),server,nowait
# suspend gues execution (??) untill it would be explicitly run from gdb
QEMU_OPTS += -S
# enabling qemu listening incoming gdb connection
QEMU_OPTS += -gdb tcp::$(GDB_SERVER_PORT)
endif


# ok, we're all set now. lets export build control variables to let
# subsequent make call(s) 'see' them properly

# export NodeJS config & build control variables
export CONFIG_ARGS
export CC
export CCX
export CC_HOST
export CCX_HOST

export SDK_PREFIX
export PATH
export BUILD_ROOT
export BUILD
export ROOTFS_SOURCE
export ROOTFS_DIR
export RAMDISK0
export SD_CARD0
export ROOTFS_IMAGE
export INSTALL_PREFIX
export NODE_ADDONS_BUILD_PATH
export NODE_ADDONS_LIB_ENABLED
export USE_TLS
export USE_KLOG
export ROOTFS_SDCARD

# export qemu control variabled
export QEMU_OPTS
export QEMU
export TARGET
export TARGET_ARCH
export NODE_PORT
export QEMU_NET
export GDB_SERVER_PORT
export LANG=C
export GDB_SUPPORT
export DEBUG_ENABLED
export QEMU_SERIAL_PORT
