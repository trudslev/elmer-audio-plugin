# GUI-SPEC — Elmer, Bus / Glue Compressor (Neon Foundry, casting six)

**This file is the build contract. There is no second document — everything to build from is here.**

## Overview
Elmer is a stereo-linked bus compressor plugin, the sixth and final plugin in the Neon Foundry suite.
This bundle contains the approved GUI design for its single fixed-size panel, the pre-rendered bitmap
assets that panel is built from, and the product icon.

The panel presents itself as a **console channel module** — something unbolted from a large mixing
desk — rather than a standalone rack unit: warm medium-grey fascia, printed scale markings directly
on the panel around every knob, small grey lamp buttons, chunky skirted knobs with colour-coded caps
and white pointer lines. Its signature element is a large **analog moving-coil gain-reduction meter**,
deliberately not the dark phosphor scope every sibling plugin uses.

Target frame: **JUCE** (AU / VST3 / Standalone), consistent with the rest of the suite.

## About the design files
`Elmer.dc.html` and `Elmer Icon.dc.html` in this bundle are **design references authored in HTML** —
prototypes that show the intended look, proportions and behaviour. They are not production code and
should not be embedded or ported line-by-line.

The task is to **recreate this design in the target codebase's own environment** — for this suite that
means JUCE C++ `Component` subclasses drawn with the bundled bitmaps, following the patterns already
established in the sibling plugins' GUI source. If no environment exists yet, pick the framework
appropriate to the product and implement the design there.

The bitmaps in `assets/` **are** production assets and should be used directly.

## Fidelity
**High fidelity.** Final colours, typography, spacing, sizes and interaction behaviour. Recreate the
UI pixel-accurately at the stated canvas size, using the codebase's existing drawing and layout
patterns. Every measurement below is authoritative.

---

## Canvas

| | |
|---|---|
| Panel size | **1120 × 776 px** (fixed aspect; scale proportionally) |
| Corner radius | 5 px |
| Fascia base | `#a9a294` warm medium grey — set by the 7:1 legibility floor, see Legibility below |
| Fascia texture | `repeating-linear-gradient(96deg, rgba(255,255,255,.05) 0 1px, rgba(0,0,0,.035) 1px 2px, transparent 2px 4px)` — a fine brushed grain at 96° |
| Fascia lighting | radial highlight `120% 90% at 28% 0%` from `rgba(255,255,255,.16)` to transparent at 58%; radial shade `120% 120% at 80% 110%` from `rgba(40,34,26,.03)` to transparent at 66% — the shade is deliberately shallow so the lower-right region stays above 7:1 |
| Side rails | 15 px wide on both edges, `linear-gradient(90deg,#847e73,#B4AE9F 60%,#98917f)` (mirrored on the right) |
| Corner screws | 6 px circles at 5 px in from each side, 8 px from top/bottom; `radial-gradient(circle at 35% 30%, #ded8c9, #6d685e)` with a 1 px white top highlight |
| Content inset | 22 px left/right, 20 px top/bottom |
| Drop shadow (presentation only) | `0 30px 70px rgba(0,0,0,.55)` |

### Vertical stack (inside the 22/20 inset)
| Band | Height |
|---|---|
| Header | 112 px |
| Divider rule | 1 px, margins 8 px above / 14 px below |
| Top row — DETECTION + meter | 352 px |
| Gap | 16 px |
| Bottom row — TIMING · CHARACTER · OUTPUT | 190 px |
| Footer | pushed to the bottom, 12 px padding-top |

Divider rule: `linear-gradient(90deg, rgba(60,54,44,.45), rgba(60,54,44,.18))` with a
`0 1px 0 rgba(255,255,255,.28)` highlight beneath — an incised score line in the metal.

---

## Typography

| Role | Font | Size / weight | Colour |
|---|---|---|---|
| Wordmark | Archivo Black | 53 px, letter-spacing 6 px | `#24231f` |
| Model tagline "BUS COMPRESSOR" | Barlow Condensed 600 | 14 px, ls 3.6 px | `#0f0f0c` |
| Model line "MODEL GL-87 · STEREO" | IBM Plex Mono 500 | 10.5 px, ls 2.2 px | `#0f0f0c` |
| Section headers | Barlow Condensed 600 | 11.5 px, ls 3.4 px | `#0f0f0c` |
| Control labels | Barlow Condensed 600 | 11.5 px, ls 2.4 px | `#0f0f0c` |
| Printed knob legends | IBM Plex Mono 400 | **10 px**, line-height 12 px | `#0f0f0c` |
| LCD / numeric readouts | IBM Plex Mono 400 | 14 px, ls 1.7 px | `#e6dcae` |
| Program menu rows | IBM Plex Mono 400 | 12 px, ls 1.4 px | `#b9ae86`, selected `#f2e9c4` |
| Program menu bank headers | IBM Plex Mono 400 | 9 px, ls 2.6 px | `#8a8163` |
| Meter header + footer lines | IBM Plex Mono 600 | 11.5 px, ls 1.8 px | `#0f0f0c` |
| Panel serial line | IBM Plex Mono 600 | 11.5 px, ls 1.8 px | `#0f0f0c` |
| Lamp-button legends | IBM Plex Mono | 9.5 px, ls 1.6 px | unlit `#3a372e`, lit `#FFF6C9` |
| Scribble strip | Permanent Marker | 21 px | `#2b2a26` |

### Legibility

BRAND.md requires **~7:1 contrast for functional text** — control labels, printed scales, section
headers, readouts and the model/function line. This constraint sets the fascia value, not the other
way round: on the original `#8b8579` (luminance 0.2365) even pure black tops out at 5.7:1, so the
panel could not reach the floor by darkening ink. The fascia was raised to **`#a9a294`** (luminance
0.364), same warm hue, and all functional ink unified at **`#0f0f0c`**.

Measured against the current fascia:

