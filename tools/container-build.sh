#!/usr/bin/env bash
set -euo pipefail

# Work on the Linux filesystem: CMake's permission changes do not work for an
# unrelated container UID on a Windows/DrvFS bind mount. Only export file data.
build_root=/tmp/casio-sandbox
mkdir -p "$build_root/tools"
cp -R /project/src "$build_root/src"
cp /project/CMakeLists.txt "$build_root/CMakeLists.txt"
for helper in build.sh verify_hh3.py toolchain-image.txt; do
    cp "/project/tools/$helper" "$build_root/tools/$helper"
done
cd "$build_root"
bash tools/build.sh
sh-elf-readelf -h -l -n build-cp/CasioSandbox.hh3 > build-cp/elf-info.txt
sh-elf-size -A build-cp/casio_sandbox > build-cp/section-sizes.txt

# No chmod/chown on the Windows output folder; the source mount is read-only.
for artifact in casio_sandbox CasioSandbox.map build-info.json elf-info.txt section-sizes.txt CasioSandbox.hh3; do
    cat "build-cp/$artifact" > "/output/.$artifact.tmp"
    mv "/output/.$artifact.tmp" "/output/$artifact"
done
printf '%s\n' 'Verified outputs exported to the project build-cp directory.'
