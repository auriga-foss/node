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
#include "spinlock.h"

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

static int orig_termios_fd = -1;
static struct termios orig_termios;
static uv_spinlock_t termios_spinlock = UV_SPINLOCK_INITIALIZER;

static int uv__tty_is_slave(const int fd) {
  int result;
  /* KOS: TODO: consider implementing ioctl() TIOCGPTN, or ptsname(). */
  result = 0;
  return result;
}

int uv_tty_init(uv_loop_t* loop, uv_tty_t* tty, int fd, int unused) {
  uv_handle_type type;
  int flags;
  int newfd;
  int r;
  int saved_flags;
  int mode;
  char path[256];
  (void)unused; /* deprecated parameter is no longer needed */

  /* File descriptors that refer to files cannot be monitored with epoll.
   * That restriction also applies to character devices like /dev/random
   * (but obviously not /dev/tty.)
   */
  type = uv_guess_handle(fd);
  if (type == UV_FILE || type == UV_UNKNOWN_HANDLE)
    return UV_EINVAL;

  flags = 0;
  newfd = -1;

  /* Save the fd flags in case we need to restore them due to an error. */
  do {
    saved_flags = fcntl(fd, F_GETFL);
    /* KNOWN_TEMP_FIX
     * Since KOS libc returns O_RDONLY for stderr instead of O_RDWR returned by
     * Linux libc, deps/uv/src/unix/stream.c -> uv__check_before_write fails on
     * if (!(stream->flags & UV_HANDLE_WRITABLE))
     *   return UV_EPIPE;
     * So we have broken console.error in JavaScript.
     * Here is a WA for this issue.
     * TODO: remove this WA.
     */
#if defined (TEST_KOS_SDK) && (TEST_KOS_SDK == 1)
#error "test and remove w/a"
#else
#if defined(__KOS__)
#warning "WA for stderr wrong O_RDONLY flag coming from KOS libc"
    if (fd==stderr->_file) saved_flags = (O_NOCTTY | O_RDWR);
#endif /* defined(__KOS__) */
#endif /* defined (TEST_KOS_SDK) && (TEST_KOS_SDK == 1) */
  }
  while (saved_flags == -1 && errno == EINTR);

  if (saved_flags == -1)
    return UV__ERR(errno);
  mode = saved_flags & O_ACCMODE;

  /* Reopen the file descriptor when it refers to a tty. This lets us put the
   * tty in non-blocking mode without affecting other processes that share it
   * with us.
   *
   * Example: `node | cat` - if we put our fd 0 in non-blocking mode, it also
   * affects fd 1 of `cat` because both file descriptors refer to the same
   * struct file in the kernel. When we reopen our fd 0, it points to a
   * different struct file, hence changing its properties doesn't affect
   * other processes.
   */
  if (type == UV_TTY) {
    /* Reopening a pty in master mode won't work either because the reopened
     * pty will be in slave mode (*BSD) or reopening will allocate a new
     * master/slave pair (Linux). Therefore check if the fd points to a
     * slave device.
     */
    if (uv__tty_is_slave(fd) && ttyname_r(fd, path, sizeof(path)) == 0)
      r = uv__open_cloexec(path, mode | O_NOCTTY);
    else
      r = -1;

    if (r < 0) {
      /* fallback to using blocking writes */
      if (mode != O_RDONLY)
        flags |= UV_HANDLE_BLOCKING_WRITES;
      goto skip;
    }

    newfd = r;

    r = uv__dup2_cloexec(newfd, fd);
    if (r < 0 && r != UV_EINVAL) {
      /* EINVAL means newfd == fd which could conceivably happen if another
       * thread called close(fd) between our calls to isatty() and open().
       * That's a rather unlikely event but let's handle it anyway.
       */
      uv__close(newfd);
      return r;
    }

    fd = newfd;
  }

skip:
  uv__stream_init(loop, (uv_stream_t*) tty, UV_TTY);

  /* If anything fails beyond this point we need to remove the handle from
   * the handle queue, since it was added by uv__handle_init in uv_stream_init.
   */

  if (!(flags & UV_HANDLE_BLOCKING_WRITES))
    uv__nonblock(fd, 1);

  if (mode != O_WRONLY)
    flags |= UV_HANDLE_READABLE;
  if (mode != O_RDONLY)
    flags |= UV_HANDLE_WRITABLE;

  uv__stream_open((uv_stream_t*) tty, fd, flags);
  tty->mode = UV_TTY_MODE_NORMAL;

  return 0;
}

static void uv__tty_make_raw(struct termios* tio) {
  assert(tio != NULL);

  /*
   * This implementation of cfmakeraw for Solaris and derivatives is taken from
   * http://www.perkin.org.uk/posts/solaris-portability-cfmakeraw.html.
   */
  tio->c_iflag &= ~(IMAXBEL | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR |
                    IGNCR | ICRNL | IXON);
  tio->c_oflag &= ~OPOST;
  tio->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tio->c_cflag &= ~(CSIZE | PARENB);
  tio->c_cflag |= CS8;

  /*
   * By default, most software expects a pending read to block until at
   * least one byte becomes available.  As per termio(7I), this requires
   * setting the MIN and TIME parameters appropriately.
   *
   * As a somewhat unfortunate artifact of history, the MIN and TIME slots
   * in the control character array overlap with the EOF and EOL slots used
   * for canonical mode processing.  Because the EOF character needs to be
   * the ASCII EOT value (aka Control-D), it has the byte value 4.  When
   * switching to raw mode, this is interpreted as a MIN value of 4; i.e.,
   * reads will block until at least four bytes have been input.
   *
   * Other platforms with a distinct MIN slot like Linux and FreeBSD appear
   * to default to a MIN value of 1, so we'll force that value here:
   */
  tio->c_cc[VMIN] = 1;
  tio->c_cc[VTIME] = 0;
}