| Surface | Ratio |
|---|---|
| Ink on flat fascia | 7.57 : 1 |
| Ink inside a section box (wash lifts it) | 8.0 : 1 |
| Ink in the darkest corner (shade radial at full strength) | 7.22 : 1 |
| Meter footer text on the cream face | 7.0 : 1 |

Two things follow, and both must be preserved if the fascia is ever retuned: the bottom-right shade
radial is held at `.03` (it was `.30`, which dragged that region to 4.7:1), and the section wash
lifts at both stops rather than darkening at the bottom.

The knob filmstrips, scale rings, lamp-button faces, nameplate plinth and meter bezel were all
re-rendered against the lighter fascia — the bundled assets are the current ones. If the fascia value
changes again, re-render them rather than reusing these.

All dark panel text carries a 1 px light relief edge: `text-shadow: 0 1px 0 rgba(255,255,255,.28–.34)`.
This is a legibility floor as much as a material cue — **nothing on the panel sits below 10 px except
the marker strip's own irregularity**, and no functional text is rendered in mid-grey. See Legibility above for the measured ratios.

Substitutions in JUCE: Archivo Black, Barlow Condensed, IBM Plex Mono and Permanent Marker should be
embedded as binary font resources. If Permanent Marker cannot be licensed for redistribution, replace
the scribble strip with a hand-lettered bitmap rather than a substitute typeface — a geometric
fallback destroys the effect.

---

## Colour system

Per BRAND.md, one accent colour per plugin, reserved for the single most important live indicator.

| Token | Value | Use |
|---|---|---|
| Fascia | `#a9a294` | panel base (luminance 0.364) |
| Section panel wash | `linear-gradient(180deg, rgba(255,255,255,.10), rgba(255,255,255,.02))` + `0 1px 0 rgba(255,255,255,.30) inset` | recessed control groups — lifts, never darkens |
| **Ink — all functional text** | **`#0f0f0c`** | headers, labels, printed scales, spec lines |
| **Accent — lamp** | **`#F3D021`** golden yellow | KNEE lamp dots only |
| LCD glass | `linear-gradient(180deg,#1b1a16,#242219)` | program display, IN/OUT |
| LCD phosphor | `#e6dcae` + `0 0 8px rgba(214,196,124,.45)` | all lit LCD text |
| Detection knob cap | `#D5257A` magenta (hi `#EE5C9C`, lo `#8E1152`) | THRESHOLD, RATIO, SIDECHAIN HP |
| Timing knob cap | `#1B9E74` green (hi `#4FC79C`, lo `#0C6247`) | ATTACK, RELEASE |
| Character/Output cap | `#3A6FD0` blue (hi `#6E9CE8`, lo `#1E4189`) | IRON, MAKEUP, MIX |
| Knob skirt | `linear-gradient(#E6E1D8 → #C0BAB0 → #948E85 → #726C64)` | all knobs |
| Lamp face — unlit | `linear-gradient(180deg,#a9a496,#8e8a7d)` | unselected KNEE button |
| Lamp face — lit | `linear-gradient(180deg,#46402f,#322d21)` | selected KNEE button — darker than unlit, by design |
| Annunciator body | `linear-gradient(180deg,#57503f,#211f19)` | SAVE / DELETE cap + collar |
| Lens window — unlit | `linear-gradient(180deg,#2a2822,#1e1c17)`, legend `#8f8a7c` | dark legend, **4.3–5.0:1** |
| Lens window — lit | ground **unchanged**, legend `#FFEFD0` + warm glow | warm bulb, **13.1–15.1:1** |
| Menu ground | `#16150f`, border `1px solid rgba(214,196,124,.30)` | program dropdown |
| Meter cream | `#EFE7D2` warming to `#FFFCEF` at the lamp | meter face |

The golden accent is **not** used anywhere else on the panel — not on knobs, not on labels, not
decoratively. Orange and red are deliberately avoided: they belong to sibling plugins.

---

## Screens / views

There is one screen: the plugin panel. It has no navigation, modals or alternate states beyond the
control states described below.

### Header (112 px tall, content inset 15 px left and 13 px right)

**Nameplate (left, 340 px column).** "ELMER" in Archivo Black 53 px, `#24231f`, on a raised moulded
plinth: `padding 7px 20px 9px 18px`, radius 3 px, `linear-gradient(180deg,#b4ad9a,#9b9488)`, with
`0 1px 0 rgba(255,255,255,.42) inset, 0 -2px 3px rgba(48,42,32,.35) inset, 0 2px 3px rgba(40,35,26,.30)`.
The letters read as **ink-filled engraving**: near-black fill doing the work, relief only as a
`0 1px 0 rgba(255,255,255,.30)` / `0 -1px 0 rgba(0,0,0,.45)` edge pair. This is Elmer's nameplate
metaphor and is distinct from every sibling's (label-maker, spray stencil, silkscreen, engraved
plate, gaffer scrawl).

Beneath it, 8 px down: `BUS COMPRESSOR`, then `MODEL GL-87 · STEREO`.

**Program display (centre, flexible width, 30 px tall, `box-sizing: border-box`).** A 3 px metal frame
`linear-gradient(180deg,#26241f,#3a372f)` with `0 2px 5px rgba(35,30,22,.55) inset` around one
continuous black glass. Inside the glass, flush against it, three cells divided by
`1px solid rgba(214,196,124,.22)` hairlines:

| Cell | Width | Contents |
|---|---|---|
| Bank | 56 px | `FACT` / `USER` / `NAME` — one label that switches text, never two with one greyed. On **INIT** it reads an em-dash `—` at 42 % phosphor with no glow: INIT sits outside the banks, so naming it here would print the word twice |
| Name | **269 px** | program name, **centred**, 11 px horizontal padding → **247 px of text** |
| Chevron | 28 px | the drawn open/close indicator |

### Header band height — 34 px (suite-wide)

