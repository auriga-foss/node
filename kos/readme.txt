utf-8

This folder contains a top-level Makefile for configuration, build and run of
NodeJS on KasperskyOS using qemu/arm64 and Raspberry Pi 4 board.
make dependency on configure isn't addressed thus far.
Run dependency on sources is respected, whereas dependency on rootfs (incl.
tests) isn't.

make configure       - configuration
make compile         - build
make helloworld      - build and run a trivial test using QEMU (similar to
                       make && ./node test/message/hello_world.js)
make run NODE_ARG=<> - build and run using QEMU (similar to
                       make && ./node NODE_ARG)
