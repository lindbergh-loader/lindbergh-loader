#!/usr/bin/env bash

echo "Install Packages Running..."
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get -y install --no-install-recommends \
  build-essential gcc-multilib fuse freeglut3-dev:i386 libsdl2-dev:i386 libfaudio-dev:i386 libvdpau1:i386 \
  libstdc++5:i386 libxmu6:i386 libpcsclite1:i386 libncurses5:i386 unzip

sudo add-apt-repository ppa:kisak/kisak-mesa -y
sudo apt-get -y update && sudo apt -y dist-upgrade
