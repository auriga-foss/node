BUILD_ROOT = $(PWD)

include ./rules/make-vars.mk

all:
	@echo "The 'all' target is intentionally left empty."
	@echo "Please consider checking possible targets with 'make help'."
.PHONY: all

.PHONY: dump-vars
dump-vars:
	@echo "	[VAR]	BUILD_ROOT=$(BUILD_ROOT)"
	@echo "	[VAR]	BUILD=$(BUILD)"
	@echo "	[VAR]	SDK_VERSION=$(SDK_VERSION)"
	@echo "	[VAR]	ROOTFS_SOURCE=$(ROOTFS_SOURCE)"
	@echo "	[VAR]	RAMDISK0=$(RAMDISK0)"
	@echo "	[VAR]	SD_CARD0=$(SD_CARD0)"
	@echo "	[VAR]	ROOTFS_IMAGE=$(ROOTFS_IMAGE)"
	@echo "	[VAR]	PATH=$(PATH)"
	@echo "	[VAR]	TARGET=$(TARGET)"
	@echo "	[VAR]   NODE_ADDONS_LIB_ENABLED=$(NODE_ADDONS_LIB_ENABLED)"
	@echo "	[VAR]   NODE_ADDONS_BUILD_PATH=$(NODE_ADDONS_BUILD_PATH)"
	@echo "	[VAR]   QEMU_OPTS=$(QEMU_OPTS)"
	@echo "	[VAR]   USE_EXECMGR=$(USE_EXECMGR)"

# Help info with respect to build/make targets
.PHONY: help
help:
	@echo "Help:"
	@echo ""
	@echo " Makefile is used to generate and start (using qemu) image with"
	@echo " test application linked with KOS image (qemu or HW)."
	@echo " Build is controlled via the following variables:"
	@echo ""
	@echo "  QEMU_NET        - could be set to 0 or 1; enables qemu start with"
	@echo "                    simulated network card, disabled by default;"
	@echo "  NODE_PORT       - NodeJS port number to be assigned upon node"
	@echo "                    server start;"
	@echo "  ROOTFS_SDCARD   - could be set to 0 or 1; if enabled, the"
	@echo "                    VfsSdCardFs entity is used as a default"
	@echo "                    filesystem backend."
	@echo ""
	@echo " Makefile has the following (most used) targets:"
	@echo ""
	@echo "  NOTE: invoking make without any target will act like 'make all'"
	@echo "  all             - build compile and disk image for the specified"
	@echo "                    architecture;"
	@echo "  configure       - do NodeJS configure with respect to KOS"
	@echo "                    project;"
	@echo "  compile         - do compile NodeJS and its' dependencies;"
	@echo "  run NODE_ARG=<> - build and run using qemu (same as"
	@echo "                    make && ./node NODE_ARG=<>);"
	@echo "  realhw          - do build image to be flashed and started on"
	@echo "                    real HW;"
	@echo "  helloworld      - build, link and run test project image using"
	@echo "                    qemu (with selected architecture)."
	@echo "                    or to build an image for HW, add FOR_HW=1;"
	@echo "  dump-vars       - [DEBUG] dump build control variables."

.PHONY: configure
configure:
	$(Q)echo "Running configure for $(TARGET_ARCH) ..."
	$(Q)CC=$(CC) CXX=$(CXX) CC_host=$(CC_HOST) CXX_host=$(CCX_HOST) \
	../configure $(CONFIG_ARGS)
	$(Q)echo "Configured to be run with TARGET_ARCH=$(TARGET_ARCH)"

.PHONY: compile
compile:
	$(Q)CC=$(CC) CXX=$(CXX) CC_host=$(CC_HOST) CXX_host=$(CCX_HOST) $(MAKE) -C ../

.PHONY: run
run: clean-arch ${ROOTFS_IMAGE}
	$(Q)$(MAKE) qemubuild
	$(Q)$(MAKE) qemurun

.PHONY: qemubuild
qemubuild: ${ROOTFS_IMAGE}
	@echo "Running QEMU ($(QEMU)) with node arg='${NODE_ARG}'"
	$(Q)rm -f $(ROOTFS_SOURCE)/Node
	$(Q)cp ../out/Release/node  $(ROOTFS_SOURCE)/Node
	@echo "UART_OPTION: $(UART_OPTION)"
	@echo " Linking test (ramdisk) with KOS-qemu image ..."
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/use_klog.psl
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/use_execmgr.psl
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/rootfs.psl
	$(Q)ln -s $(BUILD_ROOT)/image_builder/einit/src/psl/use_klog_$(USE_KLOG).psl \
		$(BUILD_ROOT)/image_builder/einit/src/psl/use_klog.psl
	$(Q)ln -s $(BUILD_ROOT)/image_builder/einit/src/psl/use_execmgr_$(USE_EXECMGR).psl \
		$(BUILD_ROOT)/image_builder/einit/src/psl/use_execmgr.psl
	$(Q)ln -s $(BUILD_ROOT)/image_builder/einit/src/psl/$(ROOTFS_PSL) \
		$(BUILD_ROOT)/image_builder/einit/src/psl/rootfs.psl
	$(Q)mkdir -p $(BUILD) && cd $(BUILD)/ && \
		cmake -G "Unix Makefiles" \
		-D CMAKE_BUILD_TYPE:STRING=Debug \
		-D CMAKE_INSTALL_PREFIX:STRING=$(INSTALL_PREFIX) \
		-D CMAKE_TOOLCHAIN_FILE=$(SDK_PREFIX)/toolchain/share/toolchain-$(TARGET).cmake \
		-D USE_TLS=$(USE_TLS) \
		-D USE_KLOG=$(USE_KLOG) \
		-D ADD_TEST=$(ADD_TEST) \
		-D USE_EXECMGR=$(USE_EXECMGR) \
		-D ROOTFS_SDCARD=$(ROOTFS_SDCARD) \
		$(UART_OPTION) \
		../ && make kos-qemu-image
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/use_klog.psl
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/use_execmgr.psl
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/rootfs.psl
ifeq  ($(QEMU_NET),1)
ifneq (${NODE_PORT},0)
	@echo "NodeJS TCP:8080 exposed to 127.0.0.1:${NODE_PORT}"
else
	@echo "NodeJS TCP:8080 exposed to 127.0.0.1:<random free port>"
	# /proc/<qemu_pid>/net/tcp can be used to determine real port on
	# Debian/Ubuntu
endif
endif

.PHONY: qemurun
qemurun:
	$(Q)$(SDK_PREFIX)/toolchain/bin/$(QEMU) $(QEMU_OPTS) \
		-kernel $(BUILD)/einit/kos-qemu-image

clean-arch:
	@echo " RM	arch-build directory for $(TARGET_ARCH)."
	$(Q)rm -rf $(BUILD)

clean-node:
	@echo " RM	NodeJS 'out' directory."
	$(Q)rm -rf ../out;

clean-tmp:
	$(Q)echo " RM	build articafts at /tmp"
	$(Q)rm -f /tmp/psl-base-c.*

# obvious ...
clean-all: clean-arch clean-node clean-tmp
	$(Q)$(MAKE) clean -C ../


# include rules for building ramfs & real HW image(s)
include ./rules/make-images.mk

# include some test targets
include ./rules/make-run-tests.mk

# include debug-enabled build specific targets
include ./rules/make-debug.mk
