# Elmer

**Bus compressor · Model GL-87 · Stereo**

A stereo-linked VCA bus compressor — the glue across a mix bus, a drum bus, or a stem. The sixth and
final casting in the [Neon Foundry](../BRAND.md) suite.

One detector drives both channels, so the stereo image cannot shift. A sidechain high-pass keeps the
low end from ducking everything above it. Iron adds output-transformer saturation after the gain
reduction, and Mix blends the compressed signal back under the dry for parallel work.

The panel is a console channel module: warm grey fascia, printed scales around every knob,
colour-coded caps, and a large analog moving-coil gain-reduction meter.

## Controls

| Section | Control | Range |
|---|---|---|
| Detection | Threshold | −40 … +10 dB |
| | Ratio | 1.5:1 · 2:1 · 4:1 · 10:1 · 20:1 |
| | Knee | Soft / Hard |
| | Sidechain HP | OFF, 40 … 500 Hz |
| Timing | Attack | 0.1 … 30 ms |
| | Release | 0.1 s · 0.3 s · 0.6 s · 1.2 s · AUTO |
| Character | Iron | 0 … 100 % |
| Output | Makeup | 0 … +20 dB |
| | Mix | 0 … 100 % |

**AUTO** is the fifth position on the Release switch, not a separate button. It is a
program-dependent dual time constant: brief transients recover fast while sustained compression
releases slowly, so it adapts to the material rather than running at a fixed rate.

## Programs

16 factory Programs. Stored snapshots are **Programs**, never "presets". SAVE always creates a new
one and never overwrites; DELETE only works on your own.

## Formats

AU, VST3 and Standalone on macOS (universal arm64 + x86_64); VST3 and Standalone on Windows and
Linux. See [BUILDING.md](BUILDING.md).
