# ELMER — GUI SPEC

Model **GL-87**, stereo bus compressor. Neon Foundry casting, harmonisation round.
Authoritative for the build.

**Read `shared/HEADER-PART.md` first.** The block, the band, the LCD cell with its budget
and cap, the Program buttons and their state matrix, and the meter wells are the shared
part and are not restated here except where this casting's material meets them.

**Asset format: vector / code-drawn, with two exported bitmaps.** The fascia, every label,
tick, numeral, knob, cap, lens and legend is drawn at runtime. The gain-reduction meter
ships as artwork — `assets/elmer/meter-face.png` and `meter-needle.png` — and is the only
baked element on the panel. Nothing carrying a live value is baked: the needle is a
separate image rotated at runtime over a static face.

---

## 1 · Canvas

| Figure | Value |
|---|---|
| Canvas | **1340 × 660** at 100 % |
| Header block | 16, 16, 1308 × 104 — shared part |
| Body origin | y **120** |
| Vertical divider, detector | x 500, y 136 → 380 |
| Horizontal divider | y 386, x 16 → 1324 |
| Vertical divider, lower | x 700, y 396 → 644 |

Fascia `linear-gradient(180deg, #b3ac9d, #aca596)` — painted steel, warm grey.
Header block `linear-gradient(180deg, #b6afa0, #ada697)` with
`inset 0 1px 0 rgba(255,255,255,.5)` and `0 3px 5px rgba(0,0,0,.14)`: the block reads as a
raised panel on the same casting rather than a different material.

**Call 1 brought +220 px** (1120 → 1340). It paid for the four-section layout at full
width and for the knobs growing under call 3.

---

## 2 · Sections and the function-group colour

Four sections, headings Barlow Condensed 600 **12 px / line box 15 / .28 em**, `#0e0d08`:
**GAIN REDUCTION METER** (60, 150) · **DETECTOR** (510, 150) · **TIMING** (26, 412) ·
**OUTPUT** (710, 412).

**Cap colour is function-group coding, carried forward and unchanged by this round.**

| Group | Highlight · base · shade | Controls |
|---|---|---|
| DETECTION | `#EE5C9C` · `#D5257A` · `#8E1152` | THRESHOLD, SIDECHAIN HP, RATIO, KNEE lenses |
| TIMING | `#4FC79C` · `#17825F` · `#0C6247` | ATTACK, RELEASE |
| OUTPUT | `#6E9CE8` · `#3A6FD0` · `#1E4189` | IRON, MAKEUP, MIX |

Colour is organisation, not information: every legend reads without it, and the
one-accent rule governs live-state indicators, which a cap colour is not.

---

## 3 · Knobs — two classes on a 280° sweep

| Figure | Value |
|---|---|
| Sweep | **280°**, start **−140°**, angle = `−140 + 280 f` |
| Classes | **Ø76 primary** (THRESHOLD, RATIO, RELEASE, MAKEUP) · **Ø56 standard** (SIDECHAIN HP, ATTACK, IRON, MIX) |
| Numerals | five on primary, three on standard |
| Skirt | `conic-gradient(from 200deg, #e8e3da, #b6afa5 18%, #efeae1 34%, #a8a199 52%, #e6e1d8 70%, #b0a9a0 86%, #e8e3da)` — machined aluminium |
| Cap | `inset: 6px`, `radial-gradient(circle at 34% 24%, hi, base 52%, lo)` in the group's three stops |
| Pointer | 3 × (r − 7), `#f6f1e6`, `0 0 2px rgba(0,0,0,.45)` |
| Sweep arc | 280° conic wedge `rgba(22,21,15,.30)`, masked to a 1.4 px ring |
| Ticks | major **2 × 9**, minor **1.5 × 5**, ink `#16150f` — **7.47:1** on fascia |
| Numerals | IBM Plex Mono 500 **11 px / 13**, `#0e0d08`, on a ring at `r + 29.5` |

