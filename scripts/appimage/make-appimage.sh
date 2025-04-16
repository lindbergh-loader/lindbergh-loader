#!/usr/bin/env bash

APPIMAGENAME=lindbergh-loader
APPIMAGEDIRNAME=lindbergh-loader.AppDir

declare -a ADDED_LIBS=(
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
	"libncurses.so.5"
	"libpcsclite.so.1"
	"libstdc++.so.5"
	"libdrm.so.2"
	"libexpat.so.1"
	"libm.so.6"
	"libopenal.so.0"
	"libcrypto.so.0.9.7"
	"libssl.so.0.9.7"
 "libuuid.so.1"
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


echo "Copying files into AppDir..."
mkdir -p "$OUTPUT_FOLDER/usr/bin"
mkdir -p "$OUTPUT_FOLDER/usr/lib"
mkdir -p "$OUTPUT_FOLDER/usr/lib32"

# We create the AppRun file
cat > "$OUTPUT_FOLDER/AppRun" << 'EOF'
#!/bin/bash
APP_IMG_ROOT="$(dirname "$(readlink -f "$0")")"
export APP_IMG_ROOT=$APP_IMG_ROOT
export LD_LIBRARY_PATH="$APP_IMG_ROOT/usr/lib32:$LD_LIBRARY_PATH"
exec "$APP_IMG_ROOT/usr/bin/lindbergh" "$@"
EOF

chmod +x "$OUTPUT_FOLDER/AppRun"

cp libkswapapi.so $OUTPUT_FOLDER/usr/lib
cp libsegaapi.so $OUTPUT_FOLDER/usr/lib
cp lindbergh.so $OUTPUT_FOLDER/usr/lib
cp libposixtime.so $OUTPUT_FOLDER/usr/lib
mv lindbergh $OUTPUT_FOLDER/usr/bin
mkdir -p $OUTPUT_FOLDER/usr/lib/dri
cp /usr/lib/i386-linux-gnu/dri/* $OUTPUT_FOLDER/usr/lib/dri
cd ..

echo "Running linuxdeploy to create AppDir..."
./linuxdeploy-x86_64 --appimage-extract-and-run --appdir="$OUTPUT_FOLDER" ${ADDED_LIBS_ARGS[@]} \
--desktop-file="./scripts/lindbergh-loader.desktop" \
--icon-file="./scripts/lindbergh-loader.png" 

mv $OUTPUT_FOLDER/usr/lib/libkswapapi.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/libsegaapi.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/lindbergh.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/libposixtime.so $OUTPUT_FOLDER/usr/lib32
mv $OUTPUT_FOLDER/usr/lib/dri $OUTPUT_FOLDER/usr/lib32

unzip libs/Cg-3.1.zip -d $OUTPUT_FOLDER/usr/lib32
cp  libs/libCg.so $OUTPUT_FOLDER/usr/lib32/libCg2.so
ln -s -f libposixtime.so $OUTPUT_FOLDER/usr/lib32/libposixtime.so.1
ln -s -f libposixtime.so $OUTPUT_FOLDER/usr/lib32/libposixtime.so.2.4
ln -s -f libkswapapi.so $OUTPUT_FOLDER/usr/lib32/libGLcore.so.1
ln -s -f libkswapapi.so $OUTPUT_FOLDER/usr/lib32/libnvidia-tls.so.1

echo "Generating AppImage..."
rm -f "$APPIMAGENAME.AppImage"
./appimagetool-x86_64 -v "$OUTPUT_FOLDER" "$APPIMAGENAME.AppImage" --appimage-extract-and-run
