# Runs a command inside the Visual Studio developer environment (MSVC, plus the CMake and
# Ninja that ship with Visual Studio), from the repository root. Used by .zed/tasks.json.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File .zed/vsdev.ps1 cmake --build --preset conan-debug
#
# All arguments are joined into one command line, because Zed splits task arguments on spaces.
# No param() block on purpose: arguments such as -DFOO=ON must reach $args, not PowerShell's binder.

$ErrorActionPreference = 'Stop'
$command = $args -join ' '
if (-not $command) { throw 'Usage: vsdev.ps1 <command> [arguments...]' }

$installer = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer"
$env:PATH = "$installer;$env:PATH"  # VsDevCmd looks up vswhere.exe on PATH
$vs = & "$installer\vswhere.exe" -latest -prerelease -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'No Visual Studio installation with the C++ build tools was found.' }

Import-Module "$vs\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
Set-Location (Split-Path -Parent $PSScriptRoot)

Write-Host "> $command"
$global:LASTEXITCODE = 0
try {
    Invoke-Expression $command
} catch {
    Write-Host $_ -ForegroundColor Red
    exit 1
}
exit $LASTEXITCODE
