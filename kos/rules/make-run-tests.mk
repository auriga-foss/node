# Here we put all supplementary make targets for running some test(s)
# in one command (lazy approach)
# This file is to be included into main Makefile

override ACT = run

ifeq ($(strip $(FOR_HW)),1)
override ACT = clean-arch realhw
endif

.PHONY: helloworld
helloworld:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/message/hello_world.js\"'

.PHONY: bareminimim
bareminimim:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/kos/bareminimim.js\"'

.PHONY: consolestderr
consolestderr:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/kos/console_stderr.js\"'

.PHONY: pi
pi:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/kos/pi_10000x25.js\"'

.PHONY: sysinfo
sysinfo:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/kos/sysinfo.js\"'

.PHONY: http
http:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/kos/http.js\"' \
		NODE_PORT=$(NODE_PORT) QEMU_NET=1

.PHONY: crypto-classes
crypto-classes:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/parallel/test-crypto-classes.js\"'

.PHONY: crypto-hash
crypto-hash:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/parallel/test-crypto-hash.js\"'

.PHONY: build-test-addon
build-test-addon:
	$(Q)CC=$(CC) CXX=$(CXX) node-gyp --arch aarch64 --fully-static configure build --directory=./addons --nodedir $(PWD)/../ 
	mv ./addons/build/Release/test_addon.a ./addons/build/Release/libtest_addon.a

.PHONY: test-addon
test-addon:
	$(Q)$(MAKE) $(ACT) NODE_ARG='\"/opt/node/test/kos/test_addon.js\"'

.PHONY: tests
tests: clean-arch
	python ./test.py $(options)
