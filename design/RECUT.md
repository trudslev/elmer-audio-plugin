# RE-CUT SHEET — ELMER GL-87

**Every row carries delivered *and* target dimensions.** A target dimension read without its
base is how three figures went wrong in this round: a needle height taken from a placement
offset, a plate "already 3×" of a canvas that no longer exists, a sprite "2×" against an old
frame. All three were true ratios with the base left out. This sheet exists so the target never
travels without it — `../MANIFEST.md` has the same rows for the whole suite.

| File | Drawn at 1× | Delivered | Ratio | **Target (3×)** |
|---|---|---|---|---|
| `assets/meter-face.png` — **REDRAWN, this bundle** | 396 × 159.2 | **1188 × 478** | 3× | **1188 × 478** |
| `assets/meter-needle.png` — **REDRAWN; re-cut from ink, not image box** | 6 × 175 | **18 × 525** | 3× | **18 × 525** |

**Redrawn, not traced.** There was no source to cut from: the panel places the delivered bitmaps,
so the only "original" was a 2.52× image made before this round's ink pass. Both pieces are now
drawn in `Artwork Cutting Sheet.dc.html` at 1× and cut from it at 3× — the scale from the
movement rather than from the old image: pivot (198, 198), 0 dB at +63°, 20 dB at −63°, majors
every 4 dB with numerals rotated to their tick, mediums every 2, minors every 1, ink `#16150f`.
That sheet is the source of record for the next re-cut.

**The needle row above is superseded twice over and this paragraph is the history, not the figure.**
It read 23.8 × 202.3 / 71 × 607 until export 7 — the needle *before* it was re-cut from the
prototype's ink instead of its image box, the old image carrying 67 rows of transparent padding. The
delivered file measures **18 × 525** in all three copies; the panel places it at 9.55 × 278.4 inside
a face shown at 1.591× drawn, so the drawn needle is **6 × 175** and the ratio is a clean **3×**.
*(The old dispute over 202.3 against 197.6 — image height against the `top: −197.6px` pivot offset —
is settled by the re-cut and applies to neither figure now. It is kept because it is the reason the
row was wrong twice: both wrong values came from reading a placement number as a dimension.)*
**`Artwork Cutting Sheet.dc.html` now draws the block 6 × 175**, so the delivered 18 × 525 divides by
exactly 3 in both axes. The **6 is the hub**, the widest ink on the part; the blade's measured
1.6 tip / 4.4 base is what governs and the clip-path percentages are derived from it
(13.33 / 86.67 / 36.67 / 63.33 %).

**The sheet now computes every needle figure it prints from five measurements** — hub 6, blade length
169, tip ink 1.6, base ink 4.4, scale 3 — instead of carrying them as prose beside the block. Block
size, delivered size, clip-path percentages, the pivot at the hub centre, the tip radius and the
clearance to the arc are all derived. **Three consecutive corrections went into that one paragraph**
before this: the blade percentages kept for a 6.4 box (tip 0.10 short), the heading's `19 × 526`
rounded from the same stale box, and a tip radius of 171.7 / 6.3 short of the arc that belonged to
neither the old block nor the new one. The pivot is the hub centre at 169 + 3, so the tip radius is
**172** and it stops **6** short of the arc at 178. A "2.1× thinner" with no definition is gone; the
comparison to the superseded cut is now stated per feature — **2.28× at the hub** (13.7 → 6) and
1.52× at the blade base (6.7 → 4.4). The face's 478 is 3 × 159.2 = 477.6 rounded **up**, so the face never falls short of its
well.

**Both `<img>` tags now pin height as well as width** (face `630 × 253.5`, needle `9.55 × 278.4`, both in panel px). Width alone
let the new files' own rounding — 1188 / 478 and 71 / 607 — set the height, which moved the
needle's pivot by about a pixel. The placement figures in `GUI-SPEC.md` §4 are unchanged.

Nothing else in this casting is baked. GUI-SPEC §4 states the pivot and travel.


---

## Meter face — numeral radii, CUT (§4.2)

The delivered face places all six scale numerals on **one centre radius, 156**, so clearance to the
tick swings with string length and angle: 3.44 drawn px at 4 dB down to **0.14 at 20 dB**, and at
**16 dB the tick's outer end falls inside the numeral box**. Two-digit numerals at the negative end
are the failures — a wide box at a steep angle presents its long edge to the dial.

