#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
image="$(tr -d '\r\n' < tools/toolchain-image.txt)"
if ! command -v docker >/dev/null 2>&1; then
    printf '%s\n' 'Docker Engine is missing. See README.md.' >&2
    exit 2
fi
if ! docker info >/dev/null 2>&1; then
    printf '%s\n' 'Docker Engine is not running or access is unavailable.' >&2
    printf '%s\n' 'In the configured WSL distribution: sudo service docker start' >&2
    exit 2
fi
if ! docker image inspect "$image" >/dev/null 2>&1; then
    printf 'The pinned toolchain image is missing: %s\n' "$image" >&2
    printf '%s\n' 'No download or installation was started.' >&2
    exit 2
fi
mkdir -p build-cp
exec docker run --rm --pull never --network none --cap-drop ALL \
    --security-opt no-new-privileges \
    --mount "type=bind,source=$PWD,target=/project,readonly" \
    --mount "type=bind,source=$PWD/build-cp,target=/output" \
    --workdir /tmp "$image" bash /project/tools/container-build.sh


