utf-8

'rootfs' folder contains root filesystem from the previous run being
overwritten from within scripts. Files might be added manually, w/o any
guarantee to be preserved across runs though.
'Node' executable is being added twice (considered a BUG to be fixed).
'cmdline' file is used to pass NodeJS arguments since KasperskyOS doesn't make
use of 'argv'.

make image-clean -- removes everything (see details in 'Makefile') from the
'rootfs' folder as well as 'ramdisk0' image.
