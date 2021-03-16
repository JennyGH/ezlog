#!/bin/bash

# =============== Set some local variables here. ===============
BUILD_DIR_NAME=build-linux

# Specify build type:
BUILD_TYPE=Release

# Specify install dir, binary files will be installed to here:
INSTALL_DIR=`pwd`/built/linux

# =================== Try to make directory. ===================
if [ ! -d $BUILD_DIR_NAME ]; then
    mkdir $BUILD_DIR_NAME
fi
# ==============================================================
cd $BUILD_DIR_NAME

cmake   -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
        ..
cmake --build . --config $BUILD_TYPE
make install

cd ..