# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

Elmer is its own independent repo and does not depend on `../taperot/`, `../gatecrasher/`,
`../chorus-60/`, `../reflect-84/` or `../fifth-member/` at runtime or at build time — it is a
sibling casting under the shared [Neon Foundry](../BRAND.md) umbrella, and those repos are read
purely as structural reference. Read `../BRAND.md` first for the cross-plugin design system, then
this file.

`design/README.md` is the authoritative GUI spec and `design/Elmer.dc.html` is the live prototype.
`design/screenshots/panel.png` is the panel rendered at 2× and is the acceptance target — this
casting ships one, so there is no need to drive the prototype in a browser.

**Note that `design/BRAND.md` is a stale copy** — the pre-update revision, with no Legibility or
Parameter-values section. The root `../BRAND.md` governs.

## Commands

Elmer builds on macOS (AU + VST3 + Standalone), Windows (VST3 + Standalone) and Linux (VST3 +
Standalone). JUCE 8.0.14 is fetched automatically via CMake `FetchContent`.

Configure once — macOS: `cmake -B build -G Xcode`. Windows: `cmake -B build -A x64`. Linux:
`cmake -B build -DCMAKE_BUILD_TYPE=Release`. Re-run configure whenever `CMakeLists.txt` changes,
**and whenever the icon artwork changes** — JUCE builds the `.icns` and `.ico` at configure time and
the PNGs are not configure dependencies, so a plain rebuild ships the previous icon silently.

Build: `cmake --build build --config Release`. Tests:
`./build/Tests/ElmerTests_artefacts/Release/ElmerTests`.

## Prompts log

`prompts/PROMPTS.md` holds numbered work-package prompts. Mark one `SHIPPED` with the date once it
is fully implemented.

## Architecture

### Signal chain (fixed order, all in `PluginProcessor::processBlock`)

```
in -+---------------------------- dry ------------------------------+
    |                                                                |
    +-> SidechainFilter -> LevelDetector -> gain reduction (dB)      |
    |      (detector only)     (ONE detector, stereo-linked)         |
    +-> x gain <-----------------------------------------------------+
           |
           +-> IronStage -> [makeup] -> OutputStage parallel mix -> out
```

### The five things most likely to be got wrong

1. **One detector, not two.** It is fed a mono sum of both channels and produces one gain-reduction
   value applied identically to left and right. That is the premise of a bus compressor: the stereo
   image cannot shift because there is nothing to shift it. There is no link control and no second
   meter. `Tests/CompressorTests.cpp`'s `StereoLinkTests` asserts bit-identical gain on both
   channels from a hard-panned input.

2. **Feed-forward, not feedback.** Gain is computed from the *input* level. This is what makes the
   printed RATIO marks tell the truth — a feedback topology's effective ratio drifts with level, so
   a panel marked 4:1 would only be 4:1 at one input level. `GainComputerTests` measures the
   realised ratio at every detent.

3. **Ballistics are applied to the gain reduction in dB, not to the input envelope.** That is what
   makes ATTACK and RELEASE mean what the panel prints; smoothing the envelope first makes the
   realised times depend on how far over threshold the signal sits.

4. **AUTO release is two real time constants, not an averaged rate.** A fast envelope handles
   transients, a slow one accumulates while the signal stays over threshold, and the effective
   release crossfades between them. `AutoReleaseTests` asserts recovery after a 20 ms burst is
   substantially faster than after a 2 s tone — which no fixed rate can pass.

5. **Mix is a true parallel blend and there is no lookahead anywhere.** The wet path is
   sample-aligned with the dry by construction, so blending cannot comb. If lookahead is ever added,
   the dry tap must be delayed to match.

**A gain-staging trap.** `IronStage` uses `tanh(x*d)/d`, not `tanh(x*d)/tanh(d)`. The normalised
form is unity only at *full scale*; below it its gain is `d/tanh(d)`. Fifth Member shipped that bug
and a feedback loop ran away.

### Parameters

`Source/Parameters.h` is the single source of truth. Two things there are load-bearing:

- **SIDECHAIN HP stores the knob POSITION, not a frequency**, and that is deliberate. The control
  has a dead zone — its first tenth is OFF, its next tenth clamps to 40 Hz — so a frequency-valued
  parameter could not represent where the pointer actually is. Every other continuous control stores
  real units; this one cannot.
- **ATTACK uses explicit `NormalisableRange` conversion lambdas** implementing `0.1 × 300^f`, not a
  fitted skew. BRAND.md makes the printed scale a correctness requirement, and a skew fitted to the
  endpoints is right at 0 and 1 and wrong everywhere between — exactly the failure the rule exists
  to catch.

`PrintedScale` carries every value the panel prints and `Tests/PrintedScaleTests.cpp` asserts the
mapping at every mark. Two marks legitimately diverge from their law and are pinned with a
documented tolerance: ATTACK's printed `0.1/0.3/1/3/10/30` is the conventionally rounded form of the
exact series `0.1/0.313/0.979/3.06/9.58/30`, and SIDECHAIN HP's 0.6 mark is exactly 141.4 Hz under a
printed "140". Both are how real panels are marked.

