# here we put debug-enabled build targets

.PHONY: configure-debug
configure-debug:
	$(Q)echo "Configuring project to be run with GDB ..."
	$(Q)$(MAKE) configure DEBUG_ENABLED=y


.PHONY: run-debug
run-debug: compile $(RAMDISK0)
	@rm -f $(ROOTFS_SOURCE)/Node
	cp ../out/Debug/node  $(ROOTFS_SOURCE)/Node_with_debug_info
	$(TARGET)-strip -g -o $(ROOTFS_SOURCE)/Node \
		$(ROOTFS_SOURCE)/Node_with_debug_info
	@echo \"${NODE_ARG}\" > image_builder/env/src/cmdline.txt
	@echo " Linking test (ramdisk) with KOS-qemu image ..."
	$(Q)mkdir -p $(BUILD) && cd $(BUILD)/ && \
		cmake -G "Unix Makefiles" \
		-D CMAKE_BUILD_TYPE:STRING=Debug \
		-D CMAKE_INSTALL_PREFIX:STRING=$(INSTALL_PREFIX) \
		-D CMAKE_TOOLCHAIN_FILE=$(SDK_PREFIX)/toolchain/share/toolchain-$(TARGET).cmake \
		../ && make kos-qemu-image
ifneq (${NODE_PORT}, 0)
	@echo "NodeJS TCP:8080 exposed to 127.0.0.1:${NODE_PORT}"
else
	@echo "NodeJS TCP:8080 exposed to 127.0.0.1:<random free port>"
	# /proc/<qemu_pid>/net/tcp can be used to determine real port on
	# Debian/Ubuntu
endif
	$(Q)$(SDK_PREFIX)/toolchain/bin/$(QEMU) $(QEMU_OPTS) \
		-kernel $(BUILD)/einit/kos-qemu-image


.PHONY: gdb
gdb:
	$(TARGET)-gdb \
		-ex "target remote 127.0.0.1:$(GDB_SERVER_PORT)" \
		--symbols=$(ROOTFS_SOURCE)/Node_with_debug_info \
		-ex "break main" -ex "c" -tui
