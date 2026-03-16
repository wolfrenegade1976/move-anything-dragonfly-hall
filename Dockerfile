FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    g++-aarch64-linux-gnu make git pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

RUN git clone --depth 1 --branch 3.2.10 \
    https://github.com/michaelwillis/dragonfly-reverb.git \
    && cd dragonfly-reverb \
    && git submodule update --init --recursive

RUN cd /build/dragonfly-reverb && \
    CROSS_COMPILING=true \
    CC=aarch64-linux-gnu-gcc \
    CXX=aarch64-linux-gnu-g++ \
    STRIP=aarch64-linux-gnu-strip \
    AR=aarch64-linux-gnu-ar \
    make -C plugins/dragonfly-hall-reverb \
    HAVE_DGL=false HAVE_ALSA=false HAVE_PULSEAUDIO=false \
    HAVE_X11=false HAVE_OPENGL=false

COPY src/dsp/hall_plugin.cpp /build/hall_plugin.cpp

ENV CXX=aarch64-linux-gnu-g++

RUN $CXX -O2 -fPIC -std=c++14 -w \
    -I dragonfly-reverb/plugins/dragonfly-hall-reverb \
    -I dragonfly-reverb/common \
    -I dragonfly-reverb/dpf/distrho \
    -c hall_plugin.cpp -o hall_plugin.o && \
    ls -lh hall_plugin.o

RUN $CXX -shared -fPIC \
    -Wl,--no-as-needed \
    hall_plugin.o \
    /build/dragonfly-reverb/build/DragonflyHallReverb/DSP.cpp.o \
    /build/dragonfly-reverb/common/freeverb/*.cpp.o \
    /build/dragonfly-reverb/common/kiss_fft/*.o \
    -lm -lstdc++ \
    -o dsp.so && \
    aarch64-linux-gnu-strip dsp.so && \
    ls -lh dsp.so

FROM scratch AS export
COPY --from=builder /build/dsp.so /
