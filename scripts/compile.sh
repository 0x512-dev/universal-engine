#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_DIR"

command -v clang++ >/dev/null || { echo "clang++ not found. Run: pkg install clang"; exit 1; }
command -v pkg-config >/dev/null || { echo "pkg-config not found. Run: pkg install pkg-config"; exit 1; }
pkg-config --exists sdl2 || { echo "SDL2 not found. Run: pkg install sdl2"; exit 1; }
[ -f "$PREFIX/include/vulkan/vulkan.h" ] || [ -f /usr/include/vulkan/vulkan.h ] || {
    echo "Vulkan headers not found. Run: pkg install vulkan-headers"; exit 1;
}

mkdir -p build
mapfile -t SOURCES < <(find src -type f -name '*.cpp' -print | sort)
SOURCES=(main.cpp "${SOURCES[@]}")

echo "Compiling Universal Engine..."
clang++ -std=c++17 -O2 -Wall -Wextra \
    -Iinclude -Isrc \
    "${SOURCES[@]}" \
    -o build/universal_engine \
    $(pkg-config --cflags --libs sdl2) \
    -lvulkan

echo "Build successful: $PROJECT_DIR/build/universal_engine"
