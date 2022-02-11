############
# Vanilla

For vanilla node building addons is simply done by the following commands:

1. Configure build first (this will simply create a set of makefiles for build).

  node-gyp configure

2. Do actual build.

  node-gyp buid

3. The binaries are available in the './Build/Release' folder.

Note:

More info could be found here: https://nodejs.org/api/addons.html

###########
# KOS-NodeJS

For KOS-NodeJS we need to install host-build of NodeJS in order to be able to
run npm and node-gyp commands:

1. build & install vanilla node into $HOME/.local/ folder
2. update PATH with ~/.local/bin
3. run npm install -g node-gyp
4. cd to kos/addons folder

.. so here comes the trick ..

5. run bash build.sh, it will provide static library at build/Release folder
6. now build nodejs, it will link libraries from addons build directory