LCD, SAVE, DELETE, IN and OUT all stand **34 px**, up from 30, per the suite figure in `BRAND.md`
("that height is 34px in every casting"). Widths are unchanged — the display stays 361 px at
x 403 → 764, the buttons 62 px, the meters 74 px, all gaps as before.

Consequences, so nothing is measured against a stale number:

| | Was | Now |
|---|---|---|
| Header row y | 61 → 91 | **59 → 93** — still (112 − h) / 2 + 20, centred on the 112 px header block |
| Caption cap ink (PROGRAM / IN / OUT) | y 47 → 54 | **y 45 → 52** — the captions sit 5 px above the row, so they rise with it |
| Dropdown top offset | 28 px | **32 px** = 34 − 2 × 3 px frame + 4 px, still flush to the glass's lower edge |
| Dropdown opens | 91 → 515 | **93 → 517** — still uncapped to the panel bottom, all seventeen rows visible |

**Character budget — 24 characters in the name cell.** IBM Plex Mono at **14 px with 1.7 px
tracking** advances 10.1 px per character (8.4 px glyph + 1.7 px tracking); 247 px ÷ 10.1 = 24.5. The build **must cap user Program
names at 19 characters** — the worst case is a Factory Program, where the display prefixes a
two-digit index and a space (`01 `, 3 chars) and appends a space and the dirty asterisk (2 chars)
to give 24 total. A 20-character name would overrun the cell the moment it was edited.
**User Programs now print without an index**, so a user name has 22 characters of room before the
asterisk rather than 19 — the budget grew, and the cap therefore stays at 19 rather than following
it up. `BRAND.md` allows a budget to grow and never a cap to shrink; raising the cap would also
orphan nothing, but it is not required and 19 is what the naming field enforces today. This clears the longest factory name (`03 MINNEAPOLIS SQUEEZE`, 22) and the longest
parameter readout (`SIDECHAIN HP 500 Hz`, 19). The number is not inferable from the layout — cap
against it explicitly.

**Confirmed unchanged by the 34 px pass.** The budget is set by cell *width* and type metrics, both
untouched: name cell 269 px (247 px of content) ÷ 10.1 px per character = 24, cap 19. It has not
fallen, which is what `BRAND.md` requires any header-height change to state.

Two notes on the cap, since the update brief quoted 22:

- **19 is Elmer's current cap, not 22.** 22 has never been implemented here, so holding at 19 is not
  a contraction and orphans nothing — there is no saved name it could strand. (Nothing in the suite
  has shipped in any case; see the roster note in `BRAND.md`.)
- **22 does not fit this cell at all.** 22 + `NN ` (3) + ` *` (2) = 27 characters against a 24-character
  budget — it would overrun by 61 px the moment a name were edited. Reaching 22 needs 273 px of
  content, i.e. **26 px more display width**, which moves SAVE, DELETE, IN and OUT — outside a
  header-height-only pass and against the locked geometry above. If the 22 matters, say so and it
  becomes a width pass: the room has to come from the wordmark block, since the meters are already
  hard against the 22 px right inset.

*Why these three moves together:* the display was 364 × 38 px, which read as a heavy block against a
panel of fine printed detail, yet only held 21 characters. Dropping to 30 px and 14 px type while
widening the cell to 269 px buys three characters and loses the weight.

The program name is **centred** in its cell, not left-aligned — names run 11 to 22 characters and
centring keeps the display balanced rather than leaving a ragged gap before the chevron. The naming
field is centred too, so the text does not jump when the field appears.

**Dirty marker.** When the loaded Program has been modified, an **asterisk is appended to the name
after a single space** — `01 UNDER PRESSURE *` — in the **same 14 px phosphor type and the same
`#e6dcae` with its glow** as the name itself. It is not distinguished by colour or weight: it is
part of the name's own string, and the name stays centred in the cell with the asterisk included, so
the text shifts left by half a character when it appears. This is the same marker the rest of the
suite uses, and it appears in step with SAVE becoming enabled — the two always agree.

Because the asterisk consumes a character cell, it is counted in the budget above.

**Chevron (drawn, not typographic).** A live element that flips with menu state, so it cannot be
baked into the display bitmap. Render as a stroked path in an 11 × 7 box, `stroke #e6dcae`,
`stroke-width 1.6`, round cap and join, no fill:

| State | Path |
|---|---|
| Closed | `M1 1.4 L5.5 5.6 L10 1.4` (points down) |
| Open | `M1 5.6 L5.5 1.4 L10 5.6` (points up) |

Specified as a path rather than a glyph so it renders identically regardless of platform font
fallback. Clicking either the name cell or the chevron cell toggles the menu.

