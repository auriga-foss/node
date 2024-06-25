# Node.JS (v20.0.0-pre) for KasperskyOS
This is a fork of [Node.js project](https://github.com/nodejs/node) adapted to
be used with KasperskyOS. For more information about the target OS, please refer
to [KasperskyOS Community Edition Online Help](https://click.kaspersky.com/?hl=en-us&link=online_help&pid=kos&version=1.2&customization=KCE_community_edition).

Please note, that to be able to develop your own NodeJS application with KasperskyOS
you'd need to build OS principles understanding (see mention link for examples, API and so on).

For general information on using Node.js, its features and so on, please see the
[Node.js website](https://nodejs.org/en/).

Please refer to the original Node.js (parent) [README.md](https://github.com/nodejs/node/blob/main/README.md)
for more details not related to this fork.

## How to build (on Linux host)

### Setup
In order to build the code successfully, you'll need to install KasperskyOS Community Edition SDK
on your system. The latest version of the SDK can be downloaded from this
[link](https://os.kaspersky.com/development/).

All of the files required to build a web interpreter with KasperskyOS, and some
examples of how you can bridge it with your solution, are located in the folder:

```bash
./kos
```

Please note that the minimum required version of KasperskyOS Community Edition SDK is 1.2.
The recent SDK release could be downloaded from the project official [page](https://os.kaspersky.com/download-community-edition/).

### Build NodejS
Once you have cloned git repository with project sources and satisfied all the
requirements listed in [Building Node.js](https://github.com/nodejs/node/blob/main/BUILDING.md#building-nodejs-on-supported-platforms),
KasperskyOS-specific build is performed as follows:

1. Go to `./kos`.
2. Add `<KasperskyOS SDK root>/toolchain/bin` to your `PATH` environment variable.
3. Invoke `make configure` to do actual project config (this is a wrapper for
   `./configure` with a pre-defined arguments list).
4. Invoke `make compile -j XX` (select number of threads XX according to your
   system's HW performance).

### Prepare and run User-solution
Once you have the NodeJS build completed it is time to prepare your own Solution
with using the NodeJS and some custom JS-script. In order to do that there are
some files prepared at the `kos/image_builder` directory already, and all you
need to do is to invoke following command (staying at the `kos` folder):
```bash
SDK_VERSION=<KasperskyOS SDK version> make helloworld
```
This will build KasperskyOS image for Qemu and run very simple `Hello-World`
example, which is a very simple test to make sure that binary you've just built
actually works (this command will also build KasperskyOS romfs image and so on). Please,
note that `make helloworld` command will run `qemu-system-aarch64` from SDK and
will, eventually, run KasperskyOS-image under emulator control.

Note: specifying `SDK_VERSION` is only necessary when `<KasperskyOS SDK root>` contains
SDK which number differs from what is assigned in `make-vars.mk` (see
[`Directory structure`](#directory-structure) section below).

### Running Solution with Raspberry Pi4B
It is also possible to run NodeJS as part of KasperskyOS-based
user solution on the Raspberry Pi 4B board. 
Further instructions describe the way to build and run 
`hello world` NodeJS application.

First of all it is advised to prepare bootable flash-card following
instruction from
the [KasperskyOS Community Edition Online Help](https://click.kaspersky.com/?hl=en-us&link=online_help&pid=kos&version=1.2&customization=KCE_preparing_sd_card_rpi) or using either [Raspberry Pi Imager](https://www.raspberrypi.com/news/raspberry-pi-imager-imaging-utility/)
or grab & flash pre-build board image, which could be found [here](https://www.raspberrypi.com/software/operating-systems/)

As the Raspberry Pi has no u-boot shipped with image, it is required to prepare
one for you specific board and update board boot configuration to start u-boot
binary instead of default RaspbianOS kernel (see guide from the [KasperskyOS Community Edition Online Help](https://click.kaspersky.com/?hl=en-us&link=online_help&pid=kos&version=1.2&customization=KCE_preparing_sd_card_rpi)
or do your own setup).

Then, the easiest way to prepare KasperskyOS image to run on hardware is
invoke following commands:

```
make qemubuild
make helloworld FOR_HW=1
```

And eventually, copy the `kos/image_builder/build-arm64/einit/kos-image` file to
the first partition of the flash-card. Then plug it (sd-card) in, power-up the
board and have fun.

The explicit way to build NodeJS with `hello world` application is

```
make qemubuild
make realhw NODE_ARG="/opt/node/test/message/hello_world.js"
```


You can find all the applications in the directory:
`kos/image_builder/build-arm64/rootfs/root/opt/node/test`


If you still need some hint how to run it using u-boot, so here is the
commands, assuming that your boot partition is formatted to FAT32.

```
fatload mmc 0 ${loadaddr} kos-image
bootelf ${loadaddr} ${fdt_addr}
```

So the solution will start.

## Directory structure

* `Makefile` - main Makefile to configure & build Node.js project for KasperskyOS.
  Use `make help` target to see available options & usage guidelines.
* `add-ons` - example `hello` add-on along with its build fixtures.
* `image_builder` - sources and CMake-based build system for KasperskyOS images along
  with build output files.
  * `build-<arch>` - architecture-specific KasperskyOS build artifacts.
  * `einit`, `env`, `klog_storage` - sources and CMake build rules for
    corresponding entities.
  * `node` - CMake build rules for Node.js itself.
  * `resources` - description files for all the entities.
    * `certs` - TLS-related stuff.
    * `edl` - entities *.edl description files
    * `ramfs` - additional files to be added to the KasperskyOS image filesystems.
  * `CMakeLists.txt` - project-level CMake build rules.
* `rules` - build rules for the aarch64 Node.js customized project (all the files
  from this folder are included into main Makefile, see above).
  * `make-images.mk` - a set of rules to build KasperskyOS images.
  * `make-vars.mk` - declaration of variables to control build rules.
  * `make-run-tests.mk`- a set of rules to run KasperskyOS test images.
  * `make-debug.mk` - a set of debug-enabled build targets.

## Known issues and limitations

1. `IPv6` is not yet supported at the moment, as it's not available in KasperskyOS for now.
2. Subprocess creation isn't available in KasperskyOS for now.
3. No `OpenSSL` CLI (since it needs to start a separate process(es) in runtime).
4. `process.getuid()` isn't supported (corresponding KasperskyOS API calls
   `getuid`/`geteuid` always return 0).
5. `fs.watch()` isn't supported (returns `ENOSYS` in KasperskyOS).
6. `Socket.addSourceSpecificMembership()` isn't supported (returns `ENOSYS` in KasperskyOS).
7. `fs.utimesSync()` isn't supported (`utime` returns `ENOSYS` in KasperskyOS).
8. `process.cpuUsage()` and `process.resourceUsage()` aren't supported (return
   `ENOSYS` in KasperskyOS).
9. `process.kill()` isn't supported (returns `ENOSYS` in KasperskyOS due to limitation
   of inter-process communication).
10. `Node.js inspector` (V8 inspector, profiler and sampler) isn't fully
   supported (due to KasperskyOS signals implementation restrictions).
11. TLS isn't fully supported (simple code responses appear to work as expected,
   whereas responses with body are 0-sized).
12. No more than 256 threads per process can be created w/o any issues creating
   more threads is likely to cause `Unhandled Page Fault`).
13. Single core configurations (i.e. `QEMU` option `-smp 1`) aren't supported.
14. Unix domain sockets (i.e. `AF_UNIX`/`AF_LOCAL`) are not bindable.
    is an empty string).
15. `getaddrinfo` failures (returns `EAI_AGAIN` or `ENOTFOUND`) on Raspberry
    Pi 4B target (for instance `test-async-exec-resource-http.js`);
16. KasperskyOS image transfer failures over TFTP (for instance `test-async-local-storage-promises.js`);
17. Timeouts (for instance `test-esm-loader-thenable.mjs`): 2 minutes per test
    isn't enough for real HW every now and then, even though typical test takes
    roughly 1 minute.
18. Building Node.js with addons is not supported.

## How to run tests

In order to be able to run Node.js tests for KasperskyOS, its configuration and build
have to be done using the following commands (from `<project repository
root>/kos` folder):

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

## How to run demos

There are three tested demo projects on the [TotalJS](https://github.com/totaljs) platform:
- [CMS](https://github.com/totaljs/cms)
- [File manager](https://github.com/totaljs/files)
- [Messenger](https://github.com/totaljs/messenger)

To run these demos, firstly you need to build NodeJS according to the [instructions](#build-nodejs). \
Then it's necessary to clone these three demo repositories into a free folder:
```bash
git clone https://github.com/totaljs/cms
git clone https://github.com/totaljs/files
git clone https://github.com/totaljs/messenger
```

To automatically install dependencies for demo projects, you should have [npmjs](https://www.npmjs.com/) pre-installed on the host system. \
After cloning the repositories, you need to go to the folder with the desired demo project and run:
```bash
npm install
```

It is also necessary to uncomment the line: `options.port = parseInt(process.argv[2])` in the files `cms\index.js`, `files\index.js`, `messenger\release.js`.

To start the `CMS` and `File Manager` demos it is required to run the following command:
```bash
make run NODE_ARG="/demo/index.js;\"8080\";--release" NODE_PORT=1234 QEMU_NET=1 NODE_DEMO=1 DEMO_PATH=<path to demo>
```
`DEMO_PATH` - path to the demo directory.

To run the `Messenger` demo project, run the following command:
```bash
make run NODE_ARG="/demo/release.js;\"8080\";--release" NODE_PORT=1234 QEMU_NET=1 NODE_DEMO=1 DEMO_PATH=<path to demo>
```

Running demos will be available from the browser at http://localhost:1234/. The port can be configured through a variable `NODE_PORT`. \
It is not recommended to change port `8080` as `QEMU` is configured to only forward from this port to the port specified in `NODE_PORT`.

## Changed third-party components
Following list of built-in third-party subprojects which were changed
with respect to enabling KasperskyOS support:
- deps/brotli;
- deps/cares;
- deps/uv;
- deps/uvwasi;
- deps/v8;

## Contributing
Please see the [CONTRIBUTING.md](CONTRIBUTING.md) page for generic info.

We'll follow the parent project contributing rules but would consider to accept only
KasperskyOS-specific changes, so for that it is advised to use pull-requests.

## Licensing
This version of NodeJS is available under the MIT license. Just like
a vanilla version, it also includes external libraries that are available
under a variety of licenses. See [LICENSE](LICENSE) for the full license text.
