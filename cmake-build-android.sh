#!/bin/bash
set -e

if [ $# -eq 0 ]; then
    echo "Error: Missing architecture argument. Usage: $0 [x64|arm64]"
    exit 1
fi

ARCH="$1"

if [ "$ARCH" == "arm64" ]; then
    ANDROID_ABI="arm64-v8a"
elif [ "$ARCH" == "x64" ]; then
    ANDROID_ABI="x86_64"
else
    echo "Unsupported architecture: $ARCH. Use x64 or arm64."
    exit 1
fi

OUTPUT_DIR="artifacts/android-$ARCH"

ANDROID_PLATFORM=${ANDROID_PLATFORM:-android-21}
BUILD_TYPE=${BUILD_TYPE:-Release}

cmake -B build-android \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ANDROID_ABI" \
    -DANDROID_PLATFORM="$ANDROID_PLATFORM" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -g0" \
    -S .

cmake --build build-android --config "$BUILD_TYPE" -j $(nproc)

$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip --strip-all ./build-android/Litematic_V7_To_V6/Litematic_V7_To_V6

mkdir -p "$OUTPUT_DIR"
cp -f ./build-android/Litematic_V7_To_V6/Litematic_V7_To_V6 "$OUTPUT_DIR/"
