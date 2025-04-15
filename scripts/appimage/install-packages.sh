#!/usr/bin/env bash

echo "Install Packages Running..."
sudo dpkg --add-architecture i386
sudo apt update
sudo apt -y install --no-install-recommends \
  build-essential gcc-multilib g++-multilib cmake fuse freeglut3-dev:i386 libsdl2-dev:i386 libfaudio-dev:i386 libvdpau1:i386 \
  libstdc++5:i386 libxmu6:i386 libpcsclite1:i386 libncurses5:i386 unzip libsndio-dev:i386

sudo add-apt-repository ppa:kisak/kisak-mesa -y
sudo apt update -y && sudo apt upgrade -y 