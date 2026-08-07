# Prompts

Numbered work packages. Mark one `SHIPPED` with the date once it is fully implemented.

PROMPT #1 - SHIPPED 2026-08-07

Build Elmer, the sixth Neon Foundry casting: a stereo-linked VCA bus compressor, model GL-87, in
its own repo at elmer/. Study ../BRAND.md, ../taperot/ for the project pattern and ../gatecrasher/
for the Program architecture; design/ is authoritative for the GUI. Knobs are 128-frame bitmap
filmstrips; the meter is a static face plus a needle sprite rotated at runtime. Printed knob legends
were hand-tuned by eye - preserve their positions rather than recomputing them from a formula.
Propose the DSP architecture for review before writing any DSP code.

PROMPT #2

Tune the DSP and the factory bank by ear. The detector constants, the 6 dB soft-knee width, Iron's
drive curve and AUTO's two time constants are a structurally-reasoned first pass; the 16 Programs
carry directionally correct values but nothing has been listened to. Keep
Tests/CompressorTests.cpp's stereo-link, realised-ratio and AUTO-adaptation cases passing
throughout - they are the guards on the architecture, not on the tuning.

PROMPT #3

Resolve the panel's mid-tone difference against design/screenshots/panel.png. A blurred comparison
puts this build ~10-15 levels darker across the right and lower panel, but the capture pipeline used
for that measurement is not colour-faithful (the scribble tape is drawn #EFE9D6 and captures as
#F5EFD4), so the finding may be measurement rather than rendering. Re-measure from a same-display 2x
capture first, and change no colour constant until the cause is isolated - the fascia value is set
by BRAND.md's 7:1 legibility floor and every bundled bitmap was re-rendered against it.

PROMPT #4

Register the suite in ../manifest/suite.toml. Held until all six plugins existed, which is now the
case. Needs a tagged release for each plugin plus a freshly generated windows_appid GUID. This is a
root-level change outside any plugin repo and needs explicit approval.
