param([string]$Distribution='kali-linux')
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$pin=Get-Content -LiteralPath '.\bench\azur-pin.txt'
$dependency=Join-Path $projectRoot '.cache\benchmark-azur'
if(-not (Test-Path -LiteralPath (Join-Path $dependency '.git'))) {
    & git -c credential.interactive=false clone --no-checkout $pin[0] $dependency
    if($LASTEXITCODE -ne 0) {throw 'Azur download failed.'}
    & git -C $dependency checkout --detach $pin[1]
    if($LASTEXITCODE -ne 0) {throw 'Cannot check out the pinned Azur commit.'}
}
$actual=& git -C $dependency rev-parse HEAD
if($actual -ne $pin[1]) {throw 'Azur checkout differs from bench/azur-pin.txt; no files changed.'}
$changes=& git -C $dependency status --porcelain
if($changes) {throw 'Azur dependency contains local changes; refusing a non-reproducible build.'}
$linuxRoot=& wsl.exe -d $Distribution -- wslpath -a -u $projectRoot
if($LASTEXITCODE -ne 0) {throw 'Cannot resolve the project path.'}
& wsl.exe -d $Distribution -u root -- service docker start
if($LASTEXITCODE -ne 0) {throw 'Cannot start Docker.'}
& wsl.exe -d $Distribution -u root --cd $linuxRoot -- bash tools/build-bench-container.sh
if($LASTEXITCODE -ne 0) {throw 'Benchmark build failed.'}
