#!/bin/bash
PROJECT_ROOT=`pwd`
BUILD_DIR=`pwd`/build-ios
BUILD_TYPE=Release
INSTALL_DIR=`pwd`/built/iOS

function do_compile(){
    if [ -d $BUILD_DIR ]; then
        rm -rf $BUILD_DIR
    fi
    mkdir $BUILD_DIR
    cd $BUILD_DIR

    cmake -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchains/ios.toolchain.cmake" \
          -DCMAKE_BUILD_TYPE=$BUILD_TYPE                                        \
          -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR                                   \
          -DPLATFORM=OS64                                                       \
          -DENABLE_CXX11=TRUE                                                   \
          -DCMAKE_VERBOSE_MAKEFILE=TRUE                                         \
          ..

    cmake --build   . --config $BUILD_TYPE &&
    cmake --install . --config $BUILD_TYPE
}

do_compile