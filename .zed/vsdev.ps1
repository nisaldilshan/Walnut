# Runs commands inside the Visual Studio developer environment (MSVC, plus the CMake and
# Ninja that ship with Visual Studio), from the repository root. Used by .zed/tasks.json.
# Each argument is one command; they run in order and stop at the first failure.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File .zed/vsdev.ps1 "<command>" ["<command>" ...]
param([Parameter(Mandatory = $true, ValueFromRemainingArguments = $true)][string[]]$Commands)

$ErrorActionPreference = 'Stop'
$installer = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer"
$env:PATH = "$installer;$env:PATH"  # VsDevCmd looks up vswhere.exe on PATH
$vs = & "$installer\vswhere.exe" -latest -prerelease -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'No Visual Studio installation with the C++ build tools was found.' }

Import-Module "$vs\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
Set-Location (Split-Path -Parent $PSScriptRoot)

$ErrorActionPreference = 'Continue'
foreach ($command in $Commands) {
    Write-Host "> $command"
    Invoke-Expression $command
    if ($LASTEXITCODE) { exit $LASTEXITCODE }
}
