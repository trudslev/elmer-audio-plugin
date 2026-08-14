# RE-CUT SHEET — ELMER GL-87

**Every row carries delivered *and* target dimensions.** A target dimension read without its
base is how three figures went wrong in this round: a needle height taken from a placement
offset, a plate "already 3×" of a canvas that no longer exists, a sprite "2×" against an old
frame. All three were true ratios with the base left out. This sheet exists so the target never
travels without it — `../MANIFEST.md` has the same rows for the whole suite.

| File | Drawn at 1× | Delivered | Ratio | **Target (3×)** |
|---|---|---|---|---|
| `assets/meter-face.png` | 396 × 159.2 | **1000 × 402** | 2.525× | **1188 × 478** |
| `assets/meter-needle.png` | 23.8 × 202.3 | **60 × 510** | 2.522× | **71 × 607** |

The needle’s 1× height is **202.3**, not 197.6. The panel’s `top: −197.6px` is where the pivot
sits, not how tall the image is — an earlier target of 71.4 × 593 came from reading the one as the
other. The face’s 478 is 3 × 159.2 = 477.6 rounded **up**, so the face never falls short of its
well.

Nothing else in this casting is baked. GUI-SPEC §4 states the pivot and travel.
