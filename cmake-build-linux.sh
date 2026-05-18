#!/bin/bash
set -e

if [ $# -eq 0 ]; then
    echo "Error: Missing architecture argument. Usage: $0 [x64|arm64]"
    exit 1
fi

ARCH="$1"

if [ "$ARCH" == "arm64" ]; then
    export CC=aarch64-linux-gnu-gcc
    export CXX=aarch64-linux-gnu-g++
    STRIP="aarch64-linux-gnu-strip"
    CMAKE_EXTRA_FLAGS="-DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64"
elif [ "$ARCH" == "x64" ]; then
    export CC=gcc
    export CXX=g++
    STRIP="strip"
    CMAKE_EXTRA_FLAGS=""
else
    echo "Unsupported architecture: $ARCH. Use x64 or arm64."
    exit 1
fi

OUTPUT_DIR="artifacts/linux-$ARCH"

cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -g0" \
    $CMAKE_EXTRA_FLAGS \
    -S .

cmake --build build

$STRIP --strip-all ./build/Litematic_V7_To_V6/Litematic_V7_To_V6
$STRIP --strip-all ./build/Litematic_V7_To_V6/NBT_Compare
$STRIP --strip-all ./build/Litematic_V7_To_V6/NBT_Print

mkdir -p "$OUTPUT_DIR"
cp -f ./build/Litematic_V7_To_V6/Litematic_V7_To_V6 "$OUTPUT_DIR/"
cp -f ./build/Litematic_V7_To_V6/NBT_Compare "$OUTPUT_DIR/"
cp -f ./build/Litematic_V7_To_V6/NBT_Print "$OUTPUT_DIR/"
