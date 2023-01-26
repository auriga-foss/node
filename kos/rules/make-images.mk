# image build specific variables; shouldn't be exposed to global make-vars.mk
FDISK     = /usr/sbin/fdisk
MKFS_EXT2 = /usr/sbin/mkfs.ext2
PART_0    = $(BUILD)/partition_0.img

# prepare test files to be placed to rootfs partition 0
$(ROOTFS_DIR): $(BUILD_ROOT)/../test
	@echo "First re-create $@"
	@rm -rf $@ && mkdir -p $@/deps/v8/src
	@echo "KasperskyOS disk image creator doesn't follow symlinks, so copy" \
				" the whole 'test' folder"
	mkdir -p $@/opt/node/
	@cp -r $< $@/opt/node/
ifeq (${NODE_DEMO}, 1)
	@echo "Copy demo files"
	mkdir -p $@/demos/cms
	@cp -r $(BUILD_ROOT)/../../kasperskyos-nodejs-cms-demo/* $@/demos/cms
	mkdir -p $@/demos/filemanager
	@cp -r $(BUILD_ROOT)/../../kasperskyos-nodejs-filemanager-demo/* $@/demos/filemanager
	mkdir -p $@/demos/messenger
	@cp -r $(BUILD_ROOT)/../../kasperskyos-nodejs-messenger-demo/* $@/demos/messenger
else
	@echo "Copy files needed by tests"
	mkdir -p $@/opt/node/benchmark
	@cp -r $(BUILD_ROOT)/../benchmark/_cli.js $@/opt/node/benchmark/
	mkdir -p $@/opt/node/deps/corepack
	@cp -r $(BUILD_ROOT)/../deps/corepack/package.json \
		$@/opt/node/deps/corepack/package.json
	mkdir -p $@/opt/node/deps/npm
	@cp -r $(BUILD_ROOT)/../deps/npm/package.json \
		$@/opt/node/deps/npm/package.json
	mkdir -p $@/opt/node/tools/icu
	@cp -r $(BUILD_ROOT)/../tools/icu/icu_versions.json \
		$@/opt/node/tools/icu/icu_versions.json
	mkdir -p $@/opt/node/deps/v8/src
	@cp -r $(BUILD_ROOT)/../deps/v8/src/objects $@/opt/node/deps/v8/src/
	@cp -r $(BUILD_ROOT)/../config.gypi $@
	mkdir -p $@/opt/node/doc/api
	@cp -r $(BUILD_ROOT)/../doc/api/cli.md $@/opt/node/doc/api
endif
	@cp -r $(BUILD_ROOT)/image_builder/resources/rootfs/* $@
	@echo "Please keep /etc/hosts and /etc/resolv.conf updated with respect to" \
				" your OS config"
	@cp -r $(BUILD_ROOT)/image_builder/resources/certs $@
	$(Q)cp -r $(BUILD_ROOT)/image_builder/application/application $@

$(PART_0): $(ROOTFS_DIR)
	@echo "Preparing partition for ramdisk ..."
	@dd if=/dev/zero of=$@ bs=1M count=63
	@$(MKFS_EXT2) $@ -d $< -L "TST_PART_0" -t ext2 -b 1024

# the ramdisk image is dependant on application(s) binaries
# which are build with respect tp KOS requirements
$(RAMDISK0): $(PART_0)
	@echo "Preparing $@ image"
	@mkdir -p $(dir $@)
	@echo "Creating empty image .."
	@dd if=/dev/zero of=$@ bs=1M count=64
	@echo "Creating partition ..."
	@printf "o\nn\np\n1\n\n\nw" | $(FDISK) $@
	@dd if=$< of=$@ bs=512 seek=2048 conv=notrunc
	@echo "Image '$@' is ready"

# target to build SD card fat32 image
$(SD_CARD0): $(ROOTFS_DIR)
	@echo "Preparing $(SD_CARD0) image"
	@$(SDK_PREFIX)/common/prepare_hdd_img.sh -d $< -s 64 -f fat32 -img $(SD_CARD0)
	@echo "Image $(SD_CARD0) is ready"

# target to build image to be used on real hardware
.PHONY: realhw
realhw: image
	@echo "Preparing test HW image with node arg='$(NODE_ARG)' ..."
	$(Q)rm -f $(ROOTFS_SOURCE)/Node
	$(Q)cp ../out/Release/node  $(ROOTFS_SOURCE)/Node
	@echo "UART_OPTION: $(UART_OPTION)"
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
		-D USE_EXECMGR=$(USE_EXECMGR) \
		$(UART_OPTION) \
		../ && make kos-image
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/use_klog.psl
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/use_execmgr.psl
	$(Q)rm -f $(BUILD_ROOT)/image_builder/einit/src/psl/rootfs.psl
	@echo "Image ($(BUILD)/einit/kos-image) with node arg='$(NODE_ARG)' ready."

# the target 'image' is used for debug purposes to check disk image generation
.PHONY: image
image: $(ROOTFS_IMAGE)
	@echo "$@ Done."

# image(s) clean-up rule
.PHONY: clean-image
clean-image:
	@echo " RM 	$(RAMDISK0)"
	@rm -f $(RAMDISK0)
	@echo " RM	$(PART_0)"
	@rm -f $(PART_0)
	@echo " RM 	$(SD_CARD0)"
	@rm -f $(SD_CARD0)
	@rm -rf ${ROOTFS_SOURCE}/root
	@rm -f  ${ROOTFS_SOURCE}/Node
	@rm -f  ../application/application*
