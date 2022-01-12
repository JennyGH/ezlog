#!/bin/bash

# =============== Configurations of vcpkg ===============
VCPKG_TARGET_TRIPLET=x64-linux
CMAKE_VCPKG_OPTIONS=
if $(command -v vcpkg  >/dev/null 2>&1); then
  CMAKE_VCPKG_OPTIONS=-DVCPKG_TARGET_TRIPLET=$VCPKG_TARGET_TRIPLET -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
  vcpkg install gtest $VCPKG_TARGET_TRIPLET
fi
# =======================================================

# =============== Set some local variables here. ===============
# You can change the name of build directory here:
BUILD_DIR_NAME=build-linux

# Save project root path.
PROJECT_ROOT=`pwd`

# Get processor name.
PROCESSOR=`uname -p`

function build() {
    # Specify build type:
    BUILD_TYPE=$1

    # Specify install dir, binary files will be installed to here:
    INSTALL_DIR=$PROJECT_ROOT/built/Linux/$PROCESSOR

    # =================== Try to make directory. ===================
    if [ ! -d $BUILD_DIR_NAME ]; then
        mkdir $BUILD_DIR_NAME
    fi
    # ==============================================================
    cd $BUILD_DIR_NAME

    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE        \
          -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
          -DCMAKE_VERBOSE_MAKEFILE=TRUE         \
          -DCMAKE_C_COMPILER=/usr/bin/gcc       \
          -DCMAKE_CXX_COMPILER=/usr/bin/g++     \
          -DBUILD_SHARED_LIBS=FALSE             \
          -DBUILD_TEST=TRUE                     \
          -DBUILD_BENCHMARK=TRUE                \
          $CMAKE_VCPKG_OPTIONS                  \
          ..
    cmake --build . --config $BUILD_TYPE
    make install
    cd "$PROJECT_ROOT"
}

build Release