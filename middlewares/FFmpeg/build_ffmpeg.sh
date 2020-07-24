PREFIX=../../out/ffmpeg
X264_PATH=../../out/x264/
PKG_CONFIG_PATH=$X264_PATH/../lib/pkgconfig:$PKG_CONFIG_PATH

./configure \
--enable-gpl \
--enable-version3 \
--prefix=$PREFIX \
--extra-cflags="-I$X264_PATH/include" \
--extra-cxxflags="-I$X264_PATH/include" \
--extra-ldflags="-L$X264_PATH/lib" \
--enable-ffmpeg \
--disable-shared \
--enable-static \
--disable-debug \
--disable-everything \
--disable-doc \
--enable-libx264 \
--enable-parser=h264 \
--enable-encoder=libx264 \
--enable-decoder=h264 \
--enable-demuxer=h264 \
--enable-muxer=h264 \
--disable-x86asm

make clean
make -j16
make install
