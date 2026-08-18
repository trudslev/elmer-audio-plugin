# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

Elmer is its own independent repo and does not depend on `../taperot/`, `../gatecrasher/`,
`../chorus-60/`, `../reflect-84/` or `../fifth-member/` at runtime or at build time — it is a
sibling casting under the shared [Neon Foundry](../BRAND.md) umbrella, and those repos are read
purely as structural reference. Read `../BRAND.md` first for the cross-plugin design system, then
this file.

`design/GUI-SPEC.md` is the authoritative GUI spec and `design/Elmer.dc.html` is the live prototype.
`design/screenshots/panel.png` is the acceptance target — this casting ships one, so there is no
need to drive the prototype in a browser. The bundle also carries `panel-menu-open.png`,
`panel-naming.png`, `header-naming.png` and `header-dirty.png`, which are the states this suite has
historically got wrong by eye.

**Every render is 2× of the CONTENT BOX, not of the canvas**, and getting that wrong puts every
measurement out by a factor and an offset at once. The content box is the canvas less its inset —
22 px left/right, 20 px top/bottom (GUI-SPEC §Canvas) — so it is 1076 × 736 at canvas origin
(22, 20). Hence `panel*.png` at 2152 × 1472 and `header-*.png` at 2152 × 224, the latter being the
same box cropped to the 112 px header block. To convert: `canvas = render / 2 + (22, 20)`.

That framing changed with the 2026-08-11 bundle and the old numbers are the trap — `panel.png` used
to be **1×** of the full canvas and the `header-*.png` **3×** of the content block. Three different
scales against two revisions, all plausible-looking, none stated in the spec. Measure the LCD's
outer edge against the 403..764 it is known to span before trusting a reading off any of them.

**GUI-SPEC's self-contradiction is resolved.** The 2026-08-11 bundle added the 34 px header band,
the annunciator/lens treatment and the `program-buttons.png` sprite sheet but left the previous
revision's prose beneath them, so the same file specified both 30 px cream buttons with a disabled
face and 34 px dark ones without. The 2026-08-12 re-cut deletes the stale half; the current half
survived intact and grew. Nothing in the build changed — it was written against the current half
from the start, on the grounds that it cited BRAND.md, carried measured ratios, and agreed with the
renders.

Worth keeping as a reading habit rather than a closed incident: **a spec revision that adds a
section is the moment to check what the old one said**, because prose does not get deleted by being
superseded. The disabled face this one described measured 1.56:1 and had already been abolished by
a suite-wide ruling, and it would still have been the first thing a fresh reader implemented.

**KNEE's lamp inversion is fixed**, artwork and code both. The lit face is now *darker* than the
unlit one (`#46402f → #322d21` against `#a9a496 → #8e8a7d`), so the selected legend reads
**9.5:1 → 12.6:1** where it used to measure 2.28–3.17:1. Elmer's one lit indicator had been its
least legible label, which is the exact inversion of what an indicator is for — and no runtime
change could have fixed it, since lifting `#FFF6C9` off that grey is not possible. It needed the
face to darken, which is a plate decision, not a code one.

## RESUME POINT — the header has a measured baseline, and it is what a panel move fails against

**Verified `6b3c610` on 2026-08-17: this casting's header draws exactly where its own constants say.**
Not read — captured from the Release standalone and measured off the pixels.

Why it is written here rather than left in a session report: the value of a baseline is entirely in
being read by whoever moves the panel next, and a figure that lives somewhere the mover does not
open is worth nothing.

**What was measured**, band at y 59, height 34, canvas 1120 x 776:

| Element | Constants | Measured |
|---|---|---|
| LCD | `lcdRowY` 59, `lcdRowH` 34 | 406.0 .. 761.0 |
| SAVE · DELETE | `saveX` 771, `deleteX` 840, `headerButtonW` 62 | 773.5 .. 899.5 |
| meter wells | — | 929.0 .. 1001.5, 1012.0 .. 1084.5 |

**What this is NOT.** This casting references `nf::HeaderGeometry` **nowhere**, so it is on its own
canvas and its own layout, and none of the figures above is expected to match the shared part. The
baseline says *internally consistent*, not *conformant*.

