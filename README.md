# MorphosisSpray

MorphosisSpray is a native macOS FFGL source generator for Resolume Arena. It procedurally recreates the soft spray-paint color fields and granular pigment texture of `morph.jpeg` without loading the image at runtime.

Version 1.1 adds native Resolume HSBA color controls for the five pigment blobs: top, right, bottom, left, and glow.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release
```

If a universal build is not available on the current SDK/toolchain, build for Apple Silicon only:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release
```

## Install

```sh
cmake --install build --config Release
```

The bundle installs to:

```text
~/Documents/Resolume Arena/Extra Effects/MorphosisSpray.bundle
```

Restart Resolume Arena after installing so it rescans FFGL plugins.

## Windows

The same repo now builds a Windows FFGL `.dll` too. Use the Windows machine for the actual DLL build, but you do not need a separate project copy or a second Codex setup.

With vcpkg installed:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
cmake --install build --config Release
```

For Windows, install with the vcpkg static triplet if you want the FFGL DLL to stay self-contained:

```sh
set VCPKG_TARGET_TRIPLET=x64-windows-static
```

That installs to:

```text
%USERPROFILE%\Documents\Resolume\Extra Effects
```
