
#include <env/env.h>
#include <precompiled_vfs/defs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uart/uart.h>
#include <tls/defs.h>
#include <vfs/rumpfs_client.h>
#include <vfs/vfs.h>
#include <coresrv/syscalls.h>

#include <kos_net.h>

#define VARGS_MAX_PARAMETERS_COUNT 10
#define VARGS_MAX_PARAMETER_LEN 512

#ifdef ENABLE_UART
#define ARGV_THRU_SERIAL
#endif

#ifdef ARGV_THRU_SERIAL
static int read_argv_from_uart(const char** pp_argv,
                               rtl_size_t* p_found_parameters) {
  if (pp_argv == NULL) {
    printk("pp_argv is NULL\n");
    return EXIT_FAILURE;
  }

  UartError rc = UartInit();
  if (rc != UART_OK) {
    printk("UartInit() failed\n");
    return EXIT_FAILURE;
  }

  UartHandle uart_handle;
  printk("PORT_NAME: %s\n", PORT_NAME);
  rc = UartOpenPort(PORT_NAME, &uart_handle);
  if (rc != UART_OK) {
    printk("UartOpenPort() failed, return %u, port name is %s\n",
           rc, PORT_NAME);
    return EXIT_FAILURE;
  }

  UartConfig uart_config;
  uart_config.baudrate = UART_BR_115200;
  uart_config.bits = UART_8BITS;
  uart_config.stopbits = UART_1STOP_BITS;
  uart_config.parity = UART_NO_PARITY;
  uart_config.flags = UART_NO_FLOW;
  rc = UartSetConfig(uart_handle, &uart_config);
  if (rc != UART_OK) {
    printk("UartSetConfig() failed, return %u, port name is %s\n",
           rc, PORT_NAME);
    return EXIT_FAILURE;
  }

  rtl_size_t char_idx;
  rtl_size_t par_idx;
  rtl_size_t char_cnt = 0;
  rtl_size_t par_cnt = 0;
  uint8_t temp[VARGS_MAX_PARAMETER_LEN];
  memset(temp, 0, VARGS_MAX_PARAMETER_LEN);
  int return_value = EXIT_FAILURE;
  UartTimeouts uart_timeouts;
  uart_timeouts.overall = 5 * 1000;  // ms
  uart_timeouts.interval = 100;  // ms

  printk("env ready\n");
  do {
    rtl_size_t bytes_read;
    rc = UartRead(uart_handle, &temp[char_cnt], sizeof(temp) - char_cnt,
                  &uart_timeouts, &bytes_read);
    printk("%llu bytes read from UART %s.\n", bytes_read, PORT_NAME);

    if (rc == UART_OK) {
      char_cnt = char_cnt + bytes_read;
    } else {
      printk("UartRead() failed, return %u, port name is %s.\n",
             rc, PORT_NAME);
    }

    if (char_cnt > VARGS_MAX_PARAMETER_LEN) {
      printk("char_cnt exceeded max value (%u).\n", VARGS_MAX_PARAMETER_LEN);
      break;
    } else if (temp[char_cnt - 1] == '\r' || temp[char_cnt - 1] == '\n') {
      return_value = EXIT_SUCCESS;
    }
  } while (return_value != EXIT_SUCCESS && char_cnt < VARGS_MAX_PARAMETER_LEN);

  if (return_value == EXIT_SUCCESS) {
    par_idx = 0;
    return_value = EXIT_FAILURE;
    for (char_idx = 0; char_idx < char_cnt; char_idx++) {
      if (temp[char_idx] == ' ' || temp[char_idx] == '\r' ||
          temp[char_idx] == '\n') {
        if (char_idx > par_idx) {
          void* p  = malloc(char_idx - par_idx + 1);
          if (p == NULL) {
            printk("malloc failed.\n");
            break;
          }

          memset(p, 0, char_idx - par_idx + 1);
          memcpy(p, temp + par_idx, char_idx - par_idx);
          printk("Got parameter %llu, len: %llu, value: %s.\n",
                  par_cnt, char_idx - par_idx, p);

          if (par_cnt == VARGS_MAX_PARAMETERS_COUNT) {
            printk("par_cnt exceeded max value (%u).\n",
                  VARGS_MAX_PARAMETERS_COUNT);
            break;
          }
          *(pp_argv + par_cnt) = p;
          par_cnt++;
          if (temp[char_idx] == '\r' || temp[char_idx] == '\n') {
            *p_found_parameters = par_cnt;
            return_value = EXIT_SUCCESS;
            break;
          }
        }
        par_idx = char_idx + 1;
      }
    }
  }

  rc = UartClosePort(uart_handle);
  if (rc != UART_OK) {
    printk("UartClosePort() failed, return %u, port name is %s\n",
           rc, PORT_NAME);
    return_value = EXIT_FAILURE;
  }
  return return_value;
}
#endif

int main(int argc, char** argv) {
#ifdef ARGV_THRU_SERIAL
  const char* NodeArgs[VARGS_MAX_PARAMETERS_COUNT] = {0};
  rtl_size_t found_parameters = 0;
  if (read_argv_from_uart(NodeArgs, &found_parameters) == EXIT_FAILURE) {
    printk("read_argv_from_uart return EXIT_FAILURE\n");
    return EXIT_FAILURE;
  }
  rtl_size_t i;
  for (i = 0; i < found_parameters; i++)
    printk("NodeArgs[%d]: %s\n", i, NodeArgs[i]);

  const char* NodeEnvs[] = {
#if (USE_TLS != 1)
    _VFS_NETWORK_BACKEND"=client:"NET_VFS_CONNECTION,
#else
    _VFS_NETWORK_BACKEND"=client:"_TLS_CONNECTION_ID,
#endif
    _VFS_FILESYSTEM_BACKEND"=client:"RAM_FS_VFS_CONNECTION,
    "PATH=/opt/node/"
  };

  envRegisterArgs("Node", (int)found_parameters, NodeArgs);
  ENV_REGISTER_VARS("Node", NodeEnvs);
#endif

  envServerRun();

#ifdef ARGV_THRU_SERIAL
  for (i = 0; i < found_parameters; i++)
    free(NodeArgs + i);
#endif
  return EXIT_SUCCESS;
}
