# image build specific variables; shouldn't be exposed to global make-vars.mk
FDISK     = /usr/sbin/fdisk
MKFS_EXT2 = /usr/sbin/mkfs.ext2
PART_0    = $(BUILD)/partition_0.img
ROOTFS_DIR ?= $(ROOTFS_SOURCE)/ramdisk0

# prepare test files to be placed to ramfs partition 0
$(ROOTFS_DIR): $(BUILD_ROOT)/../test
	@echo "First re-create $@"
	@rm -rf $@ && mkdir -p $@
	@echo "KasperskyOS disk image creator doesn't follow symlinks, so copy" \
				" the whole 'test' folder"
	@cp -r $< $@
	# copy files needed by tests
	mkdir -p $@/benchmark
	@cp -r $(BUILD_ROOT)/../benchmark/_cli.js $@/benchmark/
	mkdir -p $@/deps/corepack
	@cp -r $(BUILD_ROOT)/../deps/corepack/package.json \
		$@/deps/corepack/package.json
	mkdir -p $@/deps/npm
	@cp -r $(BUILD_ROOT)/../deps/npm/package.json $@/deps/npm/package.json
	mkdir -p $@/tools/icu
	@cp -r $(BUILD_ROOT)/../tools/icu/icu_versions.json \
		$@/tools/icu/icu_versions.json
	@cp -r $(BUILD_ROOT)/../deps/v8/src/objects $@/deps/v8/src/
	@cp -r $(BUILD_ROOT)/../config.gypi $@
	mkdir -p $@/doc/api
	@cp -r $(BUILD_ROOT)/../doc/api/cli.md $@/doc/api
	@mkdir -p $@/etc
	@cp /etc/hosts $@/etc
	@cp /etc/resolv.conf $@/etc
	@echo "Please keep /etc/hosts and /etc/resolv.conf updated with respect to" \
				" your OS config"

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

# target to build image to be used on real hardware
.PHONY: realhw
realhw:
	@echo " Preparing test HW image ..."
	$(Q)mkdir -p $(BUILD) && cd $(BUILD)/ && \
		cmake -G "Unix Makefiles" \
		-D CMAKE_BUILD_TYPE:STRING=Debug \
		-D CMAKE_INSTALL_PREFIX:STRING=$(INSTALL_PREFIX) \
		-D CMAKE_TOOLCHAIN_FILE=$(SDK_PREFIX)/toolchain/share/toolchain-$(TARGET).cmake \
		../ && make kos-image
	@echo " Image ($(BUILD)/einit/kos-image) ready."


# the target 'image' is used for debug purposes to check disk image generation
.PHONY: image
image: $(RAMDISK0)
	@echo "$@ Done."


# image(s) clean-up rule
.PHONY: clean-image
clean-image:
	@echo " RM 	$(RAMDISK0)"
	@rm $(RAMDISK0)
	@echo " RM	$(PART_0)"
	@rm $(PART_0)
	@rm -rf ${ROOTFS_SOURCE}/ramdisk0
	@rm -rf ./rootfs/ramdisk0
	@rm -f  ./rootfs/Node
