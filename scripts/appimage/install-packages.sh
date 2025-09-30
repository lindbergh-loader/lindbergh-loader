#!/usr/bin/env bash

echo "Install Packages Running..."
sudo dpkg --add-architecture i386
sudo apt update # && sudo apt upgrade -y
sudo apt -y install --no-install-recommends \
  build-essential gcc-multilib g++-multilib cmake fuse freeglut3-dev:i386 libvdpau1:i386 libstdc++5:i386 libxmu6:i386 \
  libpcsclite1:i386 libncurses5:i386 unzip libsndio-dev libsndio-dev:i386 pulseaudio-utils:i386 zlib1g:i386 libgpg-error0:i386 \
  libasound2 libasound2-dev libasound2:i386 libasound2-dev:i386 libfreetype6-dev:i386 libdbus-1-dev libpulse-dev libdbus-1-dev:i386 \
  libudev-dev:i386 libxcursor-dev:i386 libxfixes-dev:i386 libxi-dev:i386 libxrandr-dev:i386 libxss-dev:i386 libxxf86vm-dev:i386 git libvulkan1:i386 \
  mesa-vulkan-drivers:i386
