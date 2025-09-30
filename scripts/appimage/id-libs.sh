#!/bin/bash

PACKAGES=(
    "libdrm-amdgpu1:i386"
    "libdrm-intel1:i386"
    "libdrm-nouveau2:i386"
    "libdrm-radeon1:i386"
    "libgl1-mesa-dri:i386"
    "libglx-mesa0:i386"
)

COMPRESSION_TARGET_PATH="ext/usr/lib/i386-linux-gnu"

cd libs

rm -rf deb_files
mkdir -p deb_files
cd deb_files

apt-get download "${PACKAGES[@]}"

mkdir -p ext
for deb_file in *.deb; do
    dpkg-deb -x "$deb_file" ext
done

rm ext/usr/lib/i386-linux-gnu/dri/i915_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/i915_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/iris_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/iris_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/kms_swrast_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/kms_swrast_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/nouveau_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/nouveau_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/r300_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/r300_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/r600_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/r600_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/radeonsi_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/radeonsi_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/swrast_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/swrast_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/virtio_gpu_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/virtio_gpu_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/vmwgfx_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/vmwgfx_dri.so
rm ext/usr/lib/i386-linux-gnu/dri/zink_dri.so
ln -s -f crocus_dri.so ext/usr/lib/i386-linux-gnu/dri/zink_dri.so




# cp -d /usr/lib/i386-linux-gnu/libedit.so* ext/usr/lib/i386-linux-gnu
# cp -d /usr/lib/i386-linux-gnu/libglapi.so* ext/usr/lib/i386-linux-gnu
# cp -d /usr/lib/i386-linux-gnu/libLLVM-15.so* ext/usr/lib/i386-linux-gnu
# cp -d /usr/lib/i386-linux-gnu/libsensors.so* ext/usr/lib/i386-linux-gnu

tar -cf lib32ID.tar -C "$COMPRESSION_TARGET_PATH" .

mv lib32ID.tar ../

cd ..

rm -rf deb_files
