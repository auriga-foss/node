# Node.js for KOS

Version of the Node.js adapted for use on Kaspersky OS.
For information on using Node.js, see the [Node.js website][https://nodejs.org/en/].
Please refer to the original Node.js (parent) [README.md](https://github.com/nodejs/node/blob/main/README.md) for more details.

For a default build and use, you need to install the Kaspersky OS SDK on your system. The latest version of the SDK can be downloaded from this [link](https://os.kaspersky.com/development/).

All files required to build a web interpreter with Kaspersky OS and an examples of connecting it to your solution are located in the folder:

```bash
./kos
````

## How-to build (on Linux host)

Once you have cloned git repository with project sources and satisfied all the
requirements listed in [Building Node.js](https://github.com/nodejs/node/blob/main/BUILDING.md#building-nodejs-on-supported-platforms),
KOS-specific build is performed as follows:

1. Go to `./kos`.
2. Add `<KOS SDK root>/toolchain/bin` to your `PATH` environment variable.
3. Invoke `make configure` to do actual project config (this is a wrapper for
   `./configure` with a pre-defined arguments list).
4. Invoke `make compile -j XX` (select number of threads XX according to your
   system's HW performance).
5. Invoke `SDK_VERSION=<KOS SDK version> make helloworld` in order to run
   a very simple test to make sure just built binary works (this command will
   also build KOS romfs image). \
   Note: specifying `SDK_VERSION` is only necessary when `<KOS SDK root>`
   contains SDK which number differs from what is assigned in `make-vars.mk`
   (see [`Directory structure`](#directory-structure) section below).

## Directory structure

* `Makefile` - main Makefile to configure & build Node.js project for KOS.
  Use `make help` target to see available options & usage guidelines.
* `add-ons` - example `hello` add-on along with its build fixtures.
* `image_builder` - sources and CMake-based build system for KOS images along
  with build output files.
  * `build-<arch>` - architecture-specific KOS build artifacts.
  * `certs` - TLS-related stuff.
  * `einit`, `env`, `klog_storage` - sources and CMake build rules for
    corresponding entities.
  * `node` - CMake build rules for Node.js itself.
  * `resources` - description files for all the entities.
  * `CMakeLists.txt` - project-level CMake build rules.
* `resources` - additional files to be added to the KOS image filesystems.
* `rootfs` - Node.js files to be added to the KOS image filesystems.
* `rules` - build rules for the aarch64 Node.js customized project (all the files
  from this folder are included into main Makefile, see above).
  * `make-images.mk` - a set of rules to build KOS images.
  * `make-vars.mk` - declaration of variables to control build rules.
  * `make-run-tests.mk`- a set of rules to run KOS test images.
  * `make-debug.mk` - a set of debug-enabled build targets.

## Known issues and limitations

1. No `IPv6` support in KOS.
2. Subprocess creation isn't available in KOS.
3. No `OpenSSL` CLI (since it needs to start a separate process(es) in runtime).
4. `process.getuid()` isn't supported (corresponding KOS API calls
   `getuid`/`geteuid` always return 0).
5. `fs.watch()` isn't supported (returns `ENOSYS` in KOS).
6. `Socket.addSourceSpecificMembership()` isn't supported (returns `ENOSYS` in
   KOS).
7. `fs.utimesSync()` isn't supported (`utime` returns `ENOSYS` in KOS).
8. `process.cpuUsage()` and `process.resourceUsage()` aren't supported (return
   `ENOSYS` in KOS).
9. `process.kill()` isn't supported (returns `ENOSYS` in KOS due to
   limitation of inter-process communication).
10. `Node.js inspector` (V8 inspector, profiler and sampler) isn't fully
    supported (due to KOS signals implementation restrictions).
11. TLS isn't fully supported (simple code responses appear to work as expected,
    whereas responses with body are 0-sized).
12. No more than 256 threads can be created w/o any issues (creating more
    threads is likely to cause `Unhandled Page Fault`).
13. Single core configurations (i.e. `QEMU` option `-smp 1`) aren't supported.
14. Unix domain sockets (i.e. `AF_UNIX`/`AF_LOCAL`) are not bindable.
15. Host name isn't picked up from `/etc/hostname` file (default host name is
    an empty string).
16. TCP socket buffer size seems to be fixed.
17. `getaddrinfo` failures (returns `EAI_AGAIN` or `ENOTFOUND`) on Raspberry Pi 4B target (for instance `test-async-exec-resource-http.js`);
18. KOS image transfer failures over TFTP (for instance `test-async-local-storage-promises.js`);
19. Timeouts (for instance `test-esm-loader-thenable.mjs`). 2 minutes per test isn't enough for real HW every now and again, even though typical test takes roughly 1 minute?).

## How-to run tests

In order to be able to run Node.js tests for KOS, its configuration and build
have to be done using the following commands (from
`<project repository root>/kos` folder):

```bash
make configure CFLAGS=-DKOS_NODE_TEST=1 CXXFLAGS=-DKOS_NODE_TEST=1
make compile -j XX CFLAGS=-DKOS_NODE_TEST=1 CXXFLAGS=-DKOS_NODE_TEST=1
python -m pip install pandas psutil
```

The latter one installs `pandas` and `psutil` Python modules for the current
user (depending on Linux distribution being used this command might require
`python-is-python3` package to have been installed).

After the build has completed all the tests can be run with:

```bash
make tests options="-j XX --test-root=../test -t 120"
```

Again, the number of threads XX depends on your system's HW performance. \
Please also note: tests are being run via `QEMU` with `-smp 4` option thus
allowing each thread to spawn 3 others (even though it might not be the case
for the particular `QEMU` simulation running Node.js test(s)).

Alternatively, one may wish to run a specific test as follows:

```bash
make tests options="-j XX --logfile test.log --test-root=../test --filter-test=<path to the test .js file> -t 120"
```

Path to `.js` file is relative to the folder specified as `--test-root`. \
`--filter-test` parameter allows specifying several (comma-separated) `.js`
files with tests as well as a (relative) path to a folder containing `.js`
files (i.e. `parallel/`).
