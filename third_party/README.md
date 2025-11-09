#ffmpeg
cd third_party/ffmpeg
./configure --prefix=$(pwd)/build \
            --disable-shared \
            --enable-static \
            --disable-doc \
            --disable-programs \
            --disable-sdl2 \
            --disable-sndio \
            --disable-drm
make -j$(nproc)
make install