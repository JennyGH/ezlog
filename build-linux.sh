#!/bin/bash

# =============== Set some local variables here. ===============
BUILD_DIR_NAME=build-linux

# =================== Try to make directory. ===================
if [ ! -d $BUILD_DIR_NAME ]; then
    mkdir $BUILD_DIR_NAME
fi
# ==============================================================
cd $BUILD_DIR_NAME

cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug --target INSTALL

cd ..