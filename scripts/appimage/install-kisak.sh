#!/usr/bin/env bash

sudo snap refresh --hold=forever
sudo apt-mark hold linux-image-$(uname -r)
sudo apt-mark hold linux-headers-$(uname -r)
sudo add-apt-repository ppa:kisak/turtle -y
sudo apt update -y && sudo apt upgrade -y 