int uv_tty_set_mode(uv_tty_t* tty, uv_tty_mode_t mode) {
  struct termios tmp;
  int fd;

  if (tty->mode == (int) mode)
    return 0;

  fd = uv__stream_fd(tty);
  if (tty->mode == UV_TTY_MODE_NORMAL && mode != UV_TTY_MODE_NORMAL) {
    if (tcgetattr(fd, &tty->orig_termios))
      return UV__ERR(errno);

    /* This is used for uv_tty_reset_mode() */
    uv_spinlock_lock(&termios_spinlock);
    if (orig_termios_fd == -1) {
      orig_termios = tty->orig_termios;
      orig_termios_fd = fd;
    }
    uv_spinlock_unlock(&termios_spinlock);
  }

  tmp = tty->orig_termios;
  switch (mode) {
    case UV_TTY_MODE_NORMAL:
      break;
    case UV_TTY_MODE_RAW:
      tmp.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
      tmp.c_oflag |= (ONLCR);
      tmp.c_cflag |= (CS8);
      tmp.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
      tmp.c_cc[VMIN] = 1;
      tmp.c_cc[VTIME] = 0;
      break;
    case UV_TTY_MODE_IO:
      uv__tty_make_raw(&tmp);
      break;
  }

  /* Apply changes after draining */
  if (tcsetattr(fd, TCSADRAIN, &tmp))
    return UV__ERR(errno);

  tty->mode = mode;
  return 0;
}


int uv_tty_get_winsize(uv_tty_t* tty, int* width, int* height) {
  struct winsize ws;
  int err;

  fprintf(stderr, "!!! KOS DEBUG !!! %s(%s:%d)\n",
          __func__, __FILE__, __LINE__);
  *width = 80;
  *height = 25;
  fprintf(stderr, "!!! KOS DEBUG !!! (fake) %s(%s:%d)\n",
          __func__, __FILE__, __LINE__);
  return 0;

  do
    err = ioctl(uv__stream_fd(tty), TIOCGWINSZ, &ws);
  while (err == -1 && errno == EINTR);

  if (err == -1)
    return UV__ERR(errno);

  *width = ws.ws_col;
  *height = ws.ws_row;

  /* KOS: TODO: check & remove if not needed for non-debug build. */
  fprintf(stderr, "!!! KOS DEBUG !!! %s(%s:%d) [%dx%d]\n",
          __func__, __FILE__, __LINE__, ws.ws_col, ws.ws_row);

  return 0;
}


uv_handle_type uv_guess_handle(uv_file file) {
  struct sockaddr sa;
  struct stat s;
  socklen_t len;
  int type;

  if (file < 0)
    return UV_UNKNOWN_HANDLE;

  if (isatty(file))
    return UV_TTY;

  if (fstat(file, &s))
    return UV_UNKNOWN_HANDLE;

  if (S_ISREG(s.st_mode))
    return UV_FILE;

  if (S_ISCHR(s.st_mode))
    return UV_FILE;  /* XXX UV_NAMED_PIPE? */

  if (S_ISFIFO(s.st_mode))
    return UV_NAMED_PIPE;

  if (!S_ISSOCK(s.st_mode))
    return UV_UNKNOWN_HANDLE;

  len = sizeof(type);
  if (getsockopt(file, SOL_SOCKET, SO_TYPE, &type, &len))
    return UV_UNKNOWN_HANDLE;

  len = sizeof(sa);
  if (getsockname(file, &sa, &len))
    return UV_UNKNOWN_HANDLE;

  if (type == SOCK_DGRAM)
    if (sa.sa_family == AF_INET || sa.sa_family == AF_INET6)
      return UV_UDP;

  if (type == SOCK_STREAM) {
    if (sa.sa_family == AF_INET || sa.sa_family == AF_INET6)
      return UV_TCP;
    if (sa.sa_family == AF_UNIX)
      return UV_NAMED_PIPE;
  }

  return UV_UNKNOWN_HANDLE;
}


/* This function is async signal-safe, meaning that it's safe to call from
 * inside a signal handler _unless_ execution was inside uv_tty_set_mode()'s
 * critical section when the signal was raised.
 */
int uv_tty_reset_mode(void) {
  int saved_errno;
  int err;

  saved_errno = errno;
  if (!uv_spinlock_trylock(&termios_spinlock))
    return UV_EBUSY;  /* In uv_tty_set_mode(). */

  err = 0;
  if (orig_termios_fd != -1)
    if (tcsetattr(orig_termios_fd, TCSANOW, &orig_termios))
      err = UV__ERR(errno);

  uv_spinlock_unlock(&termios_spinlock);
  errno = saved_errno;

  return err;
}

void uv_tty_set_vterm_state(uv_tty_vtermstate_t state) {
}

int uv_tty_get_vterm_state(uv_tty_vtermstate_t* state) {
  return UV_ENOTSUP;
}
