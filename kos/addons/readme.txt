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

For KOS-NodeJS we can't invoke those commands for cross-compile build
environment.
