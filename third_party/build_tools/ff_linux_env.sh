#!/usr/bin/bash
FF_ARCH=x86_64
FF_PREFIX=$(pwd)/../ffmpeg_build/$FF_ARCH
FF_PLATFORM=linux
FF_CFLAGS="-std=c99"
FF_EXTRA_LDFLAGS="-lpthread"