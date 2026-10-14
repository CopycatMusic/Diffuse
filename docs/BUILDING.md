# Build CopyCat Diffuse from source

The source version is **1.0.1**. Requirements: CMake 3.22+, C++17 and JUCE 8.0.12.

```sh
git clone https://github.com/CopycatMusic/CopycatDiffuse.git
cd CopycatDiffuse
git clone --depth 1 --branch 8.0.12 https://github.com/juce-framework/JUCE.git JUCE
```

## macOS

Install Xcode command-line tools, then:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR="$PWD/JUCE"
cmake --build build --config Release --parallel 4
```

The default is a universal arm64+x86_64 build. Add
`-DCMAKE_OSX_ARCHITECTURES=arm64` or `x86_64` to select a single architecture.
Outputs: `build/CopycatDiffuse_artefacts/Release/AU/CopyCat Diffuse.component` and
`build/CopycatDiffuse_artefacts/Release/VST3/CopyCat Diffuse.vst3`.

## Windows

Use Visual Studio 2022 with the Desktop development with C++ workload:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_DIR="$PWD/JUCE"
cmake --build build --config Release --parallel 4
```

Output: `build/CopycatDiffuse_artefacts/Release/VST3/CopyCat Diffuse.vst3`.
The Windows CI also builds an installer using the tracked Inno Setup script.

## Installation

Builds do not install plugins by default. Copy the complete bundle manually to
`~/Library/Audio/Plug-Ins/Components/` (AU), `~/Library/Audio/Plug-Ins/VST3/`
(macOS VST3), or `C:\Program Files\Common Files\VST3\` (Windows VST3).
Alternatively, opt into JUCE's copy step with `-DCOPYCAT_COPY_PLUGIN_AFTER_BUILD=ON`.
Close and rescan your DAW after replacing installed bundles.

The CMake build requires no local signing or packaging helper. macOS signing
helpers and their personal configuration are kept locally and excluded from Git.