**Cut. `assets/meter-face.png` carries these radii from export 7.** Per-numeral, elliptically anchored
by the box edge facing the dial, 3 px clear of the tick's inner end at 164.3:

| dB | Angle | Numeral box | **Centre radius** |
|---|---|---|---|
| 0 | +63.0° | 9.4 × 8.1 | **155.3** |
| 4 | +37.8° | 8.7 × 8.4 | **155.3** |
| 8 | +12.6° | 7.1 × 9.8 | **155.7** |
| 12 | −12.6° | 15.5 × 11.4 | **154.0** |
| 16 | −37.8° | 14.5 × 14.8 | **151.0** |
| 20 | −63.0° | 12.8 × 15.8 | **152.0** |

Drawn px at 1× on the 396 × 159.2 card; multiply by 3 for the delivered 1188 × 478.

**`Artwork Cutting Sheet.dc.html` draws these radii too**, per numeral rather than the single 155.2 it
had for all six — it is the source of record for the next re-cut, so a sheet still showing one radius
would hand the next cutter the defect back.

**All six move inward, largest move 5 px, so nothing else on the face shifts** — the lamp fit in
§4.3 and its reference luma table are unaffected and are still what the re-cut is checked against.
**Check the inner side instead of the clearance:** the numerals approach the needle pivot and the
`GR` string, and §4 states no inner keep-out. If 16 dB is tight at 151, reduce that numeral rather
than pushing the radius back out.


### How it was cut, and what to check

**The face was edited, not redrawn.** There was no vector source, and redrawing was the option that
would have put §4.3's lamp fit back at risk — it took three wrong readings to land. So the three
numerals that failed were moved in the bitmap and nothing else was touched:

1. **Pivot recovered from the ink.** The six numeral centroids fit a dial centre at **(594.0, 592.9)**
   in delivered px with the six angles ±63 / ±37.8 / ±12.6, three independent pairs agreeing on the
   x centre to 0.6 px, and a numeral radius of **465.7 delivered = 155.2 drawn** — the "~156" §4.2
   states, measured rather than assumed. The pivot sits 38 drawn px below the card's bottom edge,
   which is why the card shows only the top of the arc.
2. **Corrections computed against the measured radius, not the nominal one.** `0`, `4` and `8` come
   out within 1.4 delivered px of where they already are, so they were left alone: moving ink to
   gain half a drawn pixel is a re-cut for nothing. Only **12, 16 and 20** moved — inward along
   their own radius by **3.7 / 12.7 / 9.7 delivered px**.
3. **Erased by diffusion, not by patching.** The first attempt filled each numeral's bounding box
   from a fitted background plane and left a visible rectangle — the lamp is radial and a plane is
   not. The ink was instead isolated per glyph (connected components, excluding the arc), the glyph
   pixels alone marked as holes, and the holes solved to the surrounding pixels. Stroke-width holes
   converge, and the boundary is exact, so there is no seam. Antialiasing was carried through as an
   alpha channel and recomposited on the ink colour, sampled bilinearly for the sub-pixel move; ink
   area is preserved to within 0.15 %.

**Measured on the cut file, clearance gained in drawn px:** 12 **+1.27**, 16 **+3.85**, 20 **+3.27**.
Against §4.2's stated figures that puts 12 at ~3.9, 16 at ~3.2 from an overlap, and 20 at ~3.4 from
0.14 — all clear of the 3 px the chain asks for. `0`, `4` and `8` are byte-identical.

**Two things to check on this file rather than trust:**

- **The inner keep-out.** All three numerals moved toward the needle pivot and the `GR −n.n dB`
  string. §4 states no inner limit, so it has not been checked against one.
- **Edge softness.** A sub-pixel move resamples the glyph, so 12, 16 and 20 are marginally softer
  than `0`, `4` and `8` — about 3 % fewer pixels below a lum-70 threshold. It is not visible at
  panel scale on this face; it is the cost of moving baked ink and the reason to move it once.

**The lamp is untouched**, so §4.3's reference luma table still applies unchanged and is still what a
future re-cut is checked against.
