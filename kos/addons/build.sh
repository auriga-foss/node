export HOST=aarch64-kos
export CPP="${HOST}-gcc -E"
export STRIP="${HOST}-strip"
export OBJCOPY="${HOST}-objcopy"
export AR="${HOST}-ar"
export RANLIB="${HOST}-ranlib"
export LD="${HOST}-ld"
export OBJDUMP="${HOST}-objdump"
export CC="${HOST}-gcc"
export CXX="${HOST}-g++"
export NM="${HOST}-nm"
export AS="${HOST}-as"
export LD="$CXX"
export LINK="$CXX"
export PATH=$PATH:/opt/KasperskyOS-Community-Edition-1.1.0.76/toolchain/bin

rm -rf build

node-gyp --arch aarch64 --dest-cpu=arm64 --fully-static configure build \
  --nodedir $PWD/../../


pushd build/Release

echo "Workaround: by some reasons we can't get prefix lib ..."
cp test_addon.a libtest_addon.a

popd
