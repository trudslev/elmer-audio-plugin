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
