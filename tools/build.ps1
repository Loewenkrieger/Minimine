param([string]$Distribution = 'kali-linux')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$linuxProjectRoot = (& wsl.exe -d $Distribution -- wslpath -a -u $projectRoot)
if ($LASTEXITCODE -ne 0) { throw 'The project path could not be resolved in WSL.' }
& wsl.exe -d $Distribution -u root -- service docker start
if ($LASTEXITCODE -ne 0) { throw 'Docker Engine could not be started in WSL.' }
& wsl.exe -d $Distribution -u root --cd $linuxProjectRoot -- bash tools/build-container.sh
if ($LASTEXITCODE -ne 0) { throw 'ClassPad build failed.' }
