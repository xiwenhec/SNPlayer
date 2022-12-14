
#配置ffmpeg
FF_CONFIGURE_FLAGS="--prefix=$FF_PREFIX"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --target-os=$FF_PLATFORM"
#

FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-avdevice"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-ffmpeg"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-ffprobe"

FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-ffplay"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-sdl2"
#FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-ffplay"
#FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-sdl2"

FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-doc"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-symver"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-ffprobe"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-postproc"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-shared"

FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-optimizations"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-debug"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-small"
# FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-small"

FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-gpl"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-yasm"


FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-pic"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-pthreads"


FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-encoders"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-encoder=aac"


FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-decoders"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=h264"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=hevc"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=pcm_s16le"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=ac3"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=aac_latm"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=mp3"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=eac3"



FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-bsfs"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-bsf=h264_mp4toannexb"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-bsf=hevc_mp4toannexb"


FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-muxers"

FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-protocols"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-protocol=http"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-protocol=file"



FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-filters"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-filter=aresample"



FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-demuxers"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-demuxer=mp3"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-demuxer=mov"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-demuxer=flv"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-demuxer=live_flv"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-demuxer=mpegvideo"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-demuxer=mpegts"




FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --disable-parsers"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-parser=aac"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-parser=aac_latm"
FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-parser=h264"


if [ $FF_PLATFORM = "android" ]
then
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-cross-compile"
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --arch=$FF_ARCH"
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --sysroot=$SYSROOT"
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-neon"
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-jni"
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-mediacodec"
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=h264_mediacodec"
    FF_CONFIGURE_FLAGS="$FF_CONFIGURE_FLAGS --enable-decoder=mpeg4_mediacodec"
fi


FF_MODULE_DIRS="compat libavcodec libavfilter libavformat libavutil libswresample libswscale"
FF_ASSEMBLER_SUB_DIRS=

if [ $FF_PLATFORM = "android" ]
then
   FF_ASSEMBLER_SUB_DIRS="aarch64 neon"
fi