**Program dropdown.** Hangs flush from the **glass's** lower edge: `top: 28px`,
`left: 0` — both measured from the frame's padding box, so the menu meets the glass, not the
outside of the 3 px metal frame (`30px − 2 × 3px` = 24 px of glass, plus 4 px so the menu clears the glass's inner shadow), **width 100% of the display** — it inherits the display's width rather than
carrying its own, so the two read as one instrument. Ground `#16150f`, border
`1px solid rgba(214,196,124,.30)` on the left, right and bottom only — **no top border**, radius
`0 0 3px 3px`, `padding: 4px 0`, drop shadow `0 14px 24px -6px rgba(0,0,0,.55)` (negative spread
so nothing bleeds upward). The menu meets the display's glass directly: no gap, no rule, no shadow
between them, `z-index: 40`.

**The list carries no height cap of its own.** Per the suite contract in the root `CLAUDE.md`, it
hangs from the display's lower edge at the display's width and runs to the **panel's** bottom
(`max-height: 665px` = 776 − 91 − 20 inset), scrolling only if a User bank ever exceeds that.
INIT + divider + FACTORY header + 16 Programs is 424 px, so the menu opens **91 → 515** and all
seventeen rows are visible without scrolling. An earlier 264 px figure in this document was a
per-plugin number that contradicted the contract; it is withdrawn, and `panel-menu-open.png` has
been re-exported against the uncapped list.

Rows are **22 px tall**, `padding: 0 12px 0 10px`, IBM Plex Mono 12 px / 1.4 px tracking:

| Row state | Treatment |
|---|---|
| Normal | text `#b9ae86`, `border-left: 2px solid transparent` |
| Hover | background `rgba(214,196,124,.13)`, text `#f2e9c4` |
| Selected (current Program) | `border-left: 2px solid #e6dcae` with padding-left 8 px, background `rgba(214,196,124,.10)`, text `#f2e9c4` with the phosphor glow |

The left bar is the current-Program marker — a lit rule rather than a tick glyph, so it reads at a
glance down the column and needs no character cell.

Contents, in this order:
1. **INIT** — unnumbered, at the top, shown in the display as `—` / `INIT`, followed by a `1px rgba(214,196,124,.24)` divider inset 10 px.
   It is a neutral starting point, not a stored sound, so it sits outside the bank rather than
   being numbered `00`.
2. **FACTORY** — a 9 px / 2.6 px-tracked header in `#8a8163`, then the sixteen authored Programs
   numbered `01`–`16` (listed under *Factory bank* below).
3. **USER** — same header treatment, then the saved Programs **listed by name with no number at
   all**. Factory Programs carry a computed `01`–`16` label; User Programs do not, because they
   sort alphabetically and any number would change whenever another was saved (`BRAND.md`,
   *A Program is identified by name, never by position*). The LCD prints a User Program's bare
   name for the same reason.
   **The USER section is always present, header and divider included.** When the bank is empty it
   prints a non-interactive `— none saved —` row in `#7a7460` on the `#16150f` menu ground —
   **3.9:1**, which clears the 3:1 floor a deliberately non-interactive row takes. An absent
   section is ambiguous between "you have none saved" and "this plugin doesn't do that"; one row
   answers it.

**Naming flow.** SAVE never overwrites, so pressing it must ask for a name. The display itself
becomes the field — it is the only screen on the panel, and a floating dialog has no hardware
equivalent:

- The bank cell switches to **`NAME`**.
- The name cell becomes an editable field in the same 14 px / 1.7 px phosphor type. The cursor is a
  **U+2588 block** in `#e6dcae` blinking at 1 s / 50 % duty — the suite convention, so the field
  reads as hardware rather than as a web form. It is focused automatically on entry, so the user
  types immediately. Because Elmer's field is **centred** where every sibling's is left-aligned, the
  cursor's cell must occupy the string on both phases (a space when dark); appending the block only
  when lit would walk the name half a character sideways at every blink.
- The chevron cell is **hidden** while naming — the menu is unreachable mid-name.
- Input is forced uppercase and hard-capped at **19 characters** (see the budget above) — what can
  be typed matches what can be displayed once the asterisk appears.
- **Confirm and cancel are the second legend on each of the two buttons already in the row** — STORE
  lights under SAVE, CANCEL under DELETE, and the two resting legends go dark. Nothing relabels; see
  *Program buttons* below. `Enter` confirms, `Esc` cancels. No new controls appear — the row's
  geometry does not move.
- On confirm the Program is appended to the User bank, selected, and the bank cell reads `USER`.
  An empty name stores as `UNTITLED`.

**SAVE / DELETE (62 × 30 px cream buttons, 7 px gaps).**

**SAVE is disabled until the Program is dirty.** Loading a Program (or INIT) clears the dirty flag;
moving any knob, switch or KNEE button sets it. With the loaded Program unmodified there is nothing
to store, so SAVE renders in the same disabled treatment as DELETE below. It re-enables on the first
parameter change, and clears again once the new Program is stored. While naming it is always
enabled, as STORE.

DELETE has exactly two states:

| DELETE state | When | Treatment |
|---|---|---|
| Enabled | a **User** Program is current, or naming is in progress (as CANCEL) | `linear-gradient(180deg,#f0e9d3,#d6cdb2)`, text `#302c24`, `cursor: pointer`, raised bevel |
| Disabled | any **Factory** Program, and **INIT** | `linear-gradient(180deg,#a5a094,#8f8a7e)`, text `#6f6a5f`, `cursor: default`, flat — `0 1px 0 rgba(255,255,255,.22) inset, 0 1px 2px rgba(40,34,26,.2)` |

It stays in place and keeps its footprint when disabled; it never hides.

**IN / OUT (two 74 px columns, 9 px gap).** Legend above, **30 px** LCD readout below, 14 px
phosphor type. All four elements of the row — display, SAVE, DELETE, IN/OUT — share the same 30 px
height deliberately, so they read as one band; the display was not allowed to become the odd one out
when it came down from 38 px.

**Header alignment.** The row is **vertically centred against the full 112 px header block**
(`align-items: center`), not against the wordmark plate alone. The left column runs plate →
`BUS COMPRESSOR` → `MODEL GL-87 · STEREO` and extends well below the plate; centring on the plate
left the display sitting high against it.

### Top row (352 px)

**DETECTION** — 340 px section box, 14/16/12 px padding, radius 4 px.
- Row 1, centred with 4 px gap: **THRESHOLD** (large: 112 px scale area, 108 px tick ring, 84 px knob)
  and **RATIO** (standard: 100 px area, 96 px ring, 74 px knob).
- Row 2, 22 px below, centred: **SIDECHAIN HP** (standard) and **KNEE** (two stacked lamp buttons,
  7 px gap).

**Gain-reduction meter** — fills the remaining top-row width, centred, 588 px wide.
- Header line above, aligned to the meter's edges: `GAIN REDUCTION METER` (left) /
  `STEREO LINKED · ONE DETECTOR` (right).
- Meter body **588 × 236 px**, radius 4 px, `0 4px 12px rgba(35,30,22,.45)` plus a 1 px top highlight,
  with a diagonal glass sheen overlay `linear-gradient(118deg, rgba(255,255,255,.10) 0 22%, transparent 40%)`.
- Footer line below: `MOVING COIL · 300 ms BALLISTIC` (left) / `GR −x.x dB` (right, live).

**There is exactly one meter.** A bus compressor is stereo-linked — one detector drives both channels,
so gain reduction is identical left and right. A second needle would imply dual-mono, the opposite of
glue. Do not add one.

### Bottom row (190 px) — three sections spread across the full width

| Section | Width | Contents |
|---|---|---|
| TIMING | 340 px | ATTACK (continuous) · RELEASE (5-position switch) |
| CHARACTER | 230 px | IRON, centred |
| OUTPUT | 340 px | MAKEUP · MIX |

Paired knobs sit as a **cluster** (4 px gap, centred in their section) with clear space around the
group, rather than pushed to the section's outer edges.

### Footer
Left: the scribble strip (see Assets). Right: `GL-87 · CONSOLE MODULE · SN 0871 · v1.0`.

---

## Controls

All knobs are drawn from 128-frame vertical bitmap filmstrips. Frame index =
`round(normalisedValue × 127)`; source Y offset = `−frame × frameSize`. Rotation spans **−140° to
+140°** (280° travel) across the strip.

| Control | Group / cap colour | Law | Range | Printed legend | Tick ring |
|---|---|---|---|---|---|
| THRESHOLD | Detection / magenta | linear | −40 … +10 dB | −40 / −30 / −20 / −10 / 0 / +10 | `scale-lg.png` (11 marks) |
| RATIO | Detection / magenta | **5 detents** | 1.5 / 2 / 4 / 10 / 20 :1 | 1.5:1 / 2:1 / 4:1 / 10:1 / 20:1 | `scale-sm.png` (9 marks) |
| KNEE | — (lamp buttons) | 2-state | Soft / Hard | SOFT · HARD | — |
| SIDECHAIN HP | Detection / magenta | Off, then log | OFF, 40 … 500 Hz | OFF / 40 / 75 / 140 / 265 / 500 | `scale-lg.png` (11 marks) |
| ATTACK | Timing / green | logarithmic | 0.1 … 30 ms | 0.1 / 0.3 / 1 / 3 / 10 / 30 | `scale-lg.png` (11 marks) |
| RELEASE | Timing / green | **5 detents** | 0.1 / 0.3 / 0.6 / 1.2 s / AUTO | 0.1s / 0.3s / 0.6s / 1.2s / AUTO | `scale-5.png` (5 marks) |
| IRON | Character / blue | linear | 0 … 100 % | 0 / 25 / 50 / 75 / 100 | `scale-sm.png` (9 marks) |
| MAKEUP | Output / blue | linear | 0 … 20 dB | 0 / 5 / 10 / 15 / 20 | `scale-sm.png` (9 marks) |
| MIX | Output / blue | linear | 0 … 100 % | 0 / 25 / 50 / 75 / 100 | `scale-sm.png` (9 marks) |

Laws, precisely:
- **SIDECHAIN HP** — `f < 0.10` → OFF; otherwise `40 × 12.5^clamp((f − 0.2)/0.8, 0, 1)` Hz.
  Displayed to the nearest 1 Hz below 100, nearest 5 Hz above.
- **ATTACK** — `0.1 × 300^f` ms. Displayed 2 dp below 1 ms, 1 dp below 10 ms, integer above.
- **RELEASE** — detent times `[100, 300, 600, 1200] ms`; the fifth position is AUTO, a
  program-dependent dual time-constant (the simulation uses an effective ~420 ms).

**Units live in the arc gap, not on the control name.** The gap at the bottom of the tick arc,
between the minimum and maximum numerals, carries the unit in the same 10 px IBM Plex Mono legend
type (0.6 px tracking, ink `#0f0f0c`), with the control name below the knob.

**Six controls carry a unit, three do not**, decided from the parameter definitions rather than from
what the labels happened to print:

| Control | Unit | |
|---|---|---|
| THRESHOLD | **dB** | scale runs −40 → +10 dB |
| SIDECHAIN HP | **Hz** | |
| ATTACK | **ms** | |
| IRON | **%** | scale runs 0 → 100 |
| MAKEUP | **dB** | |
| MIX | **%** | |
| RATIO | — | prints ratios (`4:1`) |
| RELEASE | — | values carry their own suffixes (`0.6s`, `AUTO`) |
| KNEE | — | no scale — a two-position button pair |

No unit is invented for consistency's sake, and none is omitted for the sake of the original label
text.

**KNEE's label sits below its buttons**, like every other control name on the panel, on the same
baseline as SIDECHAIN HP — its column carries `padding-top: 37px` and the label `margin-top: 15px`.
The 37 rather than 38 is deliberate: at 38 the arithmetic (38 + 63 + 15) puts KNEE's label 1 px
below the knob columns' 115, and the shared baseline is the property that is actually visible.

**RATIO's column carries `padding-top: 12px`** so its label sits on THRESHOLD's line despite its
smaller (100 px vs 112 px) tick ring. Control labels align across a section regardless of ring size.

Legend placement: each numeral is centred on its tick's exact angle at a constant radius —
**54 px** from centre on standard knobs, **62 px** on THRESHOLD, with a handful of ±2 px optical
corrections. Tick counts are chosen so a mark falls on every printed value: 11 marks for the
six-value scales, 9 for the five-value scales, 5 for RELEASE's detents. A detented switch shows no
intermediate marks between its stops.

### Program buttons (SAVE / STORE, DELETE / CANCEL)

62 × 34 px, radius 3 px, width unchanged from the 30 px row. **Split-legend annunciator caps**: one
switch body carrying two independently-lamped windows, stacked — the resting function on top, what
the button becomes while a Program is being named beneath it. **Neither button ever relabels, and
neither ever wears a disabled face.** Only the lamps change.

Stacked rather than side by side because it costs no width: the button is already sized by its
longest single word and `DELETE`/`CANCEL` are both six characters. Side by side would roughly double
both buttons and the width would have to come from the LCD.

**Construction**, outside in:

| Part | Spec |
|---|---|
| Collar / body | `linear-gradient(180deg,#57503f,#211f19)`, `0 1px 0 rgba(255,255,255,.34), 0 2px 3px rgba(40,34,26,.42)`, 2 px padding so the lens sits proud |
| Windows | two, `flex: 1` — 15 px each inside the 34 px cap. Radius `2px 2px 0 0` top, `0 0 2px 2px` bottom |
| Rib | **none.** No divider, bezel, lens or lamp is drawn as its own element on these buttons — `BRAND.md` forbids it, and the two legends separate by their own leading plus the difference in kind between a luminous and a matte legend |
| Legend type | IBM Plex Mono **10 px** / 1.4 px tracking, centred — the 10 px functional floor, not below it |

**The two faces:**

| State | Window | Legend | Contrast |
|---|---|---|---|
| **Unlit** | `linear-gradient(180deg,#2a2822,#1e1c17)` | `#8f8a7c` | **4.3–5.0:1** — clears the 3:1 state floor |
| **Lit** | **identical to unlit** — `linear-gradient(180deg,#2a2822,#1e1c17)`, no ground or shadow change | `#FFEFD0` + `0 0 3px rgba(255,239,208,.95), 0 0 7px rgba(255,214,150,.85), 0 0 13px rgba(255,196,110,.55)` | **13.1–15.1:1** |

**The lens body never changes — only the characters light.** The two windows render the same ground in
both states; what differs is the legend and the halo the glowing characters throw onto the ground
around them. An earlier revision warmed the whole lit window and it read as a filled rectangle
lighting up rather than a bulb behind a legend — which is the failure `BRAND.md` names when it says
the lamp lights the letters, not the button. The light bleed is therefore the character glow's own
spill onto the continuous ground, reaching into the neighbouring legend's half. Lit is
**warm white**, incandescent rather than any of Elmer's three function-group colours, and not the
panel accent `#F3D021`, which stays reserved for the KNEE lamp.

**Which legend is live:**

| Condition | SAVE | STORE | DELETE | CANCEL |
|---|---|---|---|---|
| Factory or INIT, unmodified | dark | dark | dark | dark |
| Factory or INIT, edited (`*` showing) | **lit** | dark | dark | dark |
| User Program, unmodified | dark | dark | **lit** | dark |
| User Program, edited | **lit** | dark | **lit** | dark |
| Naming in progress | dark | **lit** | dark | **lit** |

Both dark reads as "nothing to do here", never as a blank button — the legends stay printed and
legible, which is exactly how a real one looks with the bulb out.

**Behavioural rules the lamp table does not imply:**

| Rule | Why |
|---|---|
| A press on a dark legend does nothing at all — no press animation, no collar movement | The lamp is the affordance. A dead button that still depresses reads as a bug |
| While naming, the *same physical buttons* act as STORE and CANCEL — SAVE's body commits, DELETE's body cancels | One switch, two functions, which is the whole reason for the split legend |
| **Escape out of naming leaves the Program edited** — `naming` and `nameDraft` clear, `dirty` stays `true`, so the `*` returns and SAVE re-lights | Nothing was stored. Clearing `dirty` here would silently discard the user's edit |
| Committing a name (Enter, or a press on STORE) clears `dirty`, switches the bank to USER and selects the new Program | The edit is now stored under a name, so there is nothing left to save |
| An empty or whitespace-only name commits as `UNTITLED` rather than refusing | A modal error in a 34 px header band has nowhere to go, and the Program can be renamed |
| Entering naming closes the Program menu, so the two can never be open together | Two competing text targets in one 34 px cell |

**Two divergences from the update brief, both following `BRAND.md`:**

1. **No lamp beside the legend.** The brief asked for a small lamp next to each legend on the grounds
   that Elmer's fascia is a light warm grey. `BRAND.md` rules that out explicitly — the buttons are
   dark in every casting whatever the fascia, because a lens cap is a moulded part inserted into the
   panel rather than painted panel, and "the lamp is *inside* the switch, behind the lens, which is
   where it is on the real part." It also answers the fascia argument directly: a pale cap has
   nowhere brighter to go. SAVE showing SAVE, STORE and an indicator beside each would be four
   elements where the design has two. These caps also sit in the dark header band, not out on the
   grey fascia, so the light-fascia premise does not apply here anyway.
2. **This is not the small-LED convention** used for KNEE. That one marks whether a *control* is
   relevant and sits beside a label; these illuminate the label itself. The KNEE lamp dot and its
   `#F3D021` accent are unaffected by this pass.

### Lamp buttons (KNEE)
74 × 28 px, radius 2 px. Contents: a 6 px LED dot then the legend, 7 px gap, centred.

**The face darkens when the button is lit.** A real illuminated legend lights the letters, not the
whole button, and the cream legend has nowhere brighter to go — so the ground moves instead. Both
faces, with the contrast of their own legend measured against each end of their gradient (WCAG
relative luminance, so it is checkable rather than asserted):

| State | Face | Legend | Contrast (top → bottom of gradient) |
|---|---|---|---|
| **Unlit** | `linear-gradient(180deg,#a9a496,#8e8a7d)`, `0 1px 0 rgba(255,255,255,.30) inset, 0 1px 2px rgba(40,34,26,.30)` | `#1d1c17` | **6.9:1 → 4.9:1** |
| **Lit** | `linear-gradient(180deg,#46402f,#322d21)`, `0 1px 0 rgba(255,255,255,.14) inset, 0 0 10px rgba(243,208,33,.10) inset, 0 1px 2px rgba(40,34,26,.34)` | `#FFF6C9` | **9.5:1 → 12.6:1** |

The lit face's worst case (9.5:1, at the light top of its gradient) clears the 7:1 floor with margin;
the glow on the legend and the amber dot only add to it. Before this change the lit legend sat at
2.3–3.2:1 on the unlit face — the least legible label on the panel, on its only lit indicator.
The unlit face is unchanged. **Do not equalise the two faces**: the inversion is the indicator.

Selection is otherwise shown two ways:
- the LED lights — `radial-gradient(circle at 34% 30%, #fffdf0, #F3D021 46%, #8a7108)` with
  `0 0 9px #F3D021, 0 0 20px rgba(243,208,33,.42)`; unlit is
  `radial-gradient(circle at 34% 30%, #7d7466, #43403a 60%, #2a2825)` with an inset shadow.
- the legend lights — `#FFF6C9` with `0 0 5px rgba(255,247,196,.95), 0 0 11px #F3D021, 0 0 18px #F3D021`;
  unlit is `#1d1c17`.

### LED convention
The **selected KNEE button's dot is the only lit indicator on the panel.** AUTO is a position on the
RELEASE switch, not a button, so it needs no lamp — the pointer is the state. **No knob ever changes
appearance**: knob bodies render identically at all times regardless of any switch state. They never
dim, grey out or go inert.

### Meter behaviour
- Scale is **gain reduction, not level**. `0` at the far right, values increasing leftward in even
  steps: 0, 4, 8, 12, 16, 20. The needle rests at 0 (far right) when no compression is happening and
  swings **left** as the compressor works. **There is no red zone** — red implies a fault, and gain
  reduction is not one.
- Face is a static bitmap; the **needle is a separate sprite rotated at runtime** about a pivot that
  sits below the visible face.
- Mapping: `angle = 63° − (GR / 20) × 126°`, clamped to 0…20 dB.
- Pivot in face coordinates (1000 × 402 source): **(500, 500)**; needle sprite pivot at
  **(30, 499)** in a 60 × 510 source. At the 588 px display width the scale factor is 0.588, putting
  the pivot at (294, 294) inside the meter body, below its bottom edge.
- Ballistics: 300 ms VU-style. The prototype smooths with an attack coefficient of
  `1 − exp(−16.7 / attackMs)` and a release coefficient of `1 − exp(−0.0167 / releaseSeconds)`,
  applied at 60 fps with a 0.55 damping factor.

---

## Interactions & behaviour

**Knobs** — vertical drag. The prototype uses `Δvalue = (startY − currentY) / 190 × range`; detented
switches snap to the nearest of their positions with a 1.6× sensitivity multiplier. Use the host
codebase's standard knob drag conventions (including fine-drag modifiers and double-click-to-default)
in place of these figures.

**Parameter readout in the LCD** — grabbing any knob or pressing a KNEE button replaces the program
name in the PROGRAM display with that parameter's name and current value (`THRESHOLD −18.5 dB`,
`RELEASE AUTO`, `KNEE SOFT`), in the same typeface and phosphor treatment. It reverts to the program
name **1200 ms after release**. This reuses a display already on the panel rather than introducing a
floating tooltip, which has no hardware equivalent.

