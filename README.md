# ByteTalk
1、
git clone https://github.com/FFmpeg/FFmpeg.git
cd FFmpeg
2、
./configure \
  --prefix=/usr/local/ffmpeg_static \
  --enable-static \
  --disable-shared \
  --disable-doc \
  --disable-programs \
  --enable-avdevice \
  --enable-avformat \
  --enable-avcodec \
  --enable-swscale \
  --enable-swresample \
  --enable-avfilter
make -j$(nproc)
make install