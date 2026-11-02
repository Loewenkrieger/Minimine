#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
mkdir -p build/native/bench-logs build/native/azur
cp -R .cache/benchmark-azur/azur build/native/azur/
python3 tools/prepare_bench.py build/native/azur
python3 - <<'PY'
from pathlib import Path
p=Path('build/native/azur/azur/include/azur/config.h.in')
s=p.read_text().replace('@Azur_VERSION@','0.3').replace('@Azur_VERSION_MAJOR@','0').replace('@Azur_VERSION_MINOR@','3')
s='\n'.join(line.replace('#cmakedefine01','#define')+' 0' if line.startswith('#cmakedefine01') else line for line in s.splitlines())
p.with_name('config.h').write_text(s+'\n')
Path('build/native/build_id.h').write_text('#define BENCH_SOURCE_SHA "host-test"\n#define BENCH_AZUR_SHA "70e92c47f5d35c6d6b1a0dde73c8346966b8d7a6"\n#define BENCH_TOOLCHAIN "host-test"\n#define BENCH_BUILD_UTC "host-test"\n')
PY
flags=(-std=c11 -Wall -Wextra -Wshadow -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
    -ffunction-sections -fdata-sections -DBENCH_HOST -DBENCH_IO_TEST -DGINT_HW_CP=1 -DMINIMINE_BENCH_LIGHT
    '-DBENCH_LOG_ROOT="build/native/bench-logs"'
    -Ibench/host -Itests/host -Isrc -Ibench -Ibuild/native -Ibuild/native/azur/azur/include)
core=(src/game.c src/items.c src/inventory.c src/inventory_ui.c src/adventure.c src/boss.c src/survival.c src/validate.c src/storage.c src/perf.c src/render.c)
cc "${flags[@]}" "${core[@]}" bench/scenes.c bench/graphics.c bench/log.c tests/host/display.c \
    build/native/azur/azur/src/gint/render.c bench/test_bench.c -Wl,--gc-sections -o build/native/test_bench
ASAN_OPTIONS=detect_leaks=1 ./build/native/test_bench
cc "${flags[@]}" "${core[@]}" bench/scenes.c bench/graphics.c bench/log.c tests/host/display.c \
    build/native/azur/azur/src/gint/render.c bench/test_loop.c -Wl,--gc-sections -o build/native/test_bench_loop
ASAN_OPTIONS=detect_leaks=1 ./build/native/test_bench_loop
