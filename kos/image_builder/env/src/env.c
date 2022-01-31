#include <env/env.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
  const char* NetVfsArgs[] = {
    "-l", "nodev /tmp ramfs 0",
    "-l", "romfs /romfs romfs 0",
    "-l", "devfs /dev devfs 0"
  };
  const char* NetVfsEnvs[] = {
    "ROOTFS=ramdisk0,0 / ext4 0",
    "_VFS_NETWORK_BACKEND=server:VFS1",
    "_VFS_FILESYSTEM_BACKEND=server:VFS1"
  };
  ENV_REGISTER_PROGRAM_ENVIRONMENT("NetVfs", NetVfsArgs, NetVfsEnvs);

  const char* NodeVfsArgs[] = {
    #include "cmdline.txt"
  };
  const char* NodeVfsEnvs[] = {
    "_VFS_NETWORK_BACKEND=client:VFS1",
    "_VFS_FILESYSTEM_BACKEND=client:VFS1"
  };
  ENV_REGISTER_PROGRAM_ENVIRONMENT("Node", NodeVfsArgs, NodeVfsEnvs);

  envServerRun();

  return EXIT_SUCCESS;
}
