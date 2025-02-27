CC ?= gcc
CFLAGS = -g -fPIC -m32 -pthread -Wall -Werror -Wno-misleading-indentation -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -D_GNU_SOURCE -Wno-char-subscripts
CXX ?= g++
LD = $(CXX) 
LDFLAGS = -m32 -Wl,-z,defs -rdynamic -static-libgcc -lc -ldl -lGL -lglut -lX11 -lXcursor -lSDL2 -lm -lpthread -shared -nostdlib -lasound -L./src/libxdiff -lxdiff -L/app/lib32

BUILD = build

XDIFF_SRCS = xdiffi.c xprepare.c xpatchi.c xmerge3.c xemit.c xmissing.c xutils.c \
             xadler32.c xbdiff.c xbpatchi.c xversion.c xalloc.c xrabdiff.c

XDIFF_OBJS = $(patsubst %.c,src/libxdiff/xdiff/%.o,$(XDIFF_SRCS))

OBJS := $(patsubst %.c,%.o,$(wildcard src/lindbergh/*.c))
OBJS := $(filter-out src/lindbergh/lindbergh.o, $(OBJS))

all: lindbergh libxdiff.a lindbergh.so libsegaapi.so libkswapapi.so libposixtime.so

lindbergh: src/lindbergh/lindbergh.c src/lindbergh/log.c src/lindbergh/log.h src/lindbergh/jvs.c src/lindbergh/jvs.h src/lindbergh/config.h src/lindbergh/config.c src/lindbergh/evdevinput.h src/lindbergh/evdevinput.c
	mkdir -p $(BUILD)
	$(CC) -m32 src/lindbergh/lindbergh.c src/lindbergh/log.h src/lindbergh/log.c src/lindbergh/jvs.h src/lindbergh/jvs.c src/lindbergh/config.h src/lindbergh/config.c src/lindbergh/evdevinput.c src/lindbergh/evdevinput.h -o $(BUILD)/lindbergh -lm

libxdiff.a: $(XDIFF_OBJS)
	mkdir -p $(BUILD)
	ar rcs src/libxdiff/libxdiff.a $(XDIFF_OBJS)

src/libxdiff/xdiff/%.o: src/libxdiff/xdiff/%.c
	$(CC) -m32 -DHAVE_CONFIG_H -fPIC -c $< -o $@

lindbergh.so: $(OBJS)
	mkdir -p $(BUILD)
	$(LD) $(OBJS) $(LDFLAGS) -o $(BUILD)/lindbergh.so

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

libsegaapi.so: src/libsegaapi/libsegaapi.o
	$(CC) -m32 -O0 -g src/libsegaapi/libsegaapi.c -lFAudio -L/app/lib32 -fPIC -shared -o $(BUILD)/libsegaapi.so

libkswapapi.so: src/libkswapapi/libkswapapi.o
	$(CC) -m32 src/libkswapapi/libkswapapi.o -fPIC -shared -o $(BUILD)/libkswapapi.so

libposixtime.so:
	$(CC) -m32 src/libposixtime/libposixtime.c src/libposixtime/libposixtime.h -m32 -shared -o $(BUILD)/libposixtime.so
	ln -s -f libposixtime.so $(BUILD)/libposixtime.so.1
	ln -s -f libposixtime.so $(BUILD)/libposixtime.so.2.4

# Clean rule
clean:
	rm -f $(BUILD)/lindbergh.so
	rm -f $(BUILD)/libsegaapi.so
	rm -f $(BUILD)/lindbergh
	rm -f src/lindbergh/*.o
	rm -f src/libsegaapi/*.o

#clean all rule
cleanall:
	rm -rf $(BUILD)
	rm -f src/lindbergh/*.o
	rm -f src/libsegaapi/*.o
	rm -f src/libkswapapi/*.o
	rm -f src/libxdiff/*.a
	rm -f src/libxdiff/xdiff/*.o
