# ELMER GL-87 — GUI SPEC

**Bus compressor. Canvas 1340 × 660.** Source of record: `Elmer GL-87 Panel.dc.html`.
Artwork source of record: `Artwork Cutting Sheet.dc.html`.

Measured off the current cut, not adjusted from the previous revision. Every figure here was read
from the built panel or from the delivered bitmaps; where a figure is derived, the derivation is
shown so it can be rechecked rather than trusted.

**The meter face and needle are BITMAPS, not code-drawn.** The panel loads
`assets/elmer/meter-face.png` and `assets/elmer/meter-needle.png` as `<img>` elements. Nothing on
the meter is drawn at runtime except the needle's rotation and one sheen gradient. §4 states the
face's construction anyway, because the bitmaps were redrawn this round and the cutting sheet is
where they come from — a build that needs to re-cut them needs the figures.

---

## 1 · Canvas

| Figure | Value |
|---|---|
| Canvas | **1340 × 660** at 100 % |
| Fascia | `linear-gradient(180deg, #b3ac9d, #aca596)` |
| Panel shadow | `0 10px 40px rgba(0,0,0,.35)` |
| Header block | (16, 16), 1308 × 104 — shared part, see `shared/HEADER-PART.md` |
| Body band | y **156 → 630** |
| Footer line | (924, 640), w 400, right-aligned |
| Usable width | 1308 (16 px margin each side) |

**Height is 660 and stays there.** A band layout was built at 812 and reverted — see §7, call 6.
Every clearance in this spec is bought against a 660 panel, and the right column spends the full
520 px between header block and footer.

---

## 2 · The three columns

Two control columns flanking the meter. **Left to right the order is DETECTOR → meter → TIMING /
OUTPUT, so position carries the signal path**: detection, then timing, then output.

| Element | Position |
|---|---|
| Divider, DETECTOR / meter | x **324**, y 156 → 630, 1 px `rgba(255,255,255,.5)` |
| Divider, meter / TIMING+OUTPUT | x **1010**, y 156 → 630, same |
| DETECTOR heading | (16, 156), w 294, centred |
| TIMING heading | (1040, 156), w 268, centred |
| OUTPUT heading | (1040, 339), w 268, centred |

Headings: Barlow Condensed 600, **12 px / line box 15 / .28 em**, `#0e0d08`.

**Both dividers run the full band and are a matched pair.** They previously ended at each column's
own last row, which left the left one 74 px short; that existed to keep it clear of the scribble
strip in the bottom-left corner. The strip has moved to the centre column, so nothing is down there
to collide with and the pair reads better matched than fitted.

**Each divider sits mid-gutter**, not at a fixed offset from either side. DETECTOR's ink ends at
293 and the well starts at 355, so the rule takes 324 — 31 px of air either way. On the right the
well ends at 985 and OUTPUT's ink starts at 1035, so the rule takes 1010, 25 either way.
**Consequence: re-sizing the meter moves both rules**, and mid-gutter is the rule that says where to.

**The heading line sits 36 px below the header block.** At 6 px it read as attached to the header
rather than heading its column; 12 and 24 were both still short. All of that room came out of the
knob cell (§3), twice. OUTPUT sits **18 px above / 8 below** its own rows — a heading belongs closer
to the row it heads than to the row above it, so the spacing is deliberately asymmetric.

---

## 3 · Controls

**Eight knobs, two diameter classes, and only two.**

| Class | Ø | Controls |
|---|---|---|
| Large | **76** | THRESHOLD · RATIO · RELEASE · MAKEUP |
| Small | **56** | SIDECHAIN HP · ATTACK · IRON · MIX |

| Column | Row 1 (cy 243) | Row 2 | Row 3 |
|---|---|---|---|
| DETECTOR | THRESHOLD Ø76 (cx 83) · RATIO Ø76 (cx 226) | SIDECHAIN HP Ø56 (cx 163), cy 383 | KNEE pair (73, 489) |
| TIMING | ATTACK Ø56 (cx 1107) · RELEASE Ø76 (cx 1241) | — | — |
| OUTPUT | — | IRON Ø56 (cx 1107) · MIX Ø56 (cx 1241), cy 418 | MAKEUP Ø76 (cx 1174), cy 546 |

