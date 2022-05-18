/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "uv.h"
#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "kos-trace.h"

extern char **environ;

static void uv__chld(uv_signal_t* handle, int signum) {
  uv_process_t* process;
  uv_loop_t* loop;
  int exit_status;
  int term_signal;
  int status;
  pid_t pid;
  QUEUE pending;
  QUEUE* q;
  QUEUE* h;

  assert(signum == SIGCHLD);

  QUEUE_INIT(&pending);
  loop = handle->loop;

  h = &loop->process_handles;
  q = QUEUE_HEAD(h);
  while (q != h) {
    process = QUEUE_DATA(q, uv_process_t, queue);
    q = QUEUE_NEXT(q);

    do
      pid = waitpid(process->pid, &status, WNOHANG);
    while (pid == -1 && errno == EINTR);

    if (pid == 0)
      continue;

    if (pid == -1) {
      if (errno != ECHILD)
        abort();
      continue;
    }

    process->status = status;
    QUEUE_REMOVE(&process->queue);
    QUEUE_INSERT_TAIL(&pending, &process->queue);
  }

  h = &pending;
  q = QUEUE_HEAD(h);
  while (q != h) {
    process = QUEUE_DATA(q, uv_process_t, queue);
    q = QUEUE_NEXT(q);

    QUEUE_REMOVE(&process->queue);
    QUEUE_INIT(&process->queue);
    uv__handle_stop(process);

    if (process->exit_cb == NULL)
      continue;

    exit_status = 0;
    if (WIFEXITED(process->status))
      exit_status = WEXITSTATUS(process->status);

    term_signal = 0;
    if (WIFSIGNALED(process->status))
      term_signal = WTERMSIG(process->status);

    process->exit_cb(process, exit_status, term_signal);
  }
  assert(QUEUE_EMPTY(&pending));
}

/*
 * Used for initializing stdio streams like options.stdin_stream. Returns
 * zero on success. See also the cleanup section in uv_spawn().
 */
static int uv__process_init_stdio(uv_stdio_container_t* container, int fds[2]) {
  int mask;
  int fd;

  mask = UV_IGNORE | UV_CREATE_PIPE | UV_INHERIT_FD | UV_INHERIT_STREAM;

  switch (container->flags & mask) {
  case UV_IGNORE:
    return 0;

  case UV_CREATE_PIPE:
    assert(container->data.stream != NULL);
    if (container->data.stream->type != UV_NAMED_PIPE)
      return UV_EINVAL;
    else
      return uv_socketpair(SOCK_STREAM, 0, fds, 0, 0);

  case UV_INHERIT_FD:
  case UV_INHERIT_STREAM:
    if (container->flags & UV_INHERIT_FD)
      fd = container->data.fd;
    else
      fd = uv__stream_fd(container->data.stream);

    if (fd == -1)
      return UV_EINVAL;

    fds[1] = fd;
    return 0;

  default:
    assert(0 && "Unexpected flags");
    return UV_EINVAL;
  }
}


static int uv__process_open_stream(uv_stdio_container_t* container,
                                   int pipefds[2]) {
  int flags;
  int err;

  if (!(container->flags & UV_CREATE_PIPE) || pipefds[0] < 0)
    return 0;

  err = uv__close(pipefds[1]);
  if (err != 0)
    abort();

  pipefds[1] = -1;
  uv__nonblock(pipefds[0], 1);

  flags = 0;
  if (container->flags & UV_WRITABLE_PIPE)
    flags |= UV_HANDLE_READABLE;
  if (container->flags & UV_READABLE_PIPE)
    flags |= UV_HANDLE_WRITABLE;

  return uv__stream_open(container->data.stream, pipefds[0], flags);
}


static void uv__process_close_stream(uv_stdio_container_t* container) {
  if (!(container->flags & UV_CREATE_PIPE)) return;
  uv__stream_close(container->data.stream);
}


static void uv__write_int(int fd, int val) {
  ssize_t n;

  do
    n = write(fd, &val, sizeof(val));
  while (n == -1 && errno == EINTR);

  if (n == -1 && errno == EPIPE)
    return; /* parent process has quit */

  assert(n == sizeof(val));
}


static void uv__write_errno(int error_fd) {
  uv__write_int(error_fd, UV__ERR(errno));
  _exit(127);
}

int uv_spawn(uv_loop_t* loop,
             uv_process_t* process,
             const uv_process_options_t* options) {
  KOS_TRACE_INF("!!! KOS DEBUG !!! [attempt to SPAWN (fork)]");
  /* fork is marked __WATCHOS_PROHIBITED __TVOS_PROHIBITED. */
  return UV_ENOSYS;
}


int uv_process_kill(uv_process_t* process, int signum) {
  return uv_kill(process->pid, signum);
}


int uv_kill(int pid, int signum) {
  if (kill(pid, signum))
    return UV__ERR(errno);
  else
    return 0;
}


void uv__process_close(uv_process_t* handle) {
  QUEUE_REMOVE(&handle->queue);
  uv__handle_stop(handle);
  if (QUEUE_EMPTY(&handle->loop->process_handles))
    uv_signal_stop(&handle->loop->child_watcher);
}
