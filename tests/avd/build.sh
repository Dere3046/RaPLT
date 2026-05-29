#!/bin/bash
# Build RaPLT AVD test with NDK clang
set -e

if [ -z "$ANDROID_HOME" ]; then
    ANDROID_HOME=${ANDROID_SDK_ROOT:-/usr/local/lib/android/sdk}
fi
echo "ANDROID_HOME=$ANDROID_HOME"

NDK_VERSION=$(ls "$ANDROID_HOME/ndk/" 2>/dev/null | sort -V | tail -1)
if [ -z "$NDK_VERSION" ]; then
    echo "FAIL: no NDK found in $ANDROID_HOME/ndk/"
    exit 1
fi
NDK=$ANDROID_HOME/ndk/$NDK_VERSION
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
CC=$TOOLCHAIN/bin/x86_64-linux-android21-clang

RAPLT_SRC=$(cd "$(dirname "$0")/../../src" && pwd)
RAPLT_INC=$(cd "$(dirname "$0")/../../include" && pwd)
TEST_FILE=$(cd "$(dirname "$0")/jni" && pwd)/test_raplt.c
OUT=$(cd "$(dirname "$0")" && pwd)/libs/x86_64

mkdir -p "$OUT"

echo "==> RaPLT AVD test build"
echo "    OUT=$OUT"
echo "    CC=$CC"

# compile raplt objects
OBJS=""
for f in raplt_core raplt_elf raplt_hash raplt_signal raplt_util raplt_recon raplt_dlopen raplt_cfi; do
    echo "  CC  $f.c"
    $CC -c -o "$OUT/$f.o" "$RAPLT_SRC/$f.c" \
        -I"$RAPLT_INC" -std=gnu11 -fPIE -fvisibility=hidden \
        -Wno-unused-parameter -Wno-unused-function
    OBJS="$OBJS $OUT/$f.o"
done

# link test executable
echo "  LD  test_raplt"
$CC -o "$OUT/test_raplt" $OBJS "$TEST_FILE" \
    -I"$RAPLT_INC" -std=gnu11 -fPIE -pie -llog -ldl

echo "==> done: $(ls -lh "$OUT/test_raplt" | awk '{print $5, $NF}')"
file "$OUT/test_raplt"