**Program management**
- Terminology is **Program**, never "Preset" — in the UI, the parameter name, factory bank docs and
  marketing copy alike.
- The FACT/USER indicator is a single label inside the LCD reading one or the other.
- **SAVE always creates a new named Program and never overwrites**, even when a User program is
  loaded. There is no separate "New Program" button. Pressing it enters the naming flow described
  under *Header* above.
- **DELETE only works on User programs.** On a Factory Program or on INIT both its legends step back together; the button itself never shows a disabled face.
- Programs are selected from the panel via the LCD's chevron and dropdown — the plugin does not rely
  on the host's own Program list.
- Default state ships as `FACT` / `01 UNDER PRESSURE`.

**Factory bank (16 Programs, numbered 01–16).** Names are capped at 22 characters including the
index. User Programs continue the numbering from 17.

```
01 UNDER PRESSURE        09 HALFWAY THERE
02 ART OF GLUE           10 KILIMANJARO
03 MINNEAPOLIS SQUEEZE   11 WEST END
04 BLUE TUESDAY          12 BITE THE DUST
05 SHEFFIELD STEEL       13 DANCES ON THE SAND
06 JERSEY BUS            14 PASADENA
07 HAMMER DOWN           15 DON'T FORGET
08 QUEENS SMASH          16 PANCAKE
```