**The defect it exists to catch** was found in Chorus-60 on 2026-08-17: that casting's header pass
aliased its LCD to the shared part and left SAVE, DELETE and both meter wells as literals from the
previous canvas — **29 px right and 29 px down** — and nothing could see it, because the plate baked
those faces and the only symptom was text centred inside a box nobody drew. It surfaced the moment
the material had to be painted from those rects.

**So when this casting moves: alias every band figure in one edit, then re-measure against the table
above.** A rect that moves and a rect that does not are indistinguishable in a diff and obvious in a
measurement. And note that **a literal which happens to agree with core is indistinguishable from an
alias by reading** — Reflect-84 held four such literals, one of them 2 px off §4's shared descriptor
anchor, in the casting whose editor had been declared conformant.

> **THE ALIAS AND THE CANVAS CANNOT BE SEPARATE COMMITS — checked 2026-08-18, before starting.**
> The instruction above is right about *one edit* and understates why. The shared band puts the LCD
> at **357..998** and the meter wells out to **1302**; this canvas is **1120** wide. So aliasing the
> band before the canvas move does not merely look wrong, it places SAVE, DELETE and both meters
> past the right edge of the panel — and the reverse order leaves a 1340-wide panel with its header
> cluster bunched into the left two-thirds.
>
> The two are one change. §1's figures for this casting's move, read from the delivered spec so the
> next session does not re-derive them:
>
> | | |
> |---|---|
> | Canvas | **1340 × 660** — call 1 brought +220, and this is the only casting whose HEIGHT drops (776 → 660) |
> | Header block | 16, 16, 1308 × 104 — shared part |
> | Body origin | y **120** |
> | Divider, detector | x 500, y 136 → 380 |
> | Divider, horizontal | y 386, x 16 → 1324 |
> | Divider, lower | x 700, y 396 → 644 |
>
> **The height drop is what makes this casting different from the other five**, and the constants
> most likely to be missed are the ones derived from the old bottom rather than from the top: the
> `screwCentres` at y 765, `contentBottom` 756, and anything measured from `canvasHeight`. Chorus-60's
> equivalent move left its body regions 44 px out for two commits and nothing failed.

---

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

**Both divergences are choices rather than constraints now.** While the rings were baked into the
plate, correcting them meant re-cutting artwork to print `0.313` and `141.4`, which no panel does —
so the tolerances recorded a limit. With ticks and numerals drawn from rotation fractions the mark
values are editable, and ATTACK's range is built from conversion lambdas so `convertTo0to1` is
authoritative on it. Keeping the rounded prints is still right, because that is how the controls are
meant to read; the point is that a later reader should not carry either figure forward as a defect
nobody can reach.

### Programs

Gatecrasher's architecture reused directly: Factory/User banks, one `.elmerprogram` XML per User
Program, SAVE always creates new and never overwrites, DELETE gated to User Programs at both the
button and the model, `AsyncUpdater` for the audio-thread-safe apply, one FACT/USER field inside the
LCD. Reflect-84's two improvements are carried: name collisions use `getNonexistentSibling()`, and
the schema version is read on restore, not merely written.

**The bank on disk is `nf::UserProgramStore` now** — scanning, sort-by-stem, naming, the collision
check, save and delete all come from `neon-foundry-core`, pinned at `v1.0.0` and declared *after*
`FetchContent_MakeAvailable(JUCE)` because core refuses to fetch its own JUCE. What a Program
*contains* stays here: all nine parameters and the schema attribute. Core owns files and names; this
repo owns meaning.

Two behaviours changed with it, both deliberate. The empty-name fallback is **`TAKE n`**, not
`UNTITLED` — six castings had five different fallbacks, and consecutive empty saves now give
`TAKE 3`, `TAKE 4` rather than leaning on `getNonexistentSibling` for `UNTITLED (2)`. And
**upper-casing and the 22-character cap apply on every save path**, where they used to live in
ProgramHeader's keystroke filter alone, so any programmatic save bypassed both. `ProgramManager`
carries its own copy of the cap because it cannot include a GUI header; `DisplayBudgetTests` asserts
the two are equal, which is the binding.

