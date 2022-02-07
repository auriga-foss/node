
#include <env/env.h>
#include <precompiled_vfs/defs.h>
#include <stdio.h>
#include <stdlib.h>
#include <tls/defs.h>
#include <vfs/rumpfs_client.h>
#include <vfs/vfs.h>

#ifdef TST_VFS_CONNECTION_ID
#undef TST_VFS_CONNECTION_ID
#endif
#define TST_VFS_CONNECTION_ID "vfs.NetVfs"

int main(int argc, char** argv)
{
  const char* NetVfsArgs[] = {
    "-l", "nodev /tmp ramfs 0",
    "-l", "romfs /romfs romfs 0",
    "-l", "devfs /dev devfs 0"
  };
  const char* NetVfsEnvs[] = {
    "ROOTFS=ramdisk0,0 / ext4 0",
    _VFS_NETWORK_BACKEND"=server:"TST_VFS_CONNECTION_ID,
    _VFS_FILESYSTEM_BACKEND"=server:"TST_VFS_CONNECTION_ID,
  };
  ENV_REGISTER_PROGRAM_ENVIRONMENT("NetVfs", NetVfsArgs, NetVfsEnvs);

  const char* TlsArgs[] = {
    "--cacert",
    "/romfs/rootCA.crt",
    "--cert",
    "/romfs/server.crt",
    "--key",
    "/romfs/server.key",
  };
  const char* TlsEnvs[] = {
    _VFS_FILESYSTEM_BACKEND"=client:"TST_VFS_CONNECTION_ID,
    _VFS_NETWORK_BACKEND"=client:"TST_VFS_CONNECTION_ID,
  };
  ENV_REGISTER_PROGRAM_ENVIRONMENT("TlsEntity", TlsArgs, TlsEnvs);

  const char* NodeVfsArgs[] = {
    #include "cmdline.txt"
  };
  const char* NodeVfsEnvs[] = {
#if (USE_TLS != 1)
    _VFS_NETWORK_BACKEND"=client:"TST_VFS_CONNECTION_ID,
#else
    _VFS_NETWORK_BACKEND"=client:"_TLS_CONNECTION_ID,
#endif
    _VFS_FILESYSTEM_BACKEND"=client:"TST_VFS_CONNECTION_ID,
  };
  ENV_REGISTER_PROGRAM_ENVIRONMENT("Node", NodeVfsArgs, NodeVfsEnvs);

  envServerRun();

  return EXIT_SUCCESS;
}