**Row centres:** 243 · 383 (SIDECHAIN HP) · 418 (IRON, MIX) · 546 (MAKEUP).

**Cell centres** are 83 / 226 left and 1107 / 1241 right. **A knob alone in its row is placed by its
class:** Ø76 centres on the column (MAKEUP at 1174, on the OUTPUT heading's own centre), Ø56 sits on
a cell centre. Centring is only safe for the larger class — a lone Ø56 parked between the two cell
centres reads as a **third diameter**, intermediate between the classes flanking it. The diameters
were never a third value; position was doing it.

**RATIO's cell is at 226, not 217.** Row 1 carries two Ø76 knobs, and at the nominal 134 pitch their
numeral rings collide: THRESHOLD's widest numeral reaches 148 and RATIO's would start at 150. At 226
they clear by 11.

**MAKEUP's row sits closer to the row above than the cell arithmetic allows, and it works only
because it is centred.** Its numeral band (480–491) and top ticks (494) overlap the vertical band of
IRON and MIX's legends (bottom 496) — but not their ink, because those legends sit on 1107 and 1241
while MAKEUP's top numeral sits at 1174, in the 41 px gap between them. **Moved onto a cell centre
at this height its numerals would land on the legend above it.**

### 3.1 Knob construction

| Figure | Value |
|---|---|
| Sweep | **280°, start −140°** — angle = −140 + f × 280 (per-casting freedom, `shared/BRAND-AMENDMENT-BYPASS.md`) |
| Sweep arc | Ø d + 12, `rgba(22,21,15,.30)`, 1.4 px ring, 0.7778 turn |
| Skirt | `conic-gradient(from 200deg, #e8e3da, #b6afa5 18%, #efeae1 34%, #a8a199 52%, #e6e1d8 70%, #b0a9a0 86%, #e8e3da)` |
| Cap | inset 6, `radial-gradient(circle at 34% 24%, hi, base 52%, lo)` per group |
| Pointer | 3 × (r − 7), `#f6f1e6`, radius 1.5 |
| Numbered tick | length **r + 14**, ink 9, width 2 |
| Plain tick | length **r + 10**, ink 5, width 1.5 |
| Numerals | IBM Plex Mono 500, 11 px, line box 11, `#0e0d08` |
| Unit line | 10 px / .10 em, top = d + 12 + (38 − r) |
| Legend | Barlow Condensed 600, 12 px / .18 em, top = d + 25 + (38 − r) |

**Registration box.** Unit and legend are positioned off a **Ø76 box for every class** — the
offset is `(76 − d) / 2 = 38 − r`. Pivots register on the row's Y and both legend lines land on one
baseline across both diameters.

**Cell height is derived, not chosen.** Above cy a Ø76 knob spends tick gap 5 + tick 9 + numeral
gap 3 + half the 11 px numeral line box = **66**; below cy the legend bottom lands at **+78**. Cell
**144**. The right column spends 3 × 144 + 2 × 15 headings + gaps of 36 / 6 / 18 / 8 / 10 = **520**,
which is the whole band. The cell was trimmed from 161 in two passes to buy heading clearance, and
there is no spare vertical left anywhere on a 660 panel.

### 3.2 Knob numerals are anchored by the box edge facing the dial

A numeral clears its numbered tick's outer end (r + 14) by a constant **3 px**, measured to the box
edge facing the dial — **not to the box centre**. The radial support of an upright w × h box along a
mark at angle a is `|w/2·sin a| + |h/2·cos a|`, so the ring is **elliptical, not circular**:

```
ri = r + 14 + 3 + |(len × 6.6)/2 × sin a| + |5.5 × cos a|
```

6.6 is the IBM Plex Mono 11 px advance; 5.5 is half the line box.

**At a fixed centre radius the gap is not constant — it swung 11 px.** Measured on the previous
build: `1.5:1` at −140° **overlapped its tick by 6.3**, while `0` near 3 o'clock cleared by 4.6.
Long strings at the sweep ends are the worst case, because that is where a wide box turns its long
edge to the dial. Cell height is unaffected: at a = 0 the support is exactly 5.5, so the topmost
numeral still sits at r + 28 and **above cy stays 66**.

### 3.3 Function-group cap colour

| Group | hi · base · lo |
|---|---|
| DETECTOR | `#EE5C9C` · `#D5257A` · `#8E1152` |
| TIMING | `#4FC79C` · `#17825F` · `#0C6247` |
| OUTPUT | `#6E9CE8` · `#3A6FD0` · `#1E4189` |

**Colour is organisation, not information.** Every legend reads without it, and the one-accent rule
governs live-state indicators, which a cap colour is not.

### 3.4 KNEE — a two-position shoe, not a lamp pair

**180 × 32 shoe at (73, 489)**, two 90 px halves, no gap — one track, 3 px radius, inset ring
`inset 0 0 0 1px #6d6759`. Legends `SOFT` / `HARD` on the fascia **beneath their own half** at
**10 px / line box 13 / .16 em**, `#0e0d08`; heading `KNEE` centred at y **543**. Inside the DETECTOR
column, directly beneath RATIO: it is the detector's knee, and the column keeps it with its group
rather than exiling it to a row of its own.

**The shoe carries the state. The legends never change.** Both position names print permanently, at
one weight and one ink, and neither is re-inked, re-weighted or dimmed when the shoe moves:

| Half | Face | Shadow |
|---|---|---|
| Live | `linear-gradient(180deg,#dcd6c6,#bdb6a4)` | `inset 0 1px 0 rgba(255,255,255,.55), inset 0 -2px 4px rgba(40,34,26,.18)` |
| Idle | `linear-gradient(180deg,#413b31,#2e2921)` | `inset 0 2px 5px rgba(0,0,0,.5)` |

**This replaces the lamp-lens pair, and the replacement is a conformance fix rather than a redesign.**
The catalogue's §4B names Elmer's KNEE among the controls the two-position shoe applies to, and the
previous construction instead drew two lenses whose `SOFT` / `HARD` legends were re-inked by
selection — `#ffe9f3` lit against `#150a0f` unlit. That is the panel relabelling itself to show
state: the mechanism withdrawn from the Program buttons after three rounds. The pale-metal / dark
shoe is the suite's one construction for this part, as with the Program buttons, and the group's
magenta stays where it belongs, on the knob caps.

**It also disposes of the contrast finding rather than answering it.** The unlit legend measured
4.94–6.86 against a 7.0 functional floor, and the question raised was whether to reclassify it as a
state at a 3.0 allowance or lift it off the lamp face. Neither: §4B's last clause is that **there is
no "unselected label" role in this suite — any spec carrying one is describing the withdrawn
mechanism, and the role should be deleted rather than given a floor.** Both legends now sit on the
fascia at `#0e0d08`, 7.94:1 at the fascia's darkest, and there is no unlit-legend row left to fail.
The same clause applies to Chorus-60's `legendUnlit` at 3.0, which should be deleted from its spec
rather than matched here — its `imageSwitch` already draws the shoe correctly, so only the spec row
is stale.

*(The stated range for the unlit legend was also written backwards in the source, `6.90-4.90`, high
end first, so it had never reproduced. The row is gone rather than corrected.)*

**A lamp darkens its own face when it lights**, so the legend is the bright thing, not the lens.

---

## 4 · The meter

**A bitmap in a well, with one runtime rotation and one runtime gradient. Nothing else.**

| Element | Value |
|---|---|
| Well | **630 × 254 at (355, 199)**, radius 4, `0 4px 12px rgba(35,30,22,.45)`, `0 1px 0 rgba(255,255,255,.34)` |
| Face | `assets/elmer/meter-face.png`, drawn at **630 × 253.5** at (0, 0) |
| Needle | `assets/elmer/meter-needle.png`, drawn at **9.55 × 278.4**, at (−4.77, −273.2) from the pivot |
| Pivot | **(315, 315)** — 0.5 × face width in both axes |
| Sweep | **+63° at 0 dB → −63° at 20 dB**, `needleDeg = 63 − (gr / 20) × 126` |
| Sheen | `linear-gradient(118deg, rgba(255,255,255,.10) 0 22%, rgba(255,255,255,0) 40%)` — the glass, over the lamp |
| Label row | (355, 179), w 630 — `GAIN REDUCTION METER` left, `MOVING COIL · 300 ms BALLISTIC` right |
| Caption row | (355, 457), w 630 — `STEREO LINKED · ONE DETECTOR` left, live `GR −x.x dB` right |
| Aspect | **2.4854:1, locked by the cut face** — width and height are one figure, not two |
| Top edge | **y 199, so the CREAM CARD's top lands on 205** — level with the top of the Ø76 knob bodies (row 1 cy 243 less r 38) |

**No minus on zero.** Reduction of nothing prints `GR 0.0 dB`, not `−0.0`.

**Align the card, not the well.** The face's 4 px frame is baked into the artwork and scales with it
— at 630 wide that is **6.4 px** — so a well placed at the knob-top figure puts the cream card 6 px
low and the meter reads sunken. The well is offset up by the scaled bezel. **The offset is a
function of the meter's width**: 6.4 at 630, 7.1 at 700.

**The well needs a stated gutter because it is the only hard-edged element in the body.** A knob's
cell ends in air and clears the rules on its own; the well ends in an inset rectangle with a cast
shadow, and at one point its edge sat 4 px off the hairline while every other section had clearance.

**Two copies of the face exist and both must be written.** The panel loads
`assets/elmer/meter-face.png`; the bundle carries `handoff/elmer/assets/meter-face.png`. They are
the same image and are checksum-identical. A re-cut that writes only the bundle copy leaves the
panel rendering the previous artwork **while every measurement of the bundle copy reports a match** —
which is exactly how a flat face survived two corrections. **Verify the copy the panel loads.**

### 4.1 Face construction — for re-cutting the bitmap

Drawn on a **388-wide card** (the 396 face less its 4 px frame each side) in
`Artwork Cutting Sheet.dc.html`. Delivered at **1188 × 478**.

| Figure | Value |
|---|---|
| Pivot | (194, 194) on the card — the arc, ticks and numerals share the needle's pivot |
| Arc | **R 178**, +63° to −63°, stroke **1**, `#16150f` |
| Tick count | **41 marks, 0.5 dB step**, 3.150° pitch |
| Major, every 4 dB | **13.7** long · 1.5 wide, numeral attached |
| Whole dB | **7.41** long · 0.7 wide |
| Half dB | **4.57** long · 0.6 wide |
| Tick direction | **inward from the arc only** — zero ink outside the arc radius |
| Numerals | IBM Plex Mono **500 at 13**, centre radius **≈156** |
| `dB` | 10.7 / .14 em, centred, top 96 |
| `GAIN REDUCTION` | 7.5 / .49 em, centred, top 113 |

**Four things a styled redraw gets wrong, and did:**

1. **The scale steps 0.5 dB — 41 marks, not 21.** Sampling only the whole-dB angles hides this and
   makes the 2 dB and 1 dB marks look like one class. There are three classes.
2. **Ticks run inward only.** Ticks that overhang the arc read heavier at any length.
3. **Numerals are weight 500, not 700.** At the same cap height a 700 cut carries **496 ink px²
   against the original's 341** — 45 % more. **Weight at matched cap height reads as size**, so a
   bold cut looks oversized while measuring correct. Cap height alone does not settle it; compare
   ink area.
4. **The scale shares the needle's pivot.** An earlier cut placed arc, ticks and numerals 4 px right
   of and below it, because their containers sat inside the bezel while the needle pivots on the
   whole face image. A scale that does not share the pivot means the needle does not point at its
   own marks.

**Type is matched at panel scale, not artwork scale.** The prototype's face is drawn 588 wide on its
panel; this one is drawn 630. Comparing the two bitmaps at their own widths reads correct while the
panel reads too large — the trap that produced one wrong pass. Render each face at its panel width
and measure there.

### 4.2 The face numerals sit hard against their majors — measured, outstanding

**This is a stated figure, not a defect to nudge by eye.** The face's numerals are placed on a
**fixed centre radius (~156)**, so their gap to the tick's outer end swings with string length and
angle — the same defect §3.2 fixes for the knobs, but **baked into the bitmap**. Measured on the
delivered face, nearest-point gap in drawn px from each major tick's outer end:

| dB | Angle | Numeral box | Gap to tick end |
|---|---|---|---|
| 0 | +63.0° | 9.4 × 8.1 | 2.17 |
| 4 | +37.8° | 8.7 × 8.4 | 3.44 |
| 8 | +12.6° | 7.1 × 9.8 | 3.33 |
| 12 | −12.6° | 15.5 × 11.4 | 2.66 |
| **16** | **−37.8°** | 14.5 × 14.8 | **tick end falls inside the numeral box** |
| **20** | **−63.0°** | 12.8 × 15.8 | **0.14** |

So the gap runs from 3.44 down to overlap. **The two-digit numerals at the negative end are the
failures** — 16 and 20 — because a wide box at a steep angle presents its long edge to the dial, and
a fixed centre radius does not account for that.

**The fix is the elliptical anchor from §3.2 applied to the cutting sheet, then a re-cut:**

```
ri = 178 − 13.7 − 3 − support     (support = |w/2·sin a| + |h/2·cos a|, measured off the rendered box)
```

**Applied, as six numbers rather than a rule.** The formula is evaluated below against the measured
boxes, so the re-cut places each numeral at a stated radius and nothing is applied by eye at cut
time. `support = |w/2·sin a| + |h/2·cos a|`; `ri = 164.3 − 3 − support`, where 164.3 is the tick's
inner end (178 − 13.7) and 3 is the clearance the chain asks for.

| dB | Angle | Box | support | **ri (drawn px)** | Move from 156 |
|---|---|---|---|---|---|
| 0 | +63.0° | 9.4 × 8.1 | 6.03 | **155.3** | −0.7 |
| 4 | +37.8° | 8.7 × 8.4 | 5.98 | **155.3** | −0.7 |
| 8 | +12.6° | 7.1 × 9.8 | 5.56 | **155.7** | −0.3 |
| 12 | −12.6° | 15.5 × 11.4 | 7.25 | **154.0** | −2.0 |
| 16 | −37.8° | 14.5 × 14.8 | 10.29 | **151.0** | −5.0 |
| 20 | −63.0° | 12.8 × 15.8 | 9.29 | **152.0** | −4.0 |

**The model was checked against the measurements before being trusted.** Running it at the delivered
radius of 156 predicts the gaps as 2.27 / 2.32 / 2.74 / 1.05 / −1.99 / −0.99 against the measured
2.17 / 3.44 / 3.33 / 2.66 / overlap / 0.14 — right sign everywhere, right rank order, worst
disagreement 1.6 px, and it reproduces the 16 dB overlap and the 0.14 at 20 dB that are the actual
failures. A box-edge model against a nearest-point measurement will not agree exactly; it agrees
where it has to.

**Every correction is inward and the largest is 5 px**, so the re-cut is small and the ring's inner
extent grows by at most 5 px. **That is the one thing to check on the re-cut** rather than the
clearance: the numerals move toward the needle pivot and the `GR −n.n dB` string, and nothing in
§4 states what the inner keep-out is. Measure it on the redraw; if 16 dB is tight against the pivot
at 151, the answer is a smaller numeral at the negative end, not a radius split back out.

**The radii above supersede the fixed 156 and are what the cutting sheet should carry.** The rule
stays stated in §3.2 for the knobs, which apply it at runtime; the face applies it once, at cut.

**It is not applied *to the bitmap*.** The suite's clearance chain states the rule — the numeral sits clear of the
tick's outer end, anchored by the box edge facing the dial — and this face does not meet it. Raised
as an ask rather than fixed by eye, because nudging a baked bitmap by feel is how the three wrong
lamp readings happened. **Not blocking:** the meter reads correctly and the overlap is 0–1 px at
panel scale.

### 4.3 The lamp — three wrong readings before it was right

**The face is lamplit, and the lamp lives in the bitmap.** Never in code: §0 makes the meter the
only baked element on the panel, so a runtime gradient over the face would be a second source for
one lighting decision.

Fitted to the prototype by least-squares over a 42-point grid — **centre (50 %, 40 %), radii 35 % ×
80 % of the card**, residual scatter 3.7 luma:

```css
background: radial-gradient(49% 112% at 50% 40%,
  #fffcef 0 7%, #fffbed 14%, #fdf9e9 21%, #fdf8e8 29%, #fcf6e4 36%, #faf4e1 43%,
  #f9f2df 50%, #f6eed9 57%, #f1e9d3 64%, #ebe2cc 71%, #e3dbc4 79%, #ddd4bc 86%,
  #d7ceb6 93%, #d1c7ad 100%);
```

Match to the prototype: **mean 1.4 luma, worst 7.2** across the grid.

**Reference luma, and this is the part to check a re-cut against:**

| Point | Luma | Hex |
|---|---|---|
| Centre | **250.7** | `#FFFBEE` |
| Top centre | 243.8 | `#F9F4E5` |
| Mid left · mid right | 208.7 · 202.8 | `#D9D1B9` · `#D4CBB2` |
| Top corners | 211.8 · 200.1 | `#DBD4C0` · `#D2C8AF` |
| Bottom corners | 196.0 · 195.1 | `#CEC4AA` · `#CDC3AA` |

**The three wrong readings, because the shape of the error matters more than the fix:**

1. **"No lamp — it's flat."** Sampled down the centre line only: 229 → 242, brightening downward,
   which is front light. **The centre line is the one place a bloom and a flat card agree.**
2. **"A vertical wash, brightest at the top."** Still one axis. A vertical wash leaves the top two
   corners as bright as the middle; **a lamp never does.**
3. **"Radial, but too tight and off-centre."** Fitted at 47 % / 54 % × 132 %, which dropped the
   right flank 11 luma early.

**The corners carry the evidence.** Centre-to-corner drop is **~55 luma**. If a re-cut's corners
measure within a few luma of its top centre, the lamp has been lost. Sample **both axes**.

### 4.4 Bezel

A flat frame with a **raised outer lip**, and the lip is what a redraw loses:

| Part | Top | Sides | Bottom |
|---|---|---|---|
| Outer lip, 1 px | `#918E87` | `#7B7871` | `#8C8982` |
| Body, 3 px | `#6E6A61` | `#514D45` | `#68645B` |

Darkest at the sides, lightest along the bottom lip. **No white inner rule** — a white highlight
inside the card edge is the inverse of this frame and turns the meter into a raised shape rather
than a recessed instrument behind glass.

### 4.5 Needle

| Figure | Value |
|---|---|
| Delivered | **18 × 525** |
| Drawn on the 396 face | length **175.4**, tip **1.6** wide, base **4.4**, hub **6.3** |
| Tip radius | **171.7 — 6.3 short of the arc**, just past the inner ends of the majors |
| Ink | `linear-gradient(90deg, #24211a, #14120d 45%, #24211a)`, hub `radial-gradient(circle at 36% 30%, #2b2820, #15130e 60%, #0d0c08)` |

**Measure the old needle's ink, not its image box.** The prototype's image is 60 × 510 with **67
rows of transparent padding**; cutting to the image box makes a needle 15 % too long.

---

## 5 · The scribble strip

`CH 24 — MIX BUS / GLUE` — Permanent Marker **28.5 px**, `#2b2a26`, on torn cream tape, rotated
**−2.4°**, at **(396, 566)** in the centre column, about 45 px left of its centre line. Tape:
`linear-gradient(178deg,#efe9d6,#ded7c0 55%,#e6dfca)`, torn edge by `clip-path`, padding
10.5 / 39 / 13.5 / 42.

**It is one of this casting's identity marks** and the thing that reads console module rather than
rack unit. **It was never retired**: it existed in the 1120 × 776 prototype's footer and fell out of
a re-layout unrecorded. Scaled 1.5× from its original 19 px this round — padding and tracking scaled
with the type so the tape grows proportionally rather than the type outgrowing it.

The serial and version consolidate into **one right-aligned footer line** at (924, 640) —
`GL-87 · SN 0042 · v1.0`, IBM Plex Mono 500, 10 px / .18 em, `#34322a`.

---

## 6 · Type and contrast

| Role | Face | Size / tracking |
|---|---|---|
| Wordmark | **Archivo 700, width 125 %** — the variable face at `wdth 125, wght 700`, **not Archivo Black** | 31 / line 34 / .10 em |
| Model line | Barlow Condensed 600 | 14 / 17 / .26 em |
| Model number | IBM Plex Mono 500 | 11 / 14 / .20 em |
| Group headings | Barlow Condensed 600 | 12 / 15 / .28 em |
| Knob legends | Barlow Condensed 600 | 12 / 15 / .18 em |
| Knob units | IBM Plex Mono 500 | 10 / 13 / .10 em |
| Knob numerals | IBM Plex Mono 500 | 11 / 11 |
| Meter label + caption | IBM Plex Mono 500 | 11 / 14 / .14 em |
| Readouts | Share Tech Mono | 17 / 22 / .10 em |
| Button lamps | Barlow Condensed 600 | 11 / 13 / .12 em |
| Footer | IBM Plex Mono 500 | 10 / 13 / .18 em |
| Scribble strip | Permanent Marker | 28.5 / 1 |

**The wordmark's 31 px is measured on `Archivo` at `wdth 125, wght 700` and is valid only for that
instance.** `Archivo Black` is a separate static family at weight 900 with no width axis; it appears
in the panel's font stack as a **fallback**, never as the specified face. **31 px does not transfer to
it** — the two em sizes are unrelated, and adopting one against the other renders the wordmark
narrower and heavier at once. **The face now ships:** `elmer/fonts/Archivo_Expanded-Bold.ttf` (`wdth` 125 · `wght` 700), with the
variable `Archivo-VariableFont_wdth_wght.ttf` beside it. **31 px is directly implementable and this
row can be adopted with the rest of the table.** With the static, **drop `font-stretch: 125%`** — the
width is in the file, and leaving it applies the stretch twice or not at all depending on the
renderer. See `shared/FONTS.md`.

### Contrast, computed against this casting's own grounds

| Ink | Ground | Ratio |
|---|---|---|
| `#0e0d08` knob legends, headings | fascia `#aca596` (darkest) | **7.94:1** |
| `#0e0d08` | fascia `#b3ac9d` (lightest) | **8.62:1** |
| `#0e0d08` meter label, left | fascia at y 179 | **8.32:1** |
| `#2d2b24` meter caption, right label | fascia | **6.06:1** |
| `#34322a` footer | fascia foot | **5.24:1** |
| `#16150f` face scale ink | lamp centre `#fffcef` | **17.79:1** |
| `#16150f` face scale ink | lamp corner `#d1c7ad` | **10.87:1** |
| `#e6dcae` readouts | LCD well `#1b1a16` | **12.62:1** |
| `#0e0d08` KNEE shoe legends, both | fascia `#aca596` (darkest) | **7.94:1** |
| `#9aa1a6` header lamp, idle | button face | **6.34:1** |
| `#f4f8fa` header lamp, lit | button face | **15.53:1** |
| `#2b2a26` scribble strip | tape | **9.98:1** |

**Everything clears 4.5:1, and the floor is the footer at 5.24.** The face's scale ink is checked at
the lamp's **corner** as well as its centre, because the lamp is a bloom and the corner is the dim
end of its own ground.

---

## 7 · Changelog — what moved and why

0. **Quadrant grid retired, canvas unchanged at 660** — meter well 396 × 159 → **630 × 254**, from
   30 % of the usable width to 48 % and from 0.33× the cut face to 0.53×. Controls in two flanking
   columns; dividers at 324 / 1010.
0a. **A band layout at 992 × 399 was built and reverted.** It grew the panel to 812 on the argument
   that TapeRot and Chorus-60 give their signature displays most of the panel width. **That
   comparison does not hold:** those are wide thin strips — TapeRot's scope 1308 × 164, Chorus-60's
   1039 × 120 — where width costs almost no height. **A VU face is 2.4854:1**, so 1039 wide forces
   418 tall, half the panel. Width and height are not independent for this part, and a strip's width
   cannot be compared with a rectangle's.
0b. **Scribble strip restored, then moved and scaled** — see §5. It was never retired; it fell out
   of a re-layout unrecorded.
1. **Canvas 1120 → 1340** (call 1), four sections re-spaced.
2. **Sprite constants corrected** — the panel described a superseded asset pair (1000 px face, 60 px
   needle) against real assets of 1188 × 478 and, then, 71 × 607, which put the needle off the face.
3. **Knobs grew** into the wider canvas; registration box introduced so two diameter classes share
   one legend baseline.
4. **Readout stopped printing a minus on zero**; footer stopped running through OUTPUT's scale.
5. **Face lamp and bezel refitted** to the prototype — see §4.3 and §4.4.
6. **Dial re-cut to measured figures** — 41 ticks in three classes, numerals to weight 500, needle
   re-cut from ink rather than image box. Thirteen figures within 1.6 drawn px.
7. **Knob numerals re-anchored** to the box edge facing the dial (§3.2), removing an 11 px swing.
8. **Heading clearance bought twice** out of the knob cell, 161 → 144; MAKEUP centred on its column
   and raised; KNEE lowered 12; dividers matched; meter aligned to the Ø76 knob tops and reduced
   10 % to 630 × 254.

---

## 8 · Conformance

| Call | Requirement | State |
|---|---|---|
| 1 | Canvas 1340, four sections at full width | **Met** — superseded by the three-column layout, which keeps the four section labels |
| 2 | Sprite constants match delivered assets | **Met** — face 1188 × 478, needle 18 × 525, both verified against the files the panel loads |
| 3 | Knob geometry, registration, two diameter classes | **Met** — §3; two classes only, verified after MIX and MAKEUP were repositioned |
| 4 | No minus on zero; footer clear of scale | **Met** — §4, §5 |
| 4B | Indicating mechanism stays live, not pre-baked | **Met** — the needle is a live rotation over a baked face; only the face is a bitmap |
| 5 | Lamp lives in artwork, not code | **Met** — §4.3; one sheen gradient at runtime, nothing else |
| 6 | Meter carries the panel | **Met** — 48 % of usable width, 0.53× the cut face, top aligned to the Ø76 knob tops |
| — | Clearance chain: numeral clear of tick's outer end, anchored by the box edge facing the dial | **Met for the knobs** (§3.2). **NOT met for the face bitmap** (§4.2) — 16 dB overlaps, 20 dB clears by 0.14. Raised as an ask; not blocking |
| — | Contrast ≥ 4.5:1 on this casting's own grounds | **Met** — §6, floor 5.24:1 |
| — | Both copies of every bitmap written | **Met** — checksum-identical, §4 |

**One item outstanding: §4.2.** It needs a design decision and a re-cut, not a nudge.