### Programs

Gatecrasher's architecture reused directly: Factory/User banks, one `.elmerprogram` XML per User
Program, SAVE always creates new and never overwrites, DELETE gated to User Programs at both the
button and the model, `AsyncUpdater` for the audio-thread-safe apply, one FACT/USER field inside the
LCD. Reflect-84's two improvements are carried: name collisions use `getNonexistentSibling()`, and
the schema version is read on restore, not merely written.

**Every Program stores all nine parameters.** Elmer has no mutually exclusive selectors, so Fifth
Member's active-path filtering and its zero-fill invariant are deliberately **not** ported — a zero
in `FactoryPrograms.h` is a real zero, not an absent field. Do not add that machinery here.

### GUI

Bitmap-composited over a code-drawn fascia. There is deliberately **no dressed-panel render** in
BinaryData: Gatecrasher shipped one as its background and spent a release chasing bugs caused by
every live control sitting over a baked copy of itself. `design/screenshots/panel.png` is an
acceptance target, not a runtime asset.

`PanelBackground` bakes at **2×**. The fascia's grain is a 4 px repeating gradient with 1 px
stripes; blitted 1:1 to a Retina display it resolves to a flat wash.

**Printed legends are a transcribed table, never a formula.** `ElmerTheme.h` carries the literal
`left`/`top` offsets from the prototype. Most knobs are formula-clean — THRESHOLD sits at a constant
62 px radius, SIDECHAIN HP at 54 — but RATIO's run 54 px at the top, 59.5 at the sides and 60.3 at
the bottom because its wider labels were pushed out by eye. RELEASE's `0.6s` sits at `top=-8` where
RATIO's `4:1` sits at `-10` on an identical ring; IRON's `25` is at `left=-21` where RATIO's `2:1`
is at `-26`. A formula erases every one of those.

**The selected KNEE dot is the only lit indicator on the entire panel.** There are no other LEDs.
AUTO is a position on the RELEASE switch, not a button, so it needs no lamp — the pointer is the
state. No knob ever dims, greys out or goes inert.

**Live values take the LCD over, not a tooltip.** Grabbing a control replaces the program name with
that parameter's name and value, reverting 1200 ms after release. This is `design/README.md`'s
explicit choice over BRAND.md's tooltip convention — it reuses a display already on the panel, a
tooltip has no hardware equivalent, and it satisfies BRAND.md's stronger rule that dynamic text
lives inside a screen. The takeover fires only on a **grab**: a `SliderAttachment` raises
`onValueChange` when a Program is applied and when the host automates, and without guarding on the
drag state the display latches onto whichever parameter was written last.

Two more traps already paid for:

- The prototype's `ratLabel()` ignores its argument and returns the same style for every detent. The
  RATIO legend highlight is **dead code** — legends are static. (Fifth Member had the same thing in
  an unbound `op: 0.32`.)
- The needle transform is written as an explicit four-step chain — translate the sprite's pivot to
  the origin, scale, rotate, translate onto the meter's pivot — rather than
  `scale().translated().rotated()`. Composition order on the compact form is easy to get subtly
  wrong, and the failure looks like a needle on the correct axis at the wrong place along it.

### Build system

JUCE pinned to `8.0.14`, matching all five siblings. `PLUGIN_MANUFACTURER_CODE` (`Nfdy`),
`PLUGIN_CODE` (`Gl87`), `BUNDLE_ID` (`com.neonfoundry.elmer`) and `COMPANY_NAME` are settled —
changing them breaks saved projects in both AU and VST3.

`Tests/` compiles the DSP `.cpp` files directly — **a new DSP `.cpp` goes in both target_sources
lists**. `TestMain.cpp` creates a `ScopedJuceInitialiser_GUI`: without a MessageManager,
`AsyncUpdater::triggerAsyncUpdate()` silently clears its own pending flag and every Program test
passes while proving nothing.

## Status

- **DSP**: complete, no stubs. Detector constants, the knee width, Iron's drive curve and the AUTO
  time constants are a structurally-reasoned first pass, not a by-ear one.
- **Programs**: 16 factory Programs with authored values; no by-ear pass.
- **GUI**: complete. Geometry verified against `design/screenshots/panel.png` — all eight knob
  centres land exactly, and the LCD, IN box, meter face and section-box edges are within 1 px.
- **Outstanding**: a mid-tone comparison against the reference render shows this build reading
  ~10–15 levels darker across the right and lower panel. The cause is **not yet isolated** — the
  capture pipeline used for the comparison is demonstrably not colour-faithful (the scribble tape is
  drawn `#EFE9D6` and captures as `#F5EFD4`), so it may be measurement rather than rendering.
  Re-measure from a same-display 2× capture before changing any colour constant.
- **Not done**: by-ear tuning, and registration in `../manifest/suite.toml` — held until all six
  suite plugins exist, which as of Elmer they now do, so it is next once there is a tagged release.
