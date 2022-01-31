# Here we put all supplementary make targets for running some test(s)
# in one command (lazy approach)
# This file is to be included into main Makefile

.PHONY: helloworld
helloworld:
	$(Q)$(MAKE) run NODE_ARG="test/message/hello_world.js"

.PHONY: bareminimim
bareminimim:
	$(Q)$(MAKE) run NODE_ARG="test/kos/bareminimim.js"

.PHONY: consolestderr
consolestderr:
	$(Q)$(MAKE) run NODE_ARG="test/kos/console_stderr.js"

.PHONY: pi
pi:
	$(Q)$(MAKE) run NODE_ARG="test/kos/pi_10000x25.js"

.PHONY: http
http:
	$(Q)$(MAKE) run NODE_ARG="test/kos/http.js" NODE_PORT=$(NODE_PORT) QEMU_NET=1
