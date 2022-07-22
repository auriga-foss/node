
TARGET_ARCH ?= arm64
NODE_PORT   ?= 0
QEMU_NET    ?= 0

DEBUG_ENABLED ?= n
GDB_SUPPORT   ?= y

QEMU_SERIAL_PORT ?= 12345

USE_TLS          ?= 0

# simple helper to enable/disable commands echoing
ifeq ($(strip $(V)),1)
Q=
else
Q=@
MAKEFLAGS += -s --no-print-directory
endif

# SET aarch64 build by default
ifeq ($(strip $(TARGET_ARCH)),arm64)
TARGET       := aarch64-kos
HOST          = x86_64-linux-gnu
SDK_VERSION  ?= 1.1.0.91
DEST_CPU      = arm64
ARCH_CFG_ARGS =
QEMU          = qemu-system-aarch64
# TODO: need to figure out proper args for aarch64 and arm32
#       qemu since at the KOS-SDK it is used as '-machine virt'
QEMU_OPTS     = -machine vexpress-a15,secure=on -cpu cortex-a72
endif

ifeq ($(strip $(TARGET_ARCH)),arm32)
TARGET       := arm-kos
HOST          = i686-linux-gnu
SDK_VERSION  ?= 0.1.0.158
DEST_CPU      = arm
ARCH_CFG_ARGS = --with-arm-float-abi=soft
QEMU          = qemu-system-arm
QEMU_OPTS     = -machine vexpress-a15,secure=on
endif

QEMU_OPTS += -m 2048
QEMU_OPTS += -serial stdio
QEMU_OPTS += -nographic
QEMU_OPTS += -monitor none

ifeq ($(strip $(QEMU_NET)),1)
QEMU_OPTS += -nic user,hostfwd=tcp:127.0.0.1:${NODE_PORT}-10.0.2.10:8080
endif

SDK_PREFIX     = /opt/KasperskyOS-Buddies-Edition-$(SDK_VERSION)
# system varibale, will be exported among the others later
PATH          :=/usr/sbin:$(SDK_PREFIX)/toolchain/bin:${PATH}
BUILD          = $(BUILD_ROOT)/image_builder/build-$(TARGET_ARCH)
ROOTFS_SOURCE  = $(BUILD)/rootfs
RAMDISK0       = $(BUILD)/ramdisk0.img
INSTALL_PREFIX = $(BUILD)/../install
PKG_CONFIG     = ""

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
              --fully-static --without-ssl -C --without-dtrace $(ARCH_CFG_ARGS)

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

# Version comparison functions from https://stackoverflow.com/a/15637871
# for checking QEMU_OPTS
S :=
# For non empty strings
str.eq = $(if $(subst $1,,$2),$S,T)
str.le = $(call str.eq,$(word 1,$(sort $1 $2)),$1)
# Creates a list of digits from a number
mklist = $(eval __tmp := $1)$(foreach i,0 1 2 3 4 5 6 7 8 9,$(eval __tmp := $$(subst $$i,$$i ,$(__tmp))))$(__tmp)
shift = $(wordlist 2, $(words $1), $1)
num.le = $(eval __tmp1 := $(call mklist,$1))$(eval __tmp2 := $(call mklist,$2))$(if $(call str.eq,$(words $(__tmp1)),$(words $(__tmp2))),$(call str.le,$1,$2),$(call str.le,$(words $(__tmp1)),$(words $(__tmp2))))
#Strip zeroes from the beginning of a list
list.strip = $(eval __flag := 1)$(foreach d,$1,$(if $(__flag),$(if $(subst 0,,$d),$(eval __flag :=)$d,$S),$d))
# temp string: 0 - two number equals, L first LT, G first GT or second is short,
gen.cmpstr = $(eval __Tmp1 := $(subst ., ,$1))$(eval __Tmp2 := $(subst ., ,$2))$(foreach i,$(__Tmp1),$(eval j := $(word 1,$(__Tmp2)))$(if $j,$(if $(call str.eq,$i,$j),0,$(if $(call num.le,$i,$j),L,G)),G)$(eval __Tmp2 := $$(call shift,$(__Tmp2))))$(if $(__Tmp2), L)
ver.lt = $(call str.eq,$(word 1,$(call list.strip,$(call gen.cmpstr,$1,$2))),L)

# Check that QEMU_OPTS contains -smp4 if needed:
$(if $(call ver.lt,1.1.0.259,${SDK_VERSION}),$(if $(call findstring,-smp4,${QEMU_OPTS}),$(info QEMU_OPTS contains -smp4, OK),$(error QEMU_OPTS does not contain -smp4 though SDK_VERSION >= 1.1.0.260)))


# ok, we're all set now. lets export build control variables to let
# subsequent make call(s) 'see' them properly

# export NodeJS config & build conrol variables
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
export RAMDISK0
export INSTALL_PREFIX
export PKG_CONFIG
export NODE_ADDONS_BUILD_PATH
export NODE_ADDONS_LIB_ENABLED

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
export USE_TLS
