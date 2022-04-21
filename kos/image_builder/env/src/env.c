
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

#define VARGS_MAX_PARAMETERS_COUNT 10
#define VARGS_MAX_PARAMETER_LEN 512

#ifdef ENABLE_UART
#define ARGV_THRU_SERIAL
#endif

#ifdef ARGV_THRU_SERIAL
static int read_argv_from_uart(const char** pp_argv,
                               uint32_t* p_finded_parameters) {
  if (pp_argv == NULL) {
    printk("pp_argv is NULL\n");
    return EXIT_FAILURE;
  }

  UartHandle uartH = 0;
  int rc = UartInit();
  if (rc != UART_OK) {
    printk("UartInit() failed\n");
    return EXIT_FAILURE;
  }

  printk("PORT_NAME: %s\n", PORT_NAME);
  rc = UartOpenPort(PORT_NAME, &uartH);
  if (rc != UART_OK) {
    printk("UartOpenPort() failed, return %d, port name is %s\n",
           rc, PORT_NAME);
    return EXIT_FAILURE;
  }

  uint32_t ch_cnt = 0;
  uint32_t par_cnt = 0;
  uint8_t temp[VARGS_MAX_PARAMETER_LEN];
  memset(temp, 0, VARGS_MAX_PARAMETER_LEN);
  int return_value = EXIT_FAILURE;

  printk("env ready\n");
  do {
    UartReadByte(uartH, &temp[ch_cnt]);
    ch_cnt++;
    if (ch_cnt > VARGS_MAX_PARAMETER_LEN) {
      printk("ch_cnt reached max value\n");
      break;
    }

    if ((temp[ch_cnt-1] == ' ') || (temp[ch_cnt-1] == '\r')) {
      printk("Got parameter %d, len: %d, value: %s\n",
              par_cnt, ch_cnt, temp);
      void* p  = malloc(ch_cnt);
      if (p == NULL) {
        printk("malloc failed\n");
        break;
      }

      memset(p, 0, ch_cnt);
      memcpy(p, temp, ch_cnt-1);
      *(pp_argv + par_cnt) = p;
      par_cnt++;
      if (par_cnt > VARGS_MAX_PARAMETERS_COUNT) {
        printk("par_cnt reached max value\n");
        break;
      }

      if(temp[ch_cnt-1] == '\r') {
        *p_finded_parameters = par_cnt;
        return_value = EXIT_SUCCESS;
        break;
      }
      memset(temp, 0, VARGS_MAX_PARAMETER_LEN);
      ch_cnt = 0;
    }
  } while (1);

  rc = UartClosePort(uartH);
  if (rc != UART_OK) {
    printk("UartClosePort() failed, return %d, port name is %s\n",
           rc, PORT_NAME);
    return_value = EXIT_FAILURE;
  }
  return return_value;
}
#endif

int main(int argc, char** argv) {
#ifdef ARGV_THRU_SERIAL
  const char* NodeArgs[VARGS_MAX_PARAMETERS_COUNT] = {0};
  uint32_t finded_parameters = 0;
  if (read_argv_from_uart(NodeArgs, &finded_parameters) == EXIT_FAILURE) {
    printk("read_argv_from_uart return EXIT_FAILURE\n");
    return EXIT_FAILURE;
  }
  int i;
  for(i = 0; i < finded_parameters; i++)
    printk("NodeArgs[%d]: %s\n", i, NodeArgs[i]);

  const char* NodeEnvs[] = {
#if (USE_TLS != 1)
    _VFS_NETWORK_BACKEND"=client:"TST_VFS_CONNECTION_ID,
#else
    _VFS_NETWORK_BACKEND"=client:"_TLS_CONNECTION_ID,
#endif
	_VFS_FILESYSTEM_BACKEND"=client:"TST_VFS_CONNECTION_ID,
  };

  envRegisterArgs("Node", (int)finded_parameters, NodeArgs);
  ENV_REGISTER_VARS("Node", NodeEnvs);
#endif

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

#ifdef ARGV_THRU_SERIAL
  for(i = 0; i < finded_parameters; i++)
    free(NodeArgs + i);
#endif
  return EXIT_SUCCESS;
}
