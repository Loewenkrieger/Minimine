#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
mkdir -p build/native
compiler="${CC:-cc}"
flags=(-std=c11 -Wall -Wextra -Werror -Wshadow -Wconversion -pedantic -g -O1)
sanitizers=(-fsanitize=address,undefined -fno-omit-frame-pointer)
core=(src/game.c src/items.c src/inventory.c src/inventory_ui.c src/adventure.c src/boss.c src/survival.c src/validate.c src/storage.c)
view=(src/perf.c src/render.c tests/host/display.c)
suites=("$@")
if ((${#suites[@]} == 0)); then suites=(core crafting survival storage touch adventure loop); fi
for suite in "${suites[@]}"; do
    extra=()
    case "$suite" in
        core) extra=("${view[@]}" tests/reference_world.c tests/reference_route.c);;
        loop) extra=("${view[@]}");;
    esac
    "$compiler" "${flags[@]}" "${sanitizers[@]}" -Isrc -Itests/host \
        "${core[@]}" "${extra[@]}" "tests/test_$suite.c" -o "build/native/test_$suite"
    ASAN_OPTIONS=detect_leaks=1 "./build/native/test_$suite"
done
if (($#)); then exit 0; fi
"$compiler" "${flags[@]}" -Itests/host -Isrc -fsyntax-only src/main.c
"$compiler" "${flags[@]}" "${sanitizers[@]}" -Isrc -Itests/host \
    "${core[@]}" "${view[@]}" tests/preview.c -o build/native/preview
ASAN_OPTIONS=detect_leaks=1 ./build/native/preview
if command -v python3 >/dev/null 2>&1; then python3 tools/preview.py; fi