**INIT is index −1, and that is load-bearing in three places.** It sits outside both banks: the menu
prints it unnumbered above a divider, the bank cell shows an em-dash at 42 % phosphor rather than
naming a bank it is not in, and DELETE is disabled on it as it is on a Factory Program — INIT is not
a stored thing, so there is nothing to delete. Two consequences follow that are easy to reintroduce:
`getDisplayName` must special-case it, since `index + 1` padded to two digits prints `00`; and the
AsyncUpdater's "nothing pending" sentinel is **−2**, because −1 now means INIT.

**The dirty flag has exactly one definition and two consumers.** `ProgramManager::isModified()`
compares against an `nf::ParameterSnapshot` taken at apply time — normalised values keyed by
parameter ID, from `neon-foundry-core`, rather than the positional `std::vector<float>` of physical
values it replaced. The spin lock that used to guard it is inside the snapshot now, so all six
castings get it; four of them had that read and written across threads with nothing at all. The display's asterisk and SAVE's
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

**The header row is one 34px band, and 59 is derived, not chosen.** Display, SAVE, DELETE and the
IN/OUT readouts are all 34 px tall on one Y, so the band reads as a single instrument. The row is
centred against the **full 112 px header block** from the 20 px content inset — (112 − 34) / 2 + 20 —
not against the wordmark plate, which is what the old 37 amounted to and which left the display
sitting high against a left column that runs well below it.

**34 is the suite's figure rather than this panel's**, per BRAND.md: the castings are
differently-sized units, not scales of one design, so the header part is the same physical size in
all six. It came up from 30 with that ruling, taking the row's Y from 61 to 59 and the captions from
45 to 43 with it. `DisplayBudgetTests` asserts the centring *relationship* rather than the literal,
so the height cannot move without its Y — that half-update is exactly what would otherwise ship.
**Widths did not change**, so the character budget is untouched at 24 and the cap at 22; the extra
height went entirely into the buttons.

**The Program buttons are split-legend annunciator caps** — one body, two independently-lamped
windows, SAVE above STORE and DELETE above CANCEL. Neither relabels and neither has a disabled face.
The cream cap and its greyed-out disabled treatment are both gone and neither should return: a
printed panel legend cannot rewrite itself, no rack unit greys a button out, and that disabled label
measured **1.56:1**. Both legends dark is the "nothing to do here" state now, at 4.3–5.0:1.

**The lens body is identical lit and unlit; only the characters light.** An earlier revision warmed
the whole lit window and it read as a rectangle lighting up rather than a bulb behind a legend —
the failure BRAND.md names when it says the lamp lights the letters, not the button. The two windows
share a continuous ground with no rib or divider between them, so a lit legend's halo spills into
the neighbouring half the way a real bulb behind a split lens does.

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
2026-08-11 renders show it below**, so artefact and prose finally agree; the disagreement that stood
here for two revisions was resolved in the designers' favour of the prose, and the render caught up.

**Live values take the LCD over, not a tooltip.** Grabbing a control replaces the program name with
that parameter's name and value, reverting **900 ms** after release — `nf::ReadoutFormat::revertMs`
in `neon-foundry-core`, where it was 1200 here. The suite ran 800 / 900 / 1100 / 1200 under three
constant names and two mechanisms, with no spec justifying any of them. This is `design/GUI-SPEC.md`'s
explicit choice over BRAND.md's tooltip convention — it reuses a display already on the panel, a
tooltip has no hardware equivalent, and it satisfies BRAND.md's stronger rule that dynamic text
lives inside a screen. The takeover fires only on a **grab**: a `SliderAttachment` raises
`onValueChange` when a Program is applied and when the host automates, and without guarding on the
drag state the display latches onto whichever parameter was written last.

**The readout string comes from `nf::describeParameter`, and the value formatters live in
`Parameters.h`.** `ProgramHeader` used to hold a hand-written `if`-chain per parameter ID, which is
what made ATTACK's missing formatter invisible: its `NormalisableRange` is built from conversion
lambdas and therefore carries no interval, so JUCE printed it at **seven decimal places** in every
host's automation lane while the panel showed a tidy `4.7 ms`. The same defect Gatecrasher hit and
fixed, and TapeRot shipped — hidden here rather than absent.

