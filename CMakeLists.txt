cmake_minimum_required(VERSION 3.10)
project(Lindbergh)

# Set compiler and linker
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_LINKER g++)

# Define output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Source files for various targets
file(GLOB LIND_OBJS "src/lindbergh/*.c")
list(FILTER LIND_OBJS EXCLUDE REGEX "src/lindbergh/lindbergh.c")

file(GLOB XDIFF_SRCS "src/libxdiff/xdiff/*.c")

# Custom compiler and linker flags
set(LINDBERGH_EXE -m32)
set(LINDBERGH_C_FLAGS -g -fPIC -m32 -Wall -Werror -Wno-unused-variable -Wno-unused-function -D_GNU_SOURCE)
#SET(CMAKE_C_FLAGS_DEBUG -g)
SET(CMAKE_C_FLAGS_RELEASE -O3)
set(LINDBERGH_CXX_FLAGS ${LINDBERGH_C_FLAGS} -std=c++11)
set(LIBXDIFF_C_FLAGS -DHAVE_CONFIG_H -fPIC -m32)
set(LIBSEGAAPI_CXX_FLAGS -m32 -fPIC -shared)
set(LIBSEGAAPI_LINKER_FLAGS -m32 -lFAudio)
set(SHARED_LINKER_FLAGS -m32 -Wl,-z,defs -rdynamic -static-libstdc++ -static-libgcc -lc -ldl -lGL -lglut -lX11 -lXcursor -lSDL2 -lm -lpthread -shared -lstdc++ -nostdlib -lasound)

# Build executable for lindbergh
add_executable(lindyloader src/lindbergh/lindbergh.c src/lindbergh/evdevinput.c src/lindbergh/evdevinput.h src/lindbergh/log.c src/lindbergh/log.h src/lindbergh/jvs.c src/lindbergh/jvs.h src/lindbergh/config.h src/lindbergh/config.c)
set_target_properties(lindyloader PROPERTIES OUTPUT_NAME "lindbergh")
set_target_properties(lindyloader PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} LINK_FLAGS "-m32 -lm")
target_compile_options(lindyloader PRIVATE ${LINDBERGH_EXE})

# Build static library libxdiff.a with its own flags
add_library(xdiff STATIC ${XDIFF_SRCS})
set_target_properties(xdiff PROPERTIES OUTPUT_NAME xdiff ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} LINK_FLAGS "-m32")
target_compile_options(xdiff PRIVATE ${LIBXDIFF_C_FLAGS})

# Build shared library lindbergh.so
add_library(lindbergh SHARED ${LIND_OBJS})
set_target_properties(lindbergh PROPERTIES PREFIX "")
target_link_libraries(lindbergh xdiff ${SHARED_LINKER_FLAGS})
target_compile_options(lindbergh PRIVATE ${LINDBERGH_EXE})

# Build shared library libsegaapi.so
add_library(segaapi SHARED src/libsegaapi/libsegaapi.c)
target_link_libraries(segaapi FAudio ${LIBSEGAAPI_LINKER_FLAGS})
target_compile_options(segaapi PRIVATE ${LIBSEGAAPI_CXX_FLAGS})

# Build shared library libkswapapi.so
add_library(kswapapi SHARED src/libkswapapi/libkswapapi.c)
target_link_libraries(kswapapi ${SHARED_LINKER_FLAGS})
target_compile_options(kswapapi PRIVATE ${LINDBERGH_C_FLAGS})

# Build shared library libposixtime.so
add_library(posixtime SHARED src/libposixtime/libposixtime.c)
set_target_properties(posixtime PROPERTIES OUTPUT_NAME posixtime LINK_FLAGS "-m32")
add_custom_command(TARGET posixtime POST_BUILD
    COMMAND ln -s -f libposixtime.so ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libposixtime.so.1
    COMMAND ln -s -f libposixtime.so ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libposixtime.so.2.4)
target_compile_options(posixtime PRIVATE ${LINDBERGH_C_FLAGS})

# Custom clean rule
add_custom_target(cleanall
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/src/lindbergh/*.o
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/src/libsegaapi/*.o
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/src/libkswapapi/*.o
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/src/libxdiff/*.a
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/src/libxdiff/xdiff/*.o
)
