#!/usr/bin/bash
BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
PATH=$PATH:${BUILD_TOOLS_DIR}

source ff_linux_env.sh
source ffmpeg_config.sh

# FF_SOURCE=$(pwd)/../ffmpeg

cleanBuild() {
    rm -rf $FF_PREFIX
}

#配置编译ffmpeg
configure()
{
    ./configure $FF_CONFIGURE_FLAGS  --extra-cflags="$FF_CFLAGS"
}

build()
{
    echo ""
    echo "--------------------------"
    echo "[*] start configure ffmpeg"
    echo "--------------------------"

    configure
    make clean

    echo ""
    echo "------------------------"
    echo "[*] start complie ffmpeg"
    echo "------------------------"   
    make -j4
    make install
}




createSharedLib() {
    echo ""
    echo "------------------------"
    echo "[*] create libsnffmpeg.so"
    echo "------------------------" 
    #存放所有编译后的.o文件
    //TODO:不要忘记子目录里的.o文件
    FF_C_OBJ_FILES=
    FF_ASM_OBJ_FILES=
    for MODULE_DIR in $FF_MODULE_DIRS
    do
        C_OBJ_FILES="$MODULE_DIR/*.o"
        if ls $C_OBJ_FILES 1> /dev/null 2>&1; then
            echo "link $MODULE_DIR/$C_OBJ_FILES.o"
            FF_C_OBJ_FILES="$FF_C_OBJ_FILES $C_OBJ_FILES"
        fi

        for ASM_SUB_DIR in $FF_ASSEMBLER_SUB_DIRS
        do
            ASM_OBJ_FILES="$MODULE_DIR/$ASM_SUB_DIR/*.o"
            if ls $ASM_OBJ_FILES 1> /dev/null 2>&1; then
                echo "link $MODULE_DIR/$ASM_SUB_DIR/*.o"
                FF_ASM_OBJ_FILES="$FF_ASM_OBJ_FILES $ASM_OBJ_FILES"
            fi
        done
    done


    FF_EXTRA_LDFLAGS="-lpthread"
    mkdir -p $FF_PREFIX/include/libffmpeg

    cc -lm -lz -shared -Wl,--no-undefined $FF_EXTRA_LDFLAGS \
        -Wl,-soname=libsnffmpeg.so \
        $FF_C_OBJ_FILES \
        $FF_ASM_OBJ_FILES \
        -o $FF_PREFIX/libsnffmpeg.so

    strip $FF_PREFIX/libsnffmpeg.so
    #删除多余文件    
}

cd ../ffmpeg
cleanBuild
build
#createSharedLib