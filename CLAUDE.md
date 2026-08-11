# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

Elmer is its own independent repo and does not depend on `../taperot/`, `../gatecrasher/`,
`../chorus-60/`, `../reflect-84/` or `../fifth-member/` at runtime or at build time — it is a
sibling casting under the shared [Neon Foundry](../BRAND.md) umbrella, and those repos are read
purely as structural reference. Read `../BRAND.md` first for the cross-plugin design system, then
this file.

`design/README.md` is the authoritative GUI spec and `design/Elmer.dc.html` is the live prototype.
`design/screenshots/panel.png` is the panel rendered at 2× and is the acceptance target — this
casting ships one, so there is no need to drive the prototype in a browser. The bundle also carries
`panel-menu-open.png`, `panel-naming.png`, `header-naming.png` and `header-dirty.png`, which are the
states this suite has historically got wrong by eye; the three `header-*.png` are **3×** of a
1076 × 112 block whose origin is canvas (22, 20), not 2× of the canvas.

**`panel.png` and `panel-menu-open.png` are stale on KNEE's label** — they still show it above the
buttons, where the prose and `Elmer.dc.html` both put it below. The prototype and the prose win;
this is raised with the designers. Where an artefact and the prose disagree, measure both before
choosing, and record which won and why.

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

**INIT is index −1, and that is load-bearing in three places.** It sits outside both banks: the menu
prints it unnumbered above a divider, the bank cell shows an em-dash at 42 % phosphor rather than
naming a bank it is not in, and DELETE is disabled on it as it is on a Factory Program — INIT is not
a stored thing, so there is nothing to delete. Two consequences follow that are easy to reintroduce:
`getDisplayName` must special-case it, since `index + 1` padded to two digits prints `00`; and the
AsyncUpdater's "nothing pending" sentinel is **−2**, because −1 now means INIT.

**The dirty flag has exactly one definition and two consumers.** `ProgramManager::isModified()`
compares physical values against a snapshot taken at apply time. The display's asterisk and SAVE's
enablement both read it, rather than each deciding for itself — the spec requires the two to agree
always, and the cheapest way to guarantee that is to give them nothing to disagree about. The
snapshot is re-taken on apply, on save, and on session restore.

**Session state is the APVTS plus which Program is showing.** Those two attribute names and the
schema number are a contract: rename one and the session still parses while the Program silently
reverts to the default, with no error anywhere. Restore goes through
`setCurrentProgramIndexWithoutApplying`, which moves the pointer and re-takes the snapshot **without
touching a parameter** — re-applying would overwrite exactly what was just restored — and
`cancelPendingChange()` runs first, because a change requested just before the restore would
otherwise land just after it. Sessions with no schema attribute predate this and fall back to the
default Program: they carry values but no Program identity, and naming one that was never recorded
would be a lie rather than a default.

**Naming happens in the display, not a dialog.** SAVE asks for a name rather than storing one: the
bank cell reads `NAME`, the chevron hides, the display takes the keyboard, and the two buttons
already in the row become STORE and CANCEL. No new controls appear and the row's geometry does not
move. Three details that are each a bug if undone:

- **Cancel must never touch a parameter.** Whatever the user tweaked before pressing SAVE has to
  survive cancelling, so cancel is a mode exit and nothing else. Focus loss cancels too, which is
  Fifth Member's divergence from TapeRot — and the reason a click on the glass while naming does
  nothing rather than opening the list over a half-typed name.
- **The cursor is a block, U+2588 at 1 s / 50 % duty**, not the spec's native caret. The house form
  outranks the per-plugin figure. Elmer's field is **centred** where every sibling's is
  left-aligned, so the cursor's cell is in the string on both phases (a space when dark): appending
  the block only when lit, which is safe left-aligned, would walk the whole name half a character
  sideways at every blink.
- **19 characters, and the number is not inferable from the layout.** 3 (`01 `) + 19 + 2 (` *`) = 24,
  the budget the 269 px cell holds. `Tests/DisplayBudgetTests.cpp` pins all four figures to each
  other and to the longest name read from the bank, so a longer Program fails a test rather than a
  panel.

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

**The header row is one 30px band, and 61 is derived, not chosen.** Display, SAVE, DELETE and the
IN/OUT readouts are all 30 px tall on one Y, so the band reads as a single instrument. The row is
centred against the **full 112 px header block** from the 20 px content inset — (112 − 30) / 2 + 20 —
not against the wordmark plate, which is what the old 37 amounted to and which left the display
sitting high against a left column that runs well below it.

