#!/bin/bash
PROJECT_ROOT=`pwd`
BUILD_DIR=$PROJECT_ROOT/build-android-release
BUILD_TYPE=Release
ANDROID_API_VERSION=23

DoCompile(){
    INSTALL_DIR=$PROJECT_ROOT/built/android/$ARCH
    if [ -d $BUILD_DIR ]; then
        rm -rf $BUILD_DIR
    fi
    mkdir $BUILD_DIR
    cd $BUILD_DIR
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
          -DANDROID_TOOLCHAIN=gcc                                                 \
          -DANDROID_ABI=$ARCH                                                     \
          -DANDROID_NDK=$ANDROID_NDK                                              \
          -DANDROID_PLATFORM=android-$ANDROID_API_VERSION                         \
          -DCMAKE_BUILD_TYPE=$BUILD_TYPE                                          \
          -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR                                     \
          -DENABLE_CXX11=TRUE                                                     \
          ..
    cmake --build . --config $BUILD_TYPE -- -j $(nproc) && make install
}

for ARCH in armeabi-v7a armeabi arm64-v8a # x86 x86_64 mips mips64
do
    DoCompile
done