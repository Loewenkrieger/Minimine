param([string]$Distribution = 'kali-linux')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$linuxProjectRoot = (& wsl.exe -d $Distribution -- wslpath -a -u $projectRoot)
if ($LASTEXITCODE -ne 0) { throw 'The project path could not be resolved in WSL.' }
& wsl.exe -d $Distribution --cd $linuxProjectRoot -- bash tools/test.sh
if ($LASTEXITCODE -ne 0) { throw 'Native verification failed.' }

