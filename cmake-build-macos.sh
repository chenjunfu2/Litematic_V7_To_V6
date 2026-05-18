#!/bin/bash
set -e

if [ $# -eq 0 ]; then
    echo "Error: Missing architecture argument. Usage: $0 [x64|arm64]"
    exit 1
fi

ARCH="$1"

if [ "$ARCH" == "arm64" ]; then
    CMAKE_OSX_ARCHITECTURES="arm64"
elif [ "$ARCH" == "x64" ]; then
    CMAKE_OSX_ARCHITECTURES="x86_64"
else
    echo "Error: Unsupported architecture \"$ARCH\". Supported: x64, arm64."
    exit 1
fi

OUTPUT_DIR="artifacts/macos-$ARCH"

cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -g0" \
    -DCMAKE_OSX_ARCHITECTURES="$CMAKE_OSX_ARCHITECTURES" \
    -S .

cmake --build build

strip -Sx ./build/Litematic_V7_To_V6/Litematic_V7_To_V6
strip -Sx ./build/NBT_Compare/NBT_Compare
strip -Sx ./build/NBT_Print/NBT_Print

mkdir -p "$OUTPUT_DIR"
cp -f ./build/Litematic_V7_To_V6/Litematic_V7_To_V6 "$OUTPUT_DIR/"
cp -f ./build/NBT_Compare/NBT_Compare "$OUTPUT_DIR/"
cp -f ./build/NBT_Print/NBT_Print "$OUTPUT_DIR/"
