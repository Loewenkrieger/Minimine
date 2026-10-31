#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
if ! command -v fxsdk >/dev/null 2>&1; then
    printf '%s\n' 'fxSDK for ClassPad is missing. See README.md; no tools were installed.' >&2
    exit 2
fi
fxsdk build-cp
if [[ ! -s build-cp/CasioSandbox.hh3 ]]; then
    printf '%s\n' 'Build finished without build-cp/CasioSandbox.hh3. Check the fxSDK configuration.' >&2
    exit 3
fi
python3 tools/verify_hh3.py build-cp/CasioSandbox.hh3
printf '%s\n' 'Ready: build-cp/CasioSandbox.hh3'