**Elmer's sweep is 280°, not the suite's 270°** — per-casting sweep freedom, and it is why
its mark angles cannot be copied from another casting's table.

### 3.1 Registration

Both control bands mix classes, so the unit and label are positioned off a **Ø76
registration box for every class**: `dy = (76 − Ø) / 2`, i.e. `unitTop = Ø + 20 + dy`,
`labelTop = Ø + 34 + dy`. The label registers on the box (one line per band) and the ring
registers on itself (pivots on one Y).

**This panel was drawn before that was understood.** It held the label baseline and missed
the pivot by exactly 10 px in both bands — its Ø56 cells were bottom-aligned, which is why
the labels were right and the centres were not. Corrected by bringing the four standard
pivots back onto their band's Y: **DETECTOR on 262** (three pivots, three labels on one
line) and **TIMING/OUTPUT on 524** (five pivots, five labels). No label moved.

### 3.2 Mark lists

Angle = `−140 + 280 f`. Numerals in **bold**; other rows are minor ticks at real values.

| Knob | Ø | f · printed |
|---|---|---|
| THRESHOLD | 76 | 0 **−40** · .2 −30 · .4 **−20** · .6 −10 · .8 **0** · 1 **+10** — dB |
| SIDECHAIN HP | 56 | 0 **OFF** · .2 40 · .4 75 · .6 **140** · .8 265 · 1 **500** — Hz |
| RATIO | 76 | 0 **1.5:1** · .25 **2:1** · .5 **4:1** · .75 **10:1** · 1 **20:1** |
| ATTACK | 56 | 0 **0.1** · .1926 0.3 · .4037 **1** · .5963 3 · .8074 10 · 1 **30** — ms |
| RELEASE | 76 | 0 **0.1 s** · .25 **0.3 s** · .5 **0.6 s** · .75 **1.2 s** · 1 **AUTO** |
| IRON | 56 | 0 **0** · .25 25 · .5 **50** · .75 75 · 1 **100** — % |
| MAKEUP | 76 | 0 **0** · .25 **5** · .5 **10** · .75 **15** · 1 **20** — dB |
| MIX | 56 | 0 **0** · .25 25 · .5 **50** · .75 75 · 1 **100** — % |

ATTACK's fractions are the build's skew and **must not be evened out**. RELEASE's last
position is a word, `AUTO`, not a number — it is a real detent at f 1.0 and prints as
typed. `−` is U+2212; THRESHOLD's `+10` keeps its leading plus. Units print inside the
arc's bottom gap, never as a suffix on the control name.

---

## 4 · Gain-reduction meter

The casting's signature display and its only analogue movement.

| Figure | Value |
|---|---|
| Well | 396 × 159 at (60, 176), radius 4, `0 4px 12px rgba(35,30,22,.45)`, `0 1px 0 rgba(255,255,255,.34)` |
| Face | `assets/elmer/meter-face.png` — **delivered 1000 × 402**, drawn at 396 × 159.2 (**2.525×**) |
| Needle | `assets/elmer/meter-needle.png` — **delivered 60 × 510**, drawn at 23.8 × 202.3 (**2.522×**), pivot at (198, 198) — **0.5 × face width below the top edge**. The element's `top: −197.6px` is a placement offset, not the image height |
| Travel | **+63° at 0 dB to −63° at 20 dB**, `needleDeg = 63 − (gr / 20) × 126` |
| Glass | `linear-gradient(118deg, rgba(255,255,255,.10) 0 22%, transparent 40%)` |
| Caption | `GAIN REDUCTION METER`, IBM Plex Mono 500 11 / 14 / .14 em, `#0e0d08` |
| Sub-caption | `STEREO LINKED · ONE DETECTOR`, same face, `#2d2b24` — flavour, 5.78:1 |

**The needle rests at zero reduction and swings anticlockwise** — reduction pulls it left,
which is the movement direction on the hardware this is derived from. Live, drawn at
runtime, never baked into the face.

---

## 5 · KNEE — two lamp lenses, one hue

