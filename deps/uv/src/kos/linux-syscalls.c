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

#include "linux-syscalls.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

struct uv__mmsghdr;

int uv__sendmmsg(int fd, struct uv__mmsghdr* mmsg, unsigned int vlen) {
  return errno = ENOSYS, -1;
}


int uv__recvmmsg(int fd, struct uv__mmsghdr* mmsg, unsigned int vlen) {
  return errno = ENOSYS, -1;
}


ssize_t uv__preadv(int fd, const struct iovec *iov, int iovcnt, int64_t offset) {
#if !defined(__NR_preadv) || defined(__ANDROID_API__) && __ANDROID_API__ < 24
  return errno = ENOSYS, -1;
#else
  return syscall(__NR_preadv, fd, iov, iovcnt, (long)offset, (long)(offset >> 32));
#endif
}


ssize_t uv__pwritev(int fd, const struct iovec *iov, int iovcnt, int64_t offset) {
#if !defined(__NR_pwritev) || defined(__ANDROID_API__) && __ANDROID_API__ < 24
  return errno = ENOSYS, -1;
#else
  return syscall(__NR_pwritev, fd, iov, iovcnt, (long)offset, (long)(offset >> 32));
#endif
}


int uv__dup3(int oldfd, int newfd, int flags) {
#if !defined(__NR_dup3) || defined(__ANDROID_API__) && __ANDROID_API__ < 21
  return errno = ENOSYS, -1;
#else
  return syscall(__NR_dup3, oldfd, newfd, flags);
#endif
}


ssize_t
uv__fs_copy_file_range(int fd_in,
                       off_t* off_in,
                       int fd_out,
                       off_t* off_out,
                       size_t len,
                       unsigned int flags)
{
#ifdef __NR_copy_file_range
  return syscall(__NR_copy_file_range,
                 fd_in,
                 off_in,
                 fd_out,
                 off_out,
                 len,
                 flags);
#else
  return errno = ENOSYS, -1;
#endif
}


int uv__statx(int dirfd,
              const char* path,
              int flags,
              unsigned int mask,
              struct uv__statx* statxbuf) {
#if !defined(__NR_statx) || defined(__ANDROID_API__) && __ANDROID_API__ < 30
  return errno = ENOSYS, -1;
#else
  return syscall(__NR_statx, dirfd, path, flags, mask, statxbuf);
#endif
}


ssize_t uv__getrandom(void* buf, size_t buflen, unsigned flags) {
#if !defined(__NR_getrandom) || defined(__ANDROID_API__) && __ANDROID_API__ < 28
  return errno = ENOSYS, -1;
#else
  return syscall(__NR_getrandom, buf, buflen, flags);
#endif
}