Elmer keeps `separatorColon = false`, which is `GUI-SPEC.md`'s own ask and the one legitimate
divergence in the suite. The hard-coded `MAKEUP +` is gone; it printed `MAKEUP +0.0 dB` at zero.

`ReadoutConformanceTests` sweeps every parameter across its range and fails the build on a value
that would print badly. **It earned its place immediately**: the replacement ATTACK formatter was
written with `String(v, 0)` for the ≥ 10 ms case, which does not round — `juce::String(double, int)`
only sets a formatting flag above zero decimals and otherwise falls through to `std::ostream`'s
default, so `String(16.0191f, 0)` is `"16.0191"`. The test caught it within the minute, against a
comment two lines away warning about exactly that.

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

### The Program list's group caption

**Sized from its own type plus padding, never derived from the row height.** The construction is
`nf::captionHeight (font, topPadding, bottomPadding)` — 3px above and 4px below, the suite's adopted
default — and it comes out **19px** here, from IBM Plex Mono at 9px.

**The construction is the rule, not the number.** Writing 19 as a literal would break silently at
the first change of font, size or font construction, which is a change nobody would think to check a
caption against. It is also how this caption came to inherit JUCE's `rowHeight + rowHeight / 2` in
the first place — a caption half again *taller* than a row, which is a menu convention rather than a
panel one.

**This casting is where the rule came from.** It was the only one that had overridden JUCE's
inherited `rowHeight + rowHeight / 2`, and its own comment recorded that the default "pushed
everything below FACTORY 14px down the list". The other four took the inherited value by omission.

### Case belongs at the source

`nf::ReadoutFormat::ValueCase` is deleted from core (2026-08-13). **A panel label reads in caps
because it is authored in caps in `Parameters.h`**, not because the readout upper-cased it on the
way out — the panel and the host's automation lane read the same parameter, so any re-casing in
between makes one of them lie about the other. Every parameter name here is authored in caps for
that reason, and `Tests/ReadoutConformanceTests.cpp` asserts it off `getName()`.

The rule is in `../BRAND.md` beside the unit rule; the suite-wide record is in the root
`../CLAUDE.md`. **The choice strings are deliberately NOT all caps** — this casting prints its
values as authored, and the rule is that case is decided at the source, not that everything is caps.

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
- **The "10–15 levels darker" finding was measurement, not rendering, and is closed.** Re-measured
  on 2026-08-11 from a same-display 2× `screencapture -R`, sampling four fascia patches clear of any
  control: the build differs from the render by **−1.1 to +2.4 levels per channel**. The original
  figure came from the non-colour-faithful pipeline exactly as suspected. No colour constant was
  changed, and none should be on the strength of a capture that cannot reproduce `#EFE9D6`.

  The method that settles this class of question is a **whole-panel difference map on a 40px grid**,
  not an eyeball or a global mean. It is what found the scribble strip: the footer was the only cell
  above 30 while everything else sat under 20, and it had survived several passes that looked
  straight at it. Expect the knob columns and the meter to read 18–28 legitimately — the render is
  captured mid-compression, so its needle and some pointers are elsewhere.
- **Outstanding with the designers**: GUI-SPEC's own Program-button section contradicts the rest of
  the same file — see below — plus the 264 px dropdown cap,
  and SIDECHAIN HP's dead band — ~56° of travel does nothing, because the OFF zone ends at 0.10 and
  the frequency curve starts at 0.20. That last one is a recommendation, not a defect: the 40 Hz
  mark is baked into `scale-lg.png`, so moving the curve without re-cutting the ring would leave the
  pointer on a mark that lies. **Bundle it with the next re-cut**, whenever one happens anyway.
- **Not done**: by-ear tuning, and registration in `../manifest/suite.toml` — held until all six
  suite plugins exist, which as of Elmer they now do, so it is next once there is a tagged release.
