#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
mkdir -p build/native

compiler="${CC:-cc}"
flags=(-std=c11 -Wall -Wextra -Werror -Wshadow -Wconversion -pedantic -g -O1)
sanitizers=(-fsanitize=address,undefined -fno-omit-frame-pointer)

"$compiler" "${flags[@]}" "${sanitizers[@]}" -Isrc -Itests/host \
    src/game.c src/perf.c src/render.c tests/host/display.c tests/reference_world.c tests/reference_route.c tests/test_core.c \
    -o build/native/test_core
ASAN_OPTIONS=detect_leaks=1 ./build/native/test_core

"$compiler" "${flags[@]}" "${sanitizers[@]}" -Isrc \
    src/game.c tests/test_crafting.c -o build/native/test_crafting
ASAN_OPTIONS=detect_leaks=1 ./build/native/test_crafting

# The device entry point is syntax-checked against the small host adapter.
# This does not replace compiling/linking against the real ClassPad SDK.
"$compiler" "${flags[@]}" -Itests/host -Isrc -fsyntax-only src/main.c

"$compiler" "${flags[@]}" "${sanitizers[@]}" -Isrc -Itests/host \
    src/game.c src/perf.c src/render.c tests/host/display.c tests/test_loop.c \
    -o build/native/test_loop
ASAN_OPTIONS=detect_leaks=1 ./build/native/test_loop

"$compiler" "${flags[@]}" "${sanitizers[@]}" -Isrc -Itests/host \
    src/game.c src/perf.c src/render.c tests/host/display.c tests/preview.c \
    -o build/native/preview
ASAN_OPTIONS=detect_leaks=1 ./build/native/preview
if command -v python3 >/dev/null 2>&1; then
    python3 tools/preview.py
fi

