# Building Elmer

JUCE 8.0.14 is fetched automatically by CMake on every platform — there is no submodule and no
local JUCE checkout to manage. The first configure downloads and builds JUCE's tooling, so expect
it to take a few minutes; subsequent configures are fast.

Re-run the configure step whenever `CMakeLists.txt` changes (new sources, new `juce_add_plugin`
arguments). A plain rebuild will not pick those up.

## Requirements

| Platform | Needs |
|---|---|
| macOS | Xcode 14+ (Command Line Tools alone are not enough — JUCE needs the full toolchain for AU), CMake 3.24+ |
| Windows | Visual Studio 2022 with the Desktop C++ workload, CMake 3.24+ |
| Linux | GCC 11+ or Clang 14+, CMake 3.24+, and JUCE's dependency list below |

Linux packages:

```sh
sudo apt install libasound2-dev libjack-jackd2-dev ladspa-sdk \
  libcurl4-openssl-dev libfreetype6-dev libfontconfig1-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
  libxrandr-dev libxrender-dev libwebkit2gtk-4.1-dev libglu1-mesa-dev mesa-common-dev
```

## Configure and build

macOS (AU + VST3 + Standalone):

```sh
cmake -B build -G Xcode
cmake --build build --config Release
```

Windows (VST3 + Standalone). No explicit `-G`: pinning a Visual Studio generator version breaks
whenever the installed VS moves on.

```bat
cmake -B build -A x64
cmake --build build --config Release
```

Linux (VST3 + Standalone). A single-config generator, so the build type has to be set at configure
time rather than only at build time:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Built plugins are copied into place automatically on macOS
(`~/Library/Audio/Plug-Ins/VST3` and `.../Components`) and land in JUCE's own per-OS defaults
elsewhere — `%COMMONPROGRAMFILES%\VST3\` on Windows, `~/.vst3/` on Linux.

The universal macOS build produces arm64 + x86_64. If a CI-published package ever turns out to be
single-architecture, the cause is almost certainly `CMAKE_OSX_ARCHITECTURES` having drifted below
`project()` in `CMakeLists.txt`, where it is a silent no-op.

**After changing the icon artwork, re-run configure.** JUCE turns `ICON_BIG`/`ICON_SMALL` into
`AppIcon.icns` and the Windows `.ico` at *configure* time, and the PNGs are not configure
dependencies — so `cmake --build` alone happily rebuilds everything else and ships the previous
icon, with no warning and no changed file in the source tree. `cmake -B build ...` again, then
build. To confirm what actually shipped:

```sh
python3 - <<'EOF'
import struct
d = open('build/Elmer_artefacts/Release/Standalone/Elmer.app'
         '/Contents/Resources/AppIcon.icns', 'rb').read()
names = {'icp4':16,'icp5':32,'icp6':64,'ic07':128,'ic08':256,'ic09':512,'ic10':1024}
i = 8
while i < len(d):
    t, ln = struct.unpack('>4sI', d[i:i+8])
    print(names.get(t.decode('latin-1'), '?'), 'px', ln - 8, 'bytes')
    i += ln
EOF
```

Each entry is a PNG stored verbatim, so slicing `d[i+8:i+ln]` out to a file gives you the exact
image the bundle carries.

## Tests

```sh
./build/Tests/ElmerTests_artefacts/Release/ElmerTests          # macOS / Linux
build\Tests\ElmerTests_artefacts\Release\ElmerTests.exe        # Windows
```

The suite is JUCE's own `UnitTest` framework, not Catch2, and returns a non-zero exit code if
anything fails. It covers `GrainSpec`'s formulas against the design document, the Program bank's
structure, the Program save/delete/cancel contract, and the reverb engine — RT60 tracking,
stability at maximum settings, the four algorithms being measurably distinct, switch continuity,
and a CPU budget check.

The CPU test logs per-algorithm timings; on an M-series Mac all four sit around 0.3–0.5% of the
real-time budget at 48 kHz / 256 samples.

## Validation

macOS:

```sh
auval -v aufx Gl87 Nfdy

/Applications/pluginval.app/Contents/MacOS/pluginval \
    --strictness-level 8 \
    --validate ~/Library/Audio/Plug-Ins/VST3/Elmer.vst3
```

Windows:

```bat
pluginval.exe --strictness-level 8 --validate "%COMMONPROGRAMFILES%\VST3\Elmer.vst3"
```

Linux:

```sh
./pluginval --strictness-level 8 --validate ~/.vst3/Elmer.vst3
```

If Logic Pro does not pick up a freshly built AU: Audio Units Manager → "Reset & Rescan Selection",
or restart Logic. Note that a full `auval -a` instantiates every Audio Unit installed on the machine
and can take many minutes — validate by code (`auval -v aufx Gl87 Nfdy`) instead.

## DSP tuning

Every stage is real and complete, but the detector constants, the 6 dB soft-knee width, Iron's drive
curve and AUTO's two time constants are a structurally-reasoned first pass rather than one arrived
at by ear. The same is true of the sixteen factory Programs: their values were authored deliberately
against each Program's intent, but nothing has been listened to yet. Build, load, listen, adjust.

Keep these three cases in `Tests/CompressorTests.cpp` passing while you do — they guard the
architecture, not the tuning, and weakening one to make a change pass removes the thing it protects:

- **Stereo link** — one detector, bit-identical gain on both channels. A bus compressor that moves
  the image is not doing its job.
- **Realised ratio** — the measured ratio at each detent matches its printed mark. This is what
  keeps the panel honest.
- **AUTO adaptation** — recovery after a short burst is measurably faster than after a sustained
  tone. A fixed release rate, however well chosen, cannot pass it.