The display is 403 × 361, and 361 rather than the 353 the cells alone give: three cells
(56 + 269 + 28) plus **two** 1 px hairlines plus the 3 px frame each side. The cells are explicit
widths, never fractions of the glass, because the character budget is computed from the NAME cell's
269 less 2 × 11 px padding — a proportional split would let a change in glass width silently change
how many characters fit while the typing cap stayed put.

**The Program dropdown takes the display's OUTER width**, frame edges included, so the two share a
left and a right edge — measured 403..764 in `panel-menu-open.png`. The spec's "left: 0, width:
100 %" would resolve against the padding box in CSS and give 355 px of glass; the render is the
artefact, so it won. `ElmerMenuLookAndFeel` dresses it: ground `#16150f`, 22 px rows, and a border
that deliberately omits its **top** edge, because a rule along the join with the glass draws a seam
exactly where the design wants the two to read as one. That is also why it is three strokes and a
path rather than `drawRoundedRectangle`, which cannot omit an edge. The current Program is marked
with a **2 px left bar, not a tick** — a bar reads straight down the column and costs no character
cell, since it occupies the 2 px that unselected rows spend on padding. Row height is pinned at 22
and explicitly not allowed to grow to the platform's standard item height, which on macOS is taller:
seventeen rows have to fit the panel. The mechanism that anchors the list — and the four `PopupMenu`
traps behind it — is in the root `CLAUDE.md` under "The Program dropdown".

**`max-height: 264px` is deliberately not implemented.** The suite contract runs the list to the
panel's bottom and outranks a per-plugin figure. Here that is 424 px of list against 685 px of panel
below the display, so all seventeen rows open without scrolling where the render scrolls with 421 px
unused. Flagged to the designers; do not "fix" it to match the render.

**Units live in the arc gap, not on the control name.** They sit on the bottom legend row, between
the minimum and maximum numerals, in the same 10 px legend type at 0.6 px tracking — so a unit reads
as part of the printed scale it qualifies rather than as part of the control's name. Six controls
carry one and three do not, decided from the parameter definitions rather than from what the labels
happened to print: RATIO prints ratios, RELEASE's values carry their own suffixes, KNEE has no scale.
The offsets are transcribed literals like `legends`, for the same reason — ATTACK's top is 87 where
its neighbours' is 85.

**KNEE's label sits below its buttons**, sharing SIDECHAIN HP's baseline; it was the only control on
the panel named from the top. Its buttons start at y 399 rather than the prototype's literal 400, so
that 63 px of buttons plus the 15 px label margin lands the label on 477 exactly — the prototype's
own arithmetic puts it 1 px lower, and a shared baseline is the property that is visible. **The
bundled renders still show the label above**; see the note at the top of this file.

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
- **Programs**: 16 factory Programs with authored values, plus INIT; no by-ear pass. The dropdown,
  the dirty flag, the naming flow and the session-state contract are all in and click-verified with
  real `CGEventPost` events, not by code review — see the root `CLAUDE.md` on why a clean build is
  not sign-off for interaction.
- **GUI**: conformant to the revised handoff (`design/`, 2026-08-10). Geometry verified against
  `design/screenshots/panel.png` by measurement rather than by eye — the header row lands 61..91,
  the display 403..764, SAVE/DELETE at 771/840, IN/OUT at 928/1011, and caption ink at y 47..54,
  all exact. All eight knob centres land exactly, and the meter face and section-box edges are
  within 1 px. `auval` and `pluginval --strictness-level 8` pass on AU and VST3.

  Capture at exactly 1120 × 776 or the numbers lie: the standalone restores its last window size and
  macOS resamples the GUI to fit, so every edge differs slightly. Set the size explicitly, confirm
  it took, and guard every capture on the app being frontmost.
- **Outstanding**: a mid-tone comparison against the reference render shows this build reading
  ~10–15 levels darker across the right and lower panel. The cause is **not yet isolated** — the
  capture pipeline used for the comparison is demonstrably not colour-faithful (the scribble tape is
  drawn `#EFE9D6` and captures as `#F5EFD4`), so it may be measurement rather than rendering.
  Re-measure from a same-display 2× capture before changing any colour constant.
- **Outstanding with the designers**: the two stale renders (KNEE's label), the 264 px dropdown cap,
  and SIDECHAIN HP's dead band — ~56° of travel does nothing, because the OFF zone ends at 0.10 and
  the frequency curve starts at 0.20. That last one is a recommendation, not a defect: the 40 Hz
  mark is baked into `scale-lg.png`, so moving the curve without re-cutting the ring would leave the
  pointer on a mark that lies. **Bundle it with the next re-cut**, whenever one happens anyway.
- **Not done**: by-ear tuning, and registration in `../manifest/suite.toml` — held until all six
  suite plugins exist, which as of Elmer they now do, so it is next once there is a tagged release.