The two longest are `03 MINNEAPOLIS SQUEEZE` (22) and `13 DANCES ON THE SAND` (21) — the character
budget above is measured against these, not against invented placeholders. With the dirty asterisk
they become 24 and 23 — both still inside 24, so factory names are unaffected by the cap change.

**Meter animation** — continuous at 60 fps from the detector's gain-reduction value. The prototype
drives it from a simulated programme signal; the real build takes the DSP's GR value.

---

## State

| State | Type | Default |
|---|---|---|
| `threshold` | float, −40…+10 dB | −18.5 |
| `ratioIdx` | int 0–4 | 2 (4:1) |
| `knee` | enum soft/hard | soft |
| `hp` | float 0–1 (OFF below 0.10) | 0.42 |

> **Open item — SIDECHAIN HP dead band.** The OFF zone ends at 0.10 and the frequency curve starts
> at 0.20, so ~56° of travel does nothing. Starting the curve at 0.10 recovers it, but BRAND.md
> requires the printed scale and the mapping to agree, so the 40 Hz mark moves with the curve — and
> that mark is baked into `scale-lg.png`. **Bundle this with the next re-cut of the tick rings**;
> until then the dead band stays as specified.
| `attack` | float 0–1 (log-mapped) | 0.34 |
| `release` | int 0–4 (4 = AUTO) | 1 (0.3 s) |
| `iron` | float 0–1 | 0.34 |
| `makeup` | float 0–20 dB | 4.5 |
| `mix` | float 0–1 | 1.0 |
| `gr` | float 0–20 dB, read-only | 0 |
| `bank` | enum INIT/FACT/USER | FACT |
| `progIdx` | int, index within the current bank | 0 |
| `userBank` | string[], user Program names (≤ 19 chars each) | `[]` |
| `menuOpen` | bool | false |
| `naming` | bool | false |
| `nameDraft` | string, ≤ 19 chars, uppercase | `''` |
| `dirty` | bool — set by any parameter change, cleared on load/store | false |
| `editing` | parameter key or null (LCD takeover) | null |

