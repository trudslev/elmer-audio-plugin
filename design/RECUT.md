# RE-CUT SHEET — ELMER GL-87

**Every row carries delivered *and* target dimensions.** A target dimension read without its
base is how three figures went wrong in this round: a needle height taken from a placement
offset, a plate "already 3×" of a canvas that no longer exists, a sprite "2×" against an old
frame. All three were true ratios with the base left out. This sheet exists so the target never
travels without it — `../MANIFEST.md` has the same rows for the whole suite.

| File | Drawn at 1× | Delivered | Ratio | **Target (3×)** |
|---|---|---|---|---|
| `assets/meter-face.png` — **REDRAWN, this bundle** | 396 × 159.2 | **1188 × 478** | 3× | **1188 × 478** |
| `assets/meter-needle.png` — **REDRAWN, this bundle** | 23.8 × 202.3 | **71 × 607** | 3× | **71 × 607** |

**Redrawn, not traced.** There was no source to cut from: the panel places the delivered bitmaps,
so the only "original" was a 2.52× image made before this round's ink pass. Both pieces are now
drawn in `Artwork Cutting Sheet.dc.html` at 1× and cut from it at 3× — the scale from the
movement rather than from the old image: pivot (198, 198), 0 dB at +63°, 20 dB at −63°, majors
every 4 dB with numerals rotated to their tick, mediums every 2, minors every 1, ink `#16150f`.
That sheet is the source of record for the next re-cut.

The needle's 1× height is **202.3**, not 197.6. The panel's `top: −197.6px` is where the pivot
sits, not how tall the image is — an earlier target of 71.4 × 593 came from reading the one as the
other. The face's 478 is 3 × 159.2 = 477.6 rounded **up**, so the face never falls short of its
well.

**Both `<img>` tags now pin height as well as width** (`396 × 159.2`, `23.8 × 202.3`). Width alone
let the new files' own rounding — 1188 / 478 and 71 / 607 — set the height, which moved the
needle's pivot by about a pixel. The placement figures in `GUI-SPEC.md` §4 are unchanged.

Nothing else in this casting is baked. GUI-SPEC §4 states the pivot and travel.


---

## Meter face — numeral radii for the next cut (§4.2)

The delivered face places all six scale numerals on **one centre radius, 156**, so clearance to the
tick swings with string length and angle: 3.44 drawn px at 4 dB down to **0.14 at 20 dB**, and at
**16 dB the tick's outer end falls inside the numeral box**. Two-digit numerals at the negative end
are the failures — a wide box at a steep angle presents its long edge to the dial.

**Cut to these radii instead.** Per-numeral, elliptically anchored by the box edge facing the dial,
3 px clear of the tick's inner end at 164.3:

| dB | Angle | Numeral box | **Centre radius** |
|---|---|---|---|
| 0 | +63.0° | 9.4 × 8.1 | **155.3** |
| 4 | +37.8° | 8.7 × 8.4 | **155.3** |
| 8 | +12.6° | 7.1 × 9.8 | **155.7** |
| 12 | −12.6° | 15.5 × 11.4 | **154.0** |
| 16 | −37.8° | 14.5 × 14.8 | **151.0** |
| 20 | −63.0° | 12.8 × 15.8 | **152.0** |

Drawn px at 1× on the 396 × 159.2 card; multiply by 3 for the delivered 1188 × 478.

**All six move inward, largest move 5 px, so nothing else on the face shifts** — the lamp fit in
§4.3 and its reference luma table are unaffected and are still what the re-cut is checked against.
**Check the inner side instead of the clearance:** the numerals approach the needle pivot and the
`GR` string, and §4 states no inner keep-out. If 16 dB is tight at 151, reduce that numeral rather
than pushing the radius back out.
