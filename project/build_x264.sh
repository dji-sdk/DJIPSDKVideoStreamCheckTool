PREFIX=../../../out/x264
./configure \
--prefix=$PREFIX \
--enable-static \
--disable-gpl \
--disable-opencl \
--disable-avs \
--disable-swscale \
--disable-lavf \
--disable-ffms \
--disable-gpac \
--disable-lsmash \
--disable-thread \
--disable-cli \
--disable-asm

make clean
make -j8
make install