---

## Design tokens

**Spacing** — 4 / 6 / 8 / 10 / 14 / 16 / 20 / 22 px. Section padding `14px 16px 12px`. Row gap 16 px.
Knob-cluster gap 4 px. Control label sits 15 px below its knob area.

**Radii** — 2 px (buttons, LCD glass), 3 px (LCD frame, nameplate plinth), 4 px (section boxes, meter),
5 px (panel).

**Shadows**
- Section box: `0 1px 0 rgba(255,255,255,.30) inset, 0 -1px 0 rgba(50,44,34,.25) inset`
- Raised button: `0 1px 0 rgba(255,255,255,.85) inset, 0 -2px 3px rgba(90,80,60,.25) inset, 0 2px 3px rgba(40,34,26,.35)`
- Recessed glass: `0 2px 5px rgba(0,0,0,.55) inset, 0 1px 0 rgba(255,255,255,.32)`
- Meter: `0 4px 12px rgba(35,30,22,.45), 0 1px 0 rgba(255,255,255,.34)`

---

## Assets

All bitmaps in `assets/` are production-ready and were generated procedurally at these sizes.

| File | Size | Notes |
|---|---|---|
| `knob-detect-128.png` | 96 × 12288 | Magenta cap. 128 frames, 96 px each, stacked vertically, −140°→+140° |
| `knob-timing-128.png` | 96 × 12288 | Green cap, same geometry |
| `knob-output-128.png` | 96 × 12288 | Blue cap, same geometry |
| `program-buttons.png` | 62 × 204 | Program button sprite sheet. **Six 62 × 34 cells stacked**, in order: `save-dark`, `save-lit`, `store-lit`, `delete-dark`, `delete-lit`, `cancel-lit`. Frame *n* is at y = n × 34. Labelled reference: `screenshots/program-buttons.png` |
| `meter-face.png` | 1000 × 402 | Cream face, printed GR scale, warm lamp, bezel with inner shadow |
| `meter-needle.png` | 60 × 510 | Black tapered needle + counterweight; pivot at (30, 499) |
| `scale-lg.png` | 200 × 200 | 11-mark tick ring |
| `scale-sm.png` | 150 × 150 | 9-mark tick ring |
| `scale-5.png` | 150 × 150 | 5-mark tick ring (RELEASE detents) |

