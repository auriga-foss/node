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

#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <locale.h>

static int uv__dlerror(uv_lib_t* lib);

#if defined(__KOS__) && (_DL_USE_FAKE_LOAD == 1)

struct addon_t {
  char *file_name;
  void *addon_name;
};

#define ADDON_ENTRY(_id, _fn, _nm) \
  [_id] = { .file_name = _fn, .addon_name = _nm }

static char* error_addon_str = "error:not found";

/* Allowed addons list */
static struct addon_t addons[] = {
  ADDON_ENTRY(0, "/opt/node/test/kos/addons/test_addon.node", "test_addon"),
};

const char* uv_get_addon_name(void* handle) {
  if (!handle) {
    return error_addon_str;
  }
  return (const char*)((struct addon_t*)handle)->addon_name;
}

static void* get_addon_handle(const char* filename) {
  int i;
  for (i = 0; i < sizeof(addons) / sizeof(addons[0]); i++) {
    if (!strcmp(addons[i].file_name, filename)) {
      return (void*)&addons[i];
    }
  }
  return NULL;
}

#endif /* defined(__KOS__) && _DL_USE_FAKE_LOAD == 1 */

int uv_dlopen(const char* filename, uv_lib_t* lib) {
  dlerror(); /* Reset error status. */
  lib->errmsg = NULL;
  lib->handle = dlopen(filename, RTLD_LAZY);
#if defined(__KOS__) && (_DL_USE_FAKE_LOAD == 1)
  if (!lib->handle) {
    lib->handle = get_addon_handle(filename);
  }
#endif
  return lib->handle ? 0 : uv__dlerror(lib);
}


void uv_dlclose(uv_lib_t* lib) {
  uv__free(lib->errmsg);
  lib->errmsg = NULL;

  if (lib->handle) {
    /* Ignore errors. No good way to signal them without leaking memory. */
    dlclose(lib->handle);
    lib->handle = NULL;
  }
}


int uv_dlsym(uv_lib_t* lib, const char* name, void** ptr) {
  dlerror(); /* Reset error status. */
  *ptr = dlsym(lib->handle, name);
  return *ptr ? 0 : uv__dlerror(lib);
}


const char* uv_dlerror(const uv_lib_t* lib) {
  return lib->errmsg ? lib->errmsg : "no error";
}


static int uv__dlerror(uv_lib_t* lib) {
  const char* errmsg;

  uv__free(lib->errmsg);

  errmsg = dlerror();

  if (errmsg) {
    lib->errmsg = uv__strdup(errmsg);
    return -1;
  }
  else {
    lib->errmsg = NULL;
    return 0;
  }
}
