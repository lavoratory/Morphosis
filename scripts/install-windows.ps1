[CmdletBinding()]
param(
    [string]$VcpkgRoot = $env:VCPKG_ROOT,
    [string]$BuildDir = "build-win",
    [string]$InstallDir = (Join-Path $env:USERPROFILE "Documents\Resolume Arena\Extra Effects"),
    [string]$Triplet = "x64-windows-static",
    [switch]$Clean,
    [switch]$SkipVcpkgBootstrap
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Require-Command {
    param([string]$Name)

    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Missing '$Name'. Install it, then run this script again."
    }
}

function Invoke-Checked {
    param(
        [string]$File,
        [string[]]$CommandArgs
    )

    & $File @CommandArgs
    if ($LASTEXITCODE -ne 0) {
        throw "'$File' failed with exit code $LASTEXITCODE."
    }
}

$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BuildPath = Join-Path $ProjectRoot $BuildDir

Write-Step "Checking prerequisites"
Require-Command "git"
Require-Command "cmake"

$VsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VsWhere) {
    $VisualStudioPath = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ([string]::IsNullOrWhiteSpace($VisualStudioPath)) {
        throw "Visual Studio 2022 C++ tools were not found. Install 'Desktop development with C++' or Visual Studio Build Tools, then run this again."
    }
} else {
    Write-Warning "vswhere.exe was not found. CMake will still try the Visual Studio 2022 generator."
}

if ([string]::IsNullOrWhiteSpace($VcpkgRoot)) {
    $VcpkgRoot = Join-Path $env:USERPROFILE "vcpkg"
}

if (-not (Test-Path $VcpkgRoot)) {
    Write-Step "Cloning vcpkg to $VcpkgRoot"
    Invoke-Checked "git" @("clone", "https://github.com/microsoft/vcpkg.git", $VcpkgRoot)
}

$BootstrapVcpkg = Join-Path $VcpkgRoot "bootstrap-vcpkg.bat"
$VcpkgExe = Join-Path $VcpkgRoot "vcpkg.exe"
$ToolchainFile = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"

if (-not (Test-Path $BootstrapVcpkg)) {
    throw "Could not find $BootstrapVcpkg. Set VCPKG_ROOT to an existing vcpkg checkout."
}

if ((-not (Test-Path $VcpkgExe)) -and (-not $SkipVcpkgBootstrap)) {
    Write-Step "Bootstrapping vcpkg"
    Invoke-Checked $BootstrapVcpkg @("-disableMetrics")
}

if (-not (Test-Path $VcpkgExe)) {
    throw "Could not find $VcpkgExe. Remove -SkipVcpkgBootstrap or bootstrap vcpkg manually."
}

if (-not (Test-Path $ToolchainFile)) {
    throw "Could not find $ToolchainFile."
}

$env:VCPKG_ROOT = $VcpkgRoot
$env:VCPKG_TARGET_TRIPLET = $Triplet

if ($Clean -and (Test-Path $BuildPath)) {
    Write-Step "Removing $BuildPath"
    Remove-Item -Recurse -Force $BuildPath
}

Write-Step "Configuring MorphosisSpray"
$ConfigureArgs = @(
    "-S", $ProjectRoot,
    "-B", $BuildPath,
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile",
    "-DVCPKG_TARGET_TRIPLET=$Triplet",
    "-DMORPHOSIS_RESOLUME_PLUGIN_DIR=$InstallDir"
)
Invoke-Checked "cmake" $ConfigureArgs

Write-Step "Building Release DLL"
$BuildArgs = @("--build", $BuildPath, "--config", "Release", "--parallel")
Invoke-Checked "cmake" $BuildArgs

Write-Step "Installing to Resolume Extra Effects"
New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
$InstallArgs = @("--install", $BuildPath, "--config", "Release")
Invoke-Checked "cmake" $InstallArgs

$InstalledDll = Join-Path $InstallDir "MorphosisSpray.dll"
if (-not (Test-Path $InstalledDll)) {
    throw "Build finished, but $InstalledDll was not found."
}

Write-Host ""
Write-Host "Installed MorphosisSpray:" -ForegroundColor Green
Write-Host "  $InstalledDll"
Write-Host ""
Write-Host "Restart Resolume Arena so it rescans FFGL plugins."