Knobs are pre-rendered filmstrips rather than code-drawn gradients on purpose: the specular highlight
travelling across a coloured plastic cap as it rotates is the whole point of the colour-coding, and a
code-drawn gradient flattens it. Rendering the panel's scale rings at 0.72 opacity is intentional —
they read as printed ink on grey, not as drawn UI.

The **scribble strip** in the footer is hand-lettered in Permanent Marker at 21 px, on a
`linear-gradient(178deg,#efe9d6,#ded7c0 55%,#e6dfca)` tape rotated **−2.4°**, with hand-torn ends
produced by a 14-point `clip-path` polygon (straight long edges, ragged ends). Content:
`CH 24 — MIX BUS / GLUE`. In the real build this can stay a bitmap.

### Product icon
The chosen direction is **D — banded scale**: a black needle over a magenta / green / blue arc on a
warm cream face.

**Shipping icon (`assets/icons/`):**
- `elmer-icon-1024.png` — 1024 × 1024
- `elmer-icon-256.png` — 256 × 256

It was chosen because it carries both of Elmer's distinctive traits at once — the needle meter no
sibling has, and the three-colour knob grouping unique to this panel — and it survives 32 px with the
bands thickened to ~11 % of the frame. Needles are parked at mid-scale, not hard left; hard left would
read as 20 dB of gain reduction, which is not a representative state.

---

## Brand rules carried in from BRAND.md

- The plugin panel **never displays the "Neon Foundry" name**. `BRAND.md` is deliberately *not* bundled — read it from the repo, since a bundled copy goes stale within hours.
- "Programs", never "Presets".
- One accent colour, reserved for one live indicator.
- Hardware-spec-sheet voice: model line, function descriptor, version stamp — no software-plugin
  phrasing.
- Descriptions of what a control does are **not** printed on the fascia. Printed scales are; captions
  explaining a control's function are not. Earlier revisions carried such captions and they were
  removed deliberately — do not reintroduce them.

---

## Files in this bundle

| File | What it is |
|---|---|
| `Elmer.dc.html` | The panel design reference, interactive (drag knobs, click KNEE, SAVE/DELETE) |
| `assets/*.png` | Production bitmaps — knob filmstrips, meter face + needle, tick rings |
| `assets/icons/elmer-icon-1024.png`, `-256.png` | Shipping icon, JUCE sizes |
| `screenshots/panel.png` | The panel as rendered, 2× (2240 × 1552) |
| `screenshots/header.png` | The revised header alone, 3× |
| `screenshots/panel-menu-open.png` | The panel with the Program dropdown open, 2× |
| `screenshots/panel-naming.png` | The panel in naming mode (STORE / CANCEL), 2× |
| `screenshots/header-naming.png` | The header alone in naming mode, 3× |
| `screenshots/header-dirty.png` | The header with a dirty Program — asterisk shown, SAVE lit, 3× |
| `screenshots/program-buttons.png`, `assets/program-buttons.png` | The button pair together, all six lamp combinations, 3× |

Open the HTML file directly in a browser; it needs no server.
