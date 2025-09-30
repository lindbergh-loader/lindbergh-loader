#!/usr/bin/env bash

APPIMAGENAME=lindbergh-loader
APPIMAGEDIRNAME=lindbergh-loader.AppDir

declare -a ADDED_LIBS=(
	"libasound.so.2"
	"libfreetype.so.6"
	"libOpenGL.so.0"
	"libGLdispatch.so.0"
	"libGLX.so"
	"libGLX_indirect.so.0"
	"libGLX_mesa.so.0"
	"libGL.so.1"
	"libGLU.so.1"
	"libglut.so.3"
	"libglapi.so.0"
	"libICE.so.6"
	"libSM.so.6"
	"libX11.so.6"
	"libX11-xcb.so.1"
	"libxcb.so.1"
	"libxcb-dri2.so.0"
	"libxcb-dri3.so.0"
	"libxcb-glx.so.0"
	"libXmu.so.6"
	"libXrandr.so.2"
	"libncurses.so.5"
	"libpcsclite.so.1"
	"libstdc++.so.5"
	"libdrm.so.2"
	"libexpat.so.1"
	"libm.so.6"
	"libz.so.1"
	"libgpg-error.so.0"
	"libstdc++.so.6"
	"libudev.so.1"
	"libsndio.so.7"
	"libvulkan.so.1"
	"libVkLayer_INTEL_nullhw.so"
	"libVkLayer_MESA_device_select.so"
	"libVkLayer_MESA_overlay.so"
	"libvulkan_intel_hasvk.so"
	"libvulkan_intel.so"
	"libvulkan_lvp.so"
	"libvulkan_radeon.so"
	"libvulkan_virtio.so"
)

set -e

if [ ! -f "linuxdeploy-x86_64" ]; then
	wget -O "linuxdeploy-x86_64" https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
	chmod +x "linuxdeploy-x86_64"
fi

if [ ! -f "appimagetool-x86_64" ]; then
	wget -O "appimagetool-x86_64" https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
	chmod +x "appimagetool-x86_64"
fi

cd build
OUTPUT_FOLDER=$(realpath "./$APPIMAGEDIRNAME")
rm -fr "$OUTPUT_FOLDER"

ADDED_LIBS_ARGS=()
for lib in "${ADDED_LIBS[@]}"; do
	ADDED_LIBS_ARGS+=("--library=/usr/lib/i386-linux-gnu/$lib")
done 

ADDED_LIBS_ARGS+=("--library=/usr/lib/i386-linux-gnu/pulseaudio/libpulsedsp.so")

echo "Copying files into AppDir..."
mkdir -p "$OUTPUT_FOLDER/usr/bin"
mkdir -p "$OUTPUT_FOLDER/usr/lib"
mkdir -p "$OUTPUT_FOLDER/usr/lib32/dri"
mkdir -p "$OUTPUT_FOLDER/usr/lib32ID"
mkdir -p "$OUTPUT_FOLDER/usr/share/vulkan/icd.d"


# We create the AppRun file
cat > "$OUTPUT_FOLDER/AppRun" << 'EOF'
#!/bin/bash
APP_IMG_ROOT="$(dirname "$(readlink -f "$0")")"
export APP_IMG_ROOT=$APP_IMG_ROOT
VK="/$APP_IMG_ROOT/usr/share/vulkan/icd.d"
export VK_ICD_FILENAMES="$VK/intel_hasvk_icd.i686.json:$VK/intel_icd.i686.json:$VK/lvp_icd.i686.json:$VK/radeon_icd.i686.json:$VK/virtio_icd.i686.json"
export LD_LIBRARY_PATH="$APP_IMG_ROOT/usr/lib32:$LD_LIBRARY_PATH"
exec "$APP_IMG_ROOT/usr/bin/lindbergh" "$@"
EOF

chmod +x "$OUTPUT_FOLDER/AppRun"

cp libkswapapi.so $OUTPUT_FOLDER/usr/lib
cp libsegaapi.so $OUTPUT_FOLDER/usr/lib
cp lindbergh.so $OUTPUT_FOLDER/usr/lib
cp libposixtime.so $OUTPUT_FOLDER/usr/lib
cp lindbergh $OUTPUT_FOLDER/usr/bin
mkdir -p $OUTPUT_FOLDER/usr/lib/dri
cp /usr/lib/i386-linux-gnu/dri/libdril_dri.so $OUTPUT_FOLDER/usr/lib/dri/libdril_dri.so
cp /usr/share/vulkan/icd.d/* $OUTPUT_FOLDER/usr/share/vulkan/icd.d
for file in $OUTPUT_FOLDER/usr/share/vulkan/icd.d/*; do
  sed -i 's|/usr/lib/i386-linux-gnu/||' "$file"
done
cd ..

echo "Running linuxdeploy to create AppDir..."
./linuxdeploy-x86_64 --appimage-extract-and-run --appdir="$OUTPUT_FOLDER" ${ADDED_LIBS_ARGS[@]} \
--desktop-file="./scripts/lindbergh-loader.desktop" \
--icon-file="./scripts/lindbergh-loader.png" 

mv $OUTPUT_FOLDER/usr/lib/libkswapapi.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/libsegaapi.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/lindbergh.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/libposixtime.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/dri $OUTPUT_FOLDER/usr/lib32/
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/crocus_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/i915_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/iris_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/kms_swrast_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/nouveau_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/r300_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/r600_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/radeonsi_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/swrast_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/virtio_gpu_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/vmwgfx_dri.so
ln -s -f libdril_dri.so $OUTPUT_FOLDER/usr/lib32/dri/zink_dri.so


unzip libs/Cg-3.1.zip -d $OUTPUT_FOLDER/usr/lib32
cp libs/libCg.so $OUTPUT_FOLDER/usr/lib32/libCg2.so
cp libs/libopenal.so.0 $OUTPUT_FOLDER/usr/lib32/libopenal.so.0
cp libs/libcrypto.so.0.9.7 $OUTPUT_FOLDER/usr/lib32/libcrypto.so.0.9.7
cp libs/libssl.so.0.9.7 $OUTPUT_FOLDER/usr/lib32/libssl.so.0.9.7
tar -xf libs/lib32ID.tar -C $OUTPUT_FOLDER/usr/lib32ID
ln -s -f libposixtime.so $OUTPUT_FOLDER/usr/lib32/libposixtime.so.1
ln -s -f libposixtime.so $OUTPUT_FOLDER/usr/lib32/libposixtime.so.2.4
ln -s -f libkswapapi.so $OUTPUT_FOLDER/usr/lib32/libGLcore.so.1
ln -s -f libkswapapi.so $OUTPUT_FOLDER/usr/lib32/libnvidia-tls.so.1
ln -s -f libGLX.so $OUTPUT_FOLDER/usr/lib32/libGLX.so.0
rm -rf $OUTPUT_FOLDER/usr/share/doc

echo "Generating AppImage..."
rm -f "$APPIMAGENAME.AppImage"
./appimagetool-x86_64 -v "$OUTPUT_FOLDER" "$APPIMAGENAME.AppImage" --appimage-extract-and-run
