cd libs
find /usr/lib/i386-linux-gnu -name "libSDL3*" -exec sh -c 'tar -rf libs.tar -C /usr/lib/i386-linux-gnu $(basename {})' \;
find /usr/lib/i386-linux-gnu -name "libFAudio*" -exec sh -c 'tar -rf libs.tar -C /usr/lib/i386-linux-gnu $(basename {})' \;
tar -cf includes.tar -C /usr/include SDL3 SDL3_image SDL3_ttf FAudio.h FAudioFX.h

cd ..