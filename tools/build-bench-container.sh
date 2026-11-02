#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
image="$(tr -d '\r\n' < tools/toolchain-image.txt)"
mkdir -p build-bench
exec docker run --rm --pull never --network none --cap-drop ALL \
    --security-opt no-new-privileges \
    --mount "type=bind,source=$PWD,target=/project,readonly" \
    --mount "type=bind,source=$PWD/build-bench,target=/output" \
    --workdir /tmp "$image" bash /project/tools/container-build-bench.sh
