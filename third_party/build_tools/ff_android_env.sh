#!/usr/bin/bash

NDK=/home/sivin/Android/Sdk/ndk/25.1.8937393
FFMPEG_SOURCE_DIR=

TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
CPU_ARCH=aarch64
TARGET=$CPU_ARCH-linux-android
API=30
export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

ADDI_LDFLAGS="-fPIE -pie"
FF_CFLAGS="-O3 -pipe -march=armv8-a -mfloat-abi=softfp -mfpu=neon\
    -std=c99 \
    -ffast-math \
    -fstrict-aliasing -Werror=strict-aliasing \
    -DANDROID -DNDEBUG"



