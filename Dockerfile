FROM ubuntu:22.04 AS lindbergh-build

RUN dpkg --add-architecture i386 \
    && apt update \
    && apt install -y --no-install-recommends \
        build-essential \
        gcc-multilib \
        g++-multilib \
        cmake \
        fuse \
        freeglut3-dev:i386 \
        libfaudio-dev:i386 \
        libvdpau1:i386 \
        libstdc++5:i386 \
        libxmu6:i386 \
        libpcsclite1:i386 \
        libncurses5:i386 \
        unzip \
        libudev-dev:i386 \
        zlib1g:i386 \
        libgpg-error0:i386 \
        libxcursor-dev:i386 \
        libxfixes-dev:i386 \
        libxi-dev:i386 \
        libxrandr-dev:i386 \
        libxss-dev:i386 \
        libxxf86vm-dev:i386 \
        git \
    && apt clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /lindbergh-loader

COPY . .

RUN ls

RUN tar -xf libs/libs.tar -C /usr/lib/i386-linux-gnu
RUN tar -xf libs/includes.tar -C /usr/include

# Explicitly set the output directory
RUN make && mkdir -p /output && cp -r ./build /output