180 px pair at (1100, 266), two 34 px lenses with a 12 px gap, legend `KNEE` centred
below at (1100, 334).

| State | Face | Legend | Measured |
|---|---|---|---|
| Lit | `linear-gradient(#7a1244, #4d0a2b)` + `inset 0 0 12px 2px rgba(213,37,122,.45)` | `#ffe9f3` + 7 px bloom | **9.11:1** light end · **13.06:1** dark end |
| Unlit | `linear-gradient(#d2b9c6, #b79dab)` | `#150a0f`, flat | **10.62:1** light end · **7.78:1** dark end |

**A lamp darkens its own lens when it lights, so the legend is the bright thing.** Lit and
unlit are separated by brightness *within the DETECTION magenta*, never by a change of
hue — the unlit lens is a pale magenta lens, not a grey one. Light stops at the lens edge:
no halo on the fascia.

Both legends are printed permanently. The lens indicates state; it does not relabel the
control.

---

## 6 · Palette and measured contrast

Computed in one pass from this panel's own hexes against each ground **by name**, worst
case quoted where a ground is a gradient. Functional 7:1, flavour 4.5:1, state 3:1.
Re-measure rather than transcribe; if a figure here disagrees with yours, yours wins.

### On fascia (worst `#aca596`) and header block (worst `#ada697`)

| Ink | Role | Ratio | Class |
|---|---|---|---|
| `#0e0d08` | section headings, control labels, units, header captions, KNEE label, meter caption | **7.94** fascia · **8.04** block | functional |
| `#16150f` | tick marks | **7.47** fascia | functional |
| `#0f0f0c` | model line | **7.93** block | functional |
| `#26221a` | wordmark (31 px, moulded relief) | **6.54** block | large-text role — see below |
| `#2d2b24` | meter sub-caption | **5.78** fascia | flavour |
| `#34322a` | serial `GL-87 · SN 0042`, version stamp | **5.24** fascia | flavour |

**The model line was `#2d2b24` at 5.85 on the block's dark end — under the functional
floor — and is now `#0f0f0c`, 7.93.** It was briefly `#16150f` (7.56) in this round: the
six-material header strip had already fixed the same role to `#0f0f0c` and the body had not
inherited it, so two artefacts held two right answers. Reconciled to the strip's. Same defect and same third-of-a-stop as Gatecrasher's, on
a different fascia; per-role figures against each named ground are what surfaced both.

**The wordmark is a moulded relief, not flat ink.** At 31 px / 700 it is large text, where
the floor is 3:1, and its paint fill measures 6.54 against the block's dark end with the
relief's own highlight (`0 1px 0 rgba(255,255,255,.55)`) adding separation above that. It
is recessed and paint-filled with no outer cast shadow — the shadow pair is *inside* the
letterform.

### On LCD glass (`#1b1a16 → #242219`)

| Ink | Role | Ratio |
|---|---|---|
| `#e6dcae` | program name, bank tag, live readout, meter values, chevron | **12.62** light end · **11.54** dark end |

### On the Program cap (`#23282c → #14181b`)

| Ink | State | Ratio |
|---|---|---|
| `#f4f8fa` | lit | **13.93** light end · **16.71** dark end |
| `#9aa1a6` | idle | **5.68** light end · **6.82** dark end |

### Pointer against its cap — a graphic, and the one thin figure

| Cap | Ratio |
|---|---|
| DETECTION `#D5257A` | 4.26 |
| OUTPUT `#3A6FD0` | 4.27 |
| TIMING `#17825F` | **4.24** |

**The TIMING base was `#1B9E74` and measured 3.01** — a one-of-three rather than a floor
breach, since the pointer is a graphic with no text floor and carries a
`0 0 2px rgba(0,0,0,.45)` halo the ratio does not capture. It is now **`#17825F`, 4.24**,
in parity with its two siblings. `#146B4F` (5.74) was rejected: it would have made TIMING
the *brightest*-separated pointer on the panel, trading one inconsistency for another.
Neither option changes the hue, so the group coding survives either way. **Whether 4.26 is
itself thin is a suite question about pointer contrast, not a Chorus-60 or Elmer question**,
and belongs in the catalogue.

### Accent

Elmer's live-state indication is the KNEE lens and the meter needle. No separate accent
colour is used anywhere on this panel.

---

## 7 · State matrices

### 7.1 Program legends — shared part

| Panel state | SAVE | STORE | DELETE | CANCEL |
|---|---|---|---|---|
| Factory Program, unmodified | idle | idle | idle | idle |
| Factory Program, edited | **lit** | idle | idle | idle |
| User Program, unmodified | idle | idle | **lit** | idle |
| User Program, edited | **lit** | idle | **lit** | idle |
| Naming a Program | idle | **lit** | idle | **lit** |

Weight 600 in all twenty cells; only illumination changes. No disabled face.

### 7.2 KNEE

| Knee | SOFT lens | HARD lens |
|---|---|---|
| Soft | **lit** | unlit |
| Hard | unlit | **lit** |

Both legends printed in both states. The pair is exclusive — there is no third state and
no "neither".

### 7.3 Meter

| Reduction | Needle |
|---|---|
| 0 dB | **+63°** (rest, right end stop) |
| 10 dB | 0° |
| 20 dB | **−63°** |

Clamped at both ends; values beyond 20 dB hold the end stop rather than over-swinging.

### 7.4 Bypass

Host-driven, no on-panel control. **A 0.50 `#808080` multiply over the body only** — the
header stays lit, which is this casting's one departure from the suite's full-bleed
multiply and is deliberate: Elmer's header is a raised sub-panel with its own material, and
darkening it reads as a second unit rather than as one unlit one. Pointers do not move, the
needle holds, no caption, no desaturation. The legibility floors do not apply in this state.

---

## 8 · Type

Every size is a CSS px em size with a pinned line box (call 4).

| Role | Face | Size / line box | Tracking | Ink |
|---|---|---|---|---|
| Wordmark | Archivo 700, stretch 125 % | 31 / 34 | .10 em | `#26221a` |
| Function descriptor | Barlow Condensed 600 | 14 / 17 | .26 em | `#0e0d08` |
| Model line | IBM Plex Mono 500 | 11 / 14 | .20 em | `#0f0f0c` |
| Section heading | Barlow Condensed 600 | 12 / 15 | .28 em | `#0e0d08` |
| Control label · KNEE label | Barlow Condensed 600 | 12 / 15 | .18 em | `#0e0d08` |
| Unit | IBM Plex Mono 500 | 10 / 13 | .10 em | `#0e0d08` |
| Scale numeral | IBM Plex Mono 500 | 11 / 13 | 0 | `#0e0d08` |
| Meter caption | IBM Plex Mono 500 | 11 / 14 | .14 em | `#0e0d08` |
| Meter sub-caption | IBM Plex Mono 500 | 11 / 14 | .14 em | `#2d2b24` |
| KNEE lens legend | Barlow Condensed 600 | 11 / 13 | .14 em | see 7.2 |
| LCD / meter value | Share Tech Mono | 17 / 22 | .10 em | `#e6dcae` |
| Program legend | Barlow Condensed 600 | 11 / 13 | .12 em | see 7.1 |
| Serial · version | IBM Plex Mono 500 | 10 / 13 | .18 em | `#34322a` |

**Elmer paid the whole cost of call 7.** Its printed knob legends, lamp legends, meter
header and serial line were IBM Plex Mono; panel lettering is now Barlow Condensed 600
throughout. **Numerals, units and the model line stay in IBM Plex Mono** — the casting's
own mono, per call 7's split — and the wordmark is the nameplate metaphor, outside the call.

---

## 9 · Conformance — calls this casting already satisfied

**§9 and §10 together account for every call.** A call appearing in neither this section
nor the changelog is a gap by construction, not an omission.

| Call | State |
|---|---|
| **3's signature class** | **checked, and Elmer takes no Ø104.** It has no MODEL control; the KNEE pair is a lamp switch, not a detented selector, so the signature diameter would land on nothing that earned it. Two classes is the intended reading, not a shortfall. |
| **5** — code-drawn, cached, no filmstrips | **already conformed** in artwork: rings, ticks, numerals and pointer were always drawn from rotation fractions. Its filmstrip sheets are retired by the call; `setBufferedToImage` is the build's to add. |
| **§4B shoes** | **not applicable, checked.** Elmer has no two- or three-state shoe — its only multi-state control is the KNEE lamp pair, which is the lamp part rather than the shoe part. |
| **Lamps** — light stops at the lens edge, lit darkens the lens, unlit stays in its own hue | **already conformed** on the KNEE lenses, and this casting is where the rule was derived. |
| **Baseline rule** — labels of different size classes on a shared line | **already conformed**, and it is the half of §3.1 this panel had right. The pivot half is the correction. |

---

## 10 · Changelog and outstanding

### This round

1. **Canvas 1120 → 1340** (call 1), four sections re-spaced with dividers at 500 / 386 / 700.
2. **Ø84 → Ø76 primary and Ø74 → Ø56 standard** (call 3); standard-class numeral counts cut
   to three with the demoted values kept as minors at their real angles.
3. **Panel lettering to Barlow Condensed 600** (call 7) — the largest visible change after
   the canvas: knob legends, lamp legends, meter header and serial line all moved off
   IBM Plex Mono.
4. **LCD face to Share Tech Mono 17 / .10 em** (call 2); cap **22 → 47**, the largest rise
   in the suite.
5. **Sizes resolved to whole pixels with pinned line boxes** (call 4) — the 10.5 / 11.5
   pairs became 11 and 12.
6. **Both bands re-registered** (§3.1): four standard pivots onto their band's Y, no label
   moved.
7. **Wordmark re-cut as paint-filled relief** — moulded and recessed, no outer cast shadow.
8. **KNEE lamp inverted**: it now darkens its lens when lit, with lit and unlit separated
   by brightness inside the DETECTION hue.
9. **LCD chevron re-drawn** as the shared 14 × 8 stroked path, replacing a 9 × 9 rotated box.
10. **Model line `#2d2b24` → `#0f0f0c`**, 5.85 → 7.93 on the block's dark end, reconciled
   with the six-material strip which had already carried this fix.
11. **TIMING cap base `#1B9E74` → `#17825F`** for pointer parity, 3.01 → 4.24.

### Outstanding

- **Both meter assets need re-cutting at 3×** per call 6, and §4 now states each asset's
  delivered pixels beside its drawn size so the ratio is re-derived rather than transcribed.
  The current cuts are **2.525×** (face, 1000 × 402) and **2.522×** (needle, 60 × 510) —
  *not* 2×, which an earlier draft of this section stated. Targets: `meter-face.png`
  **1188 × 478** (3 × 396 × 159.2 = 1188 × 477.6, rounded up so the face never falls short
  of its well) and `meter-needle.png` **71.4 × 607** (3 × 23.8 × 202.3). An earlier
  draft gave the needle as 71.4 × 593, taking its height from the panel's `top: −197.6px`
  placement offset instead of the image's own 202.3 px — the fourth wrong figure recorded
  against this needle, which is why both assets now carry their delivered pixels.
- **The TIMING cap's pointer contrast**, §6 — 3.01 measured, two costed alternates, your call.
- Wire both meter wells and the gain-reduction meter to real metering; the render shows
  `−8.4` / `−6.1` and a 5 dB needle as sample values.
- Confirm ATTACK's skew against the build's `NormalisableRange` before the marks are final.
- **`shared/HEADER-PART.md` revision 3 is pending three build answers** — the meter's
  display clamp, its format at both ends, and the sign convention. Nothing on this panel
  changes either way.
