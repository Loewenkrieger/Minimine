#!/usr/bin/env bash
set -euo pipefail
mkdir -p /tmp/minimine/bench /tmp/minimine/src /tmp/minimine/benchmarks/1.0.0
cp -R /project/src/. /tmp/minimine/src/
cp -R /project/bench/. /tmp/minimine/bench/
cp /project/benchmarks/1.0.0/render-reference.c /tmp/minimine/benchmarks/1.0.0/
cp -R /project/.cache/benchmark-azur /tmp/azur
python3 /project/tools/prepare_bench.py /tmp/azur
cd /tmp/minimine/bench
fxsdk build-cp -c -DAZUR_SOURCE=/tmp/azur \
    "-DBENCH_SOURCE_SHA=$(python3 /project/tools/bench_source_id.py /project)" \
    "-DBENCH_AZUR_SHA=$(sed -n '2p' /project/bench/azur-pin.txt | tr -d '\r')" \
    "-DBENCH_TOOLCHAIN=$(tr -d '\r\n' </project/tools/toolchain-image.txt)" \
    "-DBENCH_BUILD_UTC=$(date -u +%FT%TZ)"
fxsdk build-cp
sh-elf-readelf -h -l -n build-cp/MinimineBench.hh3 >build-cp/elf-info.txt
sh-elf-size -A build-cp/minimine_bench >build-cp/section-sizes.txt
python3 /project/tools/verify_bench.py build-cp/MinimineBench.hh3 /project
for file in MinimineBench.hh3 minimine_bench MinimineBench.map elf-info.txt section-sizes.txt build-info.json; do
    cat "build-cp/$file" >"/output/.$file.tmp"
    mv "/output/.$file.tmp" "/output/$file"
done
