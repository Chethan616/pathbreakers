#!/usr/bin/env bash
# Configure (first run only), build and launch the Operator Assistant app.
# Usage: bash run.sh   (from inside project/), or bash project/run.sh from the repo root.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-cli"

# Defaults match this machine's Qt install; override via env vars if yours differs,
# e.g. QT_PREFIX=D:/Qt/6.9.0/mingw_64 bash run.sh
QT_PREFIX="${QT_PREFIX:-C:/Qt/6.11.2/mingw_64}"
MINGW_BIN="${MINGW_BIN:-C:/Qt/Tools/mingw1310_64/bin}"
CMAKE_BIN="${CMAKE_BIN:-C:/Qt/Tools/CMake_64/bin/cmake.exe}"
NINJA_BIN="${NINJA_BIN:-C:/Qt/Tools/Ninja/ninja.exe}"

if [ ! -f "$CMAKE_BIN" ]; then
    echo "cmake.exe not found at $CMAKE_BIN - set CMAKE_BIN or edit run.sh." >&2
    exit 1
fi

# Windows resolves forward-slash paths fine, so no cygpath conversion needed here -
# this PATH is only consumed by the Windows processes we spawn below (cmake, the app).
export PATH="$MINGW_BIN:$QT_PREFIX/bin:$PATH"

if [ ! -d "$BUILD_DIR" ]; then
    echo "==> Configuring (first run)..."
    "$CMAKE_BIN" -S "$SCRIPT_DIR" -B "$BUILD_DIR" -G "Ninja" \
        -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
        -DCMAKE_C_COMPILER="$MINGW_BIN/gcc.exe" \
        -DCMAKE_CXX_COMPILER="$MINGW_BIN/g++.exe" \
        -DCMAKE_MAKE_PROGRAM="$NINJA_BIN"
fi

echo "==> Building..."
"$CMAKE_BIN" --build "$BUILD_DIR"

echo "==> Launching..."
nohup "$BUILD_DIR/appproject.exe" </dev/null >/dev/null 2>&1 &
disown
echo "Started."
