#include "PanelBackground.h"

using namespace ElmerTheme;

namespace
{
    const juce::Image& ringImage (Layout::Ring r)
    {
        static const juce::Image large = juce::ImageCache::getFromMemory (
            BinaryData::scalelg_png, BinaryData::scalelg_pngSize);
        static const juce::Image small = juce::ImageCache::getFromMemory (
            BinaryData::scalesm_png, BinaryData::scalesm_pngSize);
        static const juce::Image five = juce::ImageCache::getFromMemory (
            BinaryData::scale5_png, BinaryData::scale5_pngSize);

        switch (r)
        {
            case Layout::Ring::small9: return small;
            case Layout::Ring::five:   return five;
            case Layout::Ring::large11:
            default:                   return large;
        }
    }

    /** A recessed control group. The wash is drawn as two passes because CSS interpolates
        translucent stops in premultiplied space and JUCE does not - one gradient carrying a mid
        grey at partial alpha lightens the middle of the box and it stops reading as a recess. */
    void drawSection (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& heading)
    {
        Paint::translucentVertical (g, r, Layout::sectionRadius,
                                    juce::Colours::white.withAlpha (0.10f),
                                    juce::Colours::white.withAlpha (0.02f));

        g.setColour (juce::Colours::white.withAlpha (0.30f));
        g.drawLine (r.getX() + Layout::sectionRadius, r.getY() + 0.5f,
                    r.getRight() - Layout::sectionRadius, r.getY() + 0.5f, 1.0f);

        g.setColour (juce::Colour (0xFF322C22).withAlpha (0.25f));
        g.drawLine (r.getX() + Layout::sectionRadius, r.getBottom() - 0.5f,
                    r.getRight() - Layout::sectionRadius, r.getBottom() - 0.5f, 1.0f);

        Text::drawTracked (g, heading, Font::label (Layout::sectionHeaderSize),
                           Layout::sectionHeaderTracking,
                           { r.getX() + Layout::sectionPadX + 1.0f, r.getY() + Layout::sectionPadTop,
                             r.getWidth(), 14.0f },
                           juce::Justification::left, Colour::ink);
    }
}

//==============================================================================
PanelBackground::PanelBackground()
{
    setInterceptsMouseClicks (false, false);
    buildImage();
}

void PanelBackground::paint (juce::Graphics& g)
{
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (baked, juce::Rectangle<float> (Layout::canvasWidth, Layout::canvasHeight));
}

void PanelBackground::buildImage()
{
    baked = juce::Image (juce::Image::ARGB,
                         (int) Layout::canvasWidth * bakeScale,
                         (int) Layout::canvasHeight * bakeScale, true);
    juce::Graphics g { baked };

    // Everything below paints in design coordinates; the bake resolution is this one transform.
    g.addTransform (juce::AffineTransform::scale ((float) bakeScale));

    {
        juce::Path chassis;
        chassis.addRoundedRectangle (
            juce::Rectangle<float> (Layout::canvasWidth, Layout::canvasHeight), Layout::cornerRadius);
        g.reduceClipRegion (chassis);
    }

    paintFascia (g);
    paintRailsAndScrews (g);
    paintHeaderChrome (g);
    paintSections (g);
    paintKnobFurniture (g);
    paintMeterChrome (g);
    paintFooter (g);
}

//==============================================================================
void PanelBackground::paintFascia (juce::Graphics& g)
{
    const juce::Rectangle<float> panel { Layout::canvasWidth, Layout::canvasHeight };

    g.setColour (Colour::fascia);
    g.fillRect (panel);

    // Brushed grain: repeating-linear-gradient(96deg, white .05 0-1px, black .035 1-2px,
    // transparent 2-4px). 96deg is 6 degrees past due east, so the stripes lean 6 degrees off
    // vertical - over 776px that walks them 80px sideways, and the near-miss against the pixel grid
    // is what gives the metal its shimmer. Drawn dead vertical it is a flat pinstripe.
    {
        juce::Graphics::ScopedSaveState state { g };
        g.addTransform (juce::AffineTransform::rotation (juce::degreesToRadians (-6.0f),
                                                         panel.getCentreX(), panel.getCentreY()));

        const float overhang = Layout::canvasHeight;

        for (float x = -overhang; x < Layout::canvasWidth + overhang; x += 4.0f)
        {
            g.setColour (juce::Colours::white.withAlpha (0.05f));
            g.fillRect (x, -overhang, 1.0f, Layout::canvasHeight + overhang * 2.0f);
            g.setColour (juce::Colours::black.withAlpha (0.035f));
            g.fillRect (x + 1.0f, -overhang, 1.0f, Layout::canvasHeight + overhang * 2.0f);
        }
    }

    // Ambient lighting. The shade radial is held at .03 deliberately: at .30 it dragged the
    // lower-right corner's text contrast to 4.7:1, under BRAND.md's 7:1 floor. If the fascia is
    // ever retuned, this stays shallow.
    {
        // CSS radial-gradients are ELLIPSES - `120% 90% at 28% 0%` is 1.2x the panel width by 0.9x
        // its height. JUCE's radial gradients are circular, so each is drawn under a squash about
        // its own centre. Drawn as circles they reach far too far vertically and the mid-panel
        // tone comes out several levels off.
        const auto ellipticalRadial = [&g, panel] (juce::Point<float> centre, float rx, float ry,
                                                   float stop, juce::Colour tint)
        {
            juce::Graphics::ScopedSaveState state { g };
            g.addTransform (juce::AffineTransform::scale (1.0f, ry / rx, centre.x, centre.y));

            juce::ColourGradient grad { tint, centre.x, centre.y,
                                        tint.withAlpha (0.0f), centre.x + rx, centre.y, true };
            grad.addColour ((double) stop, tint.withAlpha (0.0f));
            g.setGradientFill (grad);

            // The fill must cover the panel after the inverse squash, so expand generously.
            g.fillRect (panel.expanded (0.0f, Layout::canvasHeight * 2.0f));
        };

        ellipticalRadial ({ Layout::canvasWidth * 0.28f, 0.0f },
                          Layout::canvasWidth * 1.2f, Layout::canvasHeight * 0.9f, 0.58f,
                          juce::Colours::white.withAlpha (0.16f));

        ellipticalRadial ({ Layout::canvasWidth * 0.80f, Layout::canvasHeight * 1.10f },
                          Layout::canvasWidth * 1.2f, Layout::canvasHeight * 1.2f, 0.66f,
                          juce::Colour (0xFF28221A).withAlpha (0.03f));
    }
}

void PanelBackground::paintRailsAndScrews (juce::Graphics& g)
{
    const auto rail = [&g] (juce::Rectangle<float> r, bool mirrored)
    {
        juce::ColourGradient grad { mirrored ? Colour::railEdge : Colour::railDark, r.getX(), 0.0f,
                                    mirrored ? Colour::railDark : Colour::railEdge, r.getRight(), 0.0f,
                                    false };
        grad.addColour (0.60, Colour::railLight);
        g.setGradientFill (grad);
        g.fillRect (r);
    };

    rail ({ 0.0f, 0.0f, Layout::railWidth, Layout::canvasHeight }, false);
    rail ({ Layout::canvasWidth - Layout::railWidth, 0.0f, Layout::railWidth, Layout::canvasHeight }, true);

    for (const auto& c : Layout::screwCentres)
    {
        const float d = Layout::screwDiameter;
        const juce::Rectangle<float> body { c.x - d * 0.5f, c.y - d * 0.5f, d, d };

        juce::ColourGradient head { Colour::screwLight, body.getX() + d * 0.35f, body.getY() + d * 0.30f,
                                    Colour::screwDark, body.getRight(), body.getBottom(), true };
        g.setGradientFill (head);
        g.fillEllipse (body);

        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.drawLine (body.getX() + 1.0f, body.getY() + 0.5f, body.getRight() - 1.0f, body.getY() + 0.5f, 1.0f);
    }
}

void PanelBackground::paintHeaderChrome (juce::Graphics& g)
{
    // --- nameplate plinth: ink-filled engraving, not embossing -------------------------------
    const auto wordmarkFont = Font::wordmark (Layout::wordmarkSize);
    const float textWidth = Text::trackedWidth ("ELMER", wordmarkFont, Layout::wordmarkTracking);

    const juce::Rectangle<float> plinth {
        Layout::nameplateX, Layout::nameplateY,
        Layout::plinthPadLeft + textWidth + Layout::plinthPadRight,
        Layout::plinthPadTop + Layout::wordmarkSize + Layout::plinthPadBottom };

    {
        juce::Path p;
        p.addRoundedRectangle (plinth, Layout::plinthRadius);
        juce::DropShadow shadow { juce::Colour (0xFF282318).withAlpha (0.30f), 3, { 0, 2 } };
        shadow.drawForPath (g, p);
    }

    g.setGradientFill (Paint::vertical (plinth, Colour::plinthTop, Colour::plinthBottom));
    g.fillRoundedRectangle (plinth, Layout::plinthRadius);

    g.setColour (juce::Colours::white.withAlpha (0.42f));
    g.drawLine (plinth.getX() + 2.0f, plinth.getY() + 0.5f,
                plinth.getRight() - 2.0f, plinth.getY() + 0.5f, 1.0f);

    // The letters read as ink-filled engraving: near-black fill doing the work, relief only as a
    // one-pixel edge pair. Pure highlight-and-shadow on a same-value panel is unreadable, which is
    // why BRAND.md requires relief treatments to be paint-filled.
    {
        const juce::Rectangle<float> textArea { plinth.getX() + Layout::plinthPadLeft,
                                                plinth.getY() + Layout::plinthPadTop,
                                                textWidth + 4.0f, Layout::wordmarkSize };

        g.setFont (wordmarkFont);
        float x = textArea.getX();

        for (int pass = 0; pass < 3; ++pass)
        {
            const float dy = pass == 0 ? 1.0f : (pass == 1 ? -1.0f : 0.0f);
            g.setColour (pass == 0 ? juce::Colours::white.withAlpha (0.30f)
                       : pass == 1 ? juce::Colours::black.withAlpha (0.45f)
                                   : Colour::wordmarkInk);
            x = textArea.getX();

            for (int i = 0; i < 5; ++i)
            {
                const auto ch = juce::String::charToString (juce::String ("ELMER")[i]);
                const float w = juce::GlyphArrangement::getStringWidth (wordmarkFont, ch);
                g.drawText (ch, juce::Rectangle<float> (x, textArea.getY() + dy, w + 2.0f, textArea.getHeight()),
                            juce::Justification::centredLeft, false);
                x += w + Layout::wordmarkTracking;
            }
        }
    }

    const float taglineY = plinth.getBottom() + 8.0f;
    Text::drawTracked (g, "BUS COMPRESSOR", Font::label (Layout::taglineSize), Layout::taglineTracking,
                       { Layout::nameplateX, taglineY, 340.0f, 17.0f },
                       juce::Justification::left, Colour::ink);

    Text::drawTracked (g, "MODEL GL-87 " + Text::middleDot() + " STEREO",
                       Font::monoMed (Layout::modelLineSize), Layout::modelLineTracking,
                       { Layout::nameplateX, taglineY + 19.0f, 340.0f, 13.0f },
                       juce::Justification::left, Colour::ink);

    // --- captions ----------------------------------------------------------------------------
    const auto caption = Font::mono (Layout::captionSize);

    Text::drawTracked (g, "PROGRAM", caption, Layout::captionTracking,
                       { Layout::programX, Layout::captionY, 200.0f, 12.0f },
                       juce::Justification::left, Colour::ink);
    Text::drawTracked (g, "IN", caption, Layout::captionTracking,
                       { Layout::meterInX, Layout::captionY, 100.0f, 12.0f },
                       juce::Justification::left, Colour::ink);
    Text::drawTracked (g, "OUT", caption, Layout::captionTracking,
                       { Layout::meterOutX, Layout::captionY, 100.0f, 12.0f },
                       juce::Justification::left, Colour::ink);

    // --- divider: an incised score line in the metal ------------------------------------------
    juce::ColourGradient rule { Colour::dividerLeft, Layout::contentX, 0.0f,
                                Colour::dividerRight, Layout::contentRight, 0.0f, false };
    g.setGradientFill (rule);
    g.fillRect (Layout::contentX, Layout::dividerY, Layout::contentWidth, 1.0f);

    g.setColour (juce::Colours::white.withAlpha (0.28f));
    g.fillRect (Layout::contentX, Layout::dividerY + 1.0f, Layout::contentWidth, 1.0f);
}

void PanelBackground::paintSections (juce::Graphics& g)
{
    /*  **§2: three scored rules and four centred headings. The recessed group boxes are gone.**

        `drawSection` drew a translucent wash on a rounded rect with a light top edge, a dark bottom
        edge and the heading set left inside it. The delivered prototype carries no box treatment at
        all and divides the body with 1 px `rgba(255,255,255,.5)` rules, each heading centred over
        the region it names. A recessed group says "these controls sit in a well"; a scored line says
        "the casting is divided here", which is what a console module's silkscreen does. */
    g.setColour (juce::Colours::white.withAlpha (Layout::dividerInk));
    g.fillRect (Layout::dividerDetectorX, Layout::dividerDetectorY, 1.0f, Layout::dividerDetectorH);
    g.fillRect (Layout::dividerHorizontalX, Layout::dividerHorizontalY,
                Layout::dividerHorizontalW, 1.0f);
    g.fillRect (Layout::dividerLowerX, Layout::dividerLowerY, 1.0f, Layout::dividerLowerH);

    const auto headingFont = Font::label (Layout::sectionHeadingCssPx);
    const float headingTracking = Layout::sectionHeadingTracking;

    for (const auto& h : Layout::sectionHeadings)
        Text::drawTracked (g, h.text, headingFont, headingTracking,
                           { h.x, h.y, h.w, Layout::sectionHeadingLineBox },
                           juce::Justification::centred, Colour::ink);

    // The meter's own caption pair, in mono rather than the heading face — see the theme's note on
    // where §2 and the prototype disagree.
    const auto capFont = Font::mono (Layout::meterCaptionCssPx);
    const float capTracking = Layout::meterCaptionTracking;

    Text::drawTracked (g, Layout::meterCaption.text, capFont, capTracking,
                       { Layout::meterCaption.x, Layout::meterCaption.y,
                         Layout::meterCaption.w, Layout::meterCaptionLineBox },
                       juce::Justification::centred, Colour::ink);

    Text::drawTracked (g, juce::String::fromUTF8 (Layout::meterSubCaption.text), capFont, capTracking,
                       { Layout::meterSubCaption.x, Layout::meterSubCaption.y,
                         Layout::meterSubCaption.w, Layout::meterCaptionLineBox },
                       juce::Justification::centred, Colour::inkFlavour);

    // KNEE sits in DETECTION's second row and has a heading of its own.
    Text::drawTracked (g, "KNEE", Font::label (Layout::controlLabelSize), Layout::controlLabelTracking,
                       { Layout::kneeButtonsTopLeft.x, Layout::kneeLabelY, Layout::lampW, 14.0f },
                       juce::Justification::centred, Colour::ink);
}

void PanelBackground::paintKnobFurniture (juce::Graphics& g)
{
    const auto legendFont = Font::mono (Layout::legendSize);
    const auto labelFont  = Font::label (Layout::controlLabelSize);

    for (size_t k = 0; k < Layout::knobs.size(); ++k)
    {
        const auto& spec = Layout::knobs[k];
        const juce::Point<float> areaTopLeft { spec.areaCentre.x - spec.areaSize * 0.5f,
                                               spec.areaCentre.y - spec.areaSize * 0.5f };

        // Tick ring, under where the knob will sit. Drawn at 0.62 so it reads as printed ink on
        // grey rather than as drawn UI - that is intentional and should not be "fixed".
        {
            const auto& ring = ringImage (spec.ring);

            if (ring.isValid())
            {
                juce::Graphics::ScopedSaveState state { g };
                g.setOpacity (Layout::tickRingOpacity);
                g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
                g.drawImage (ring, juce::Rectangle<float> (areaTopLeft.x + 2.0f, areaTopLeft.y + 2.0f,
                                                           spec.ringSize, spec.ringSize),
                             juce::RectanglePlacement::stretchToFit, false);
            }
        }

        // Printed legends, at their literal hand-tuned offsets. Each is a 40px centred box, so the
        // glyph centre is (left + 20, top + 6). Never recomputed from an angle.
        for (const auto& legend : Layout::legends[k])
        {
            if (legend.text == nullptr)
                continue;

            Text::drawTracked (g, legend.text, legendFont, 0.0f,
                               { areaTopLeft.x + legend.left, areaTopLeft.y + legend.top,
                                 Layout::legendBoxWidth, Layout::legendLineHeight },
                               juce::Justification::centred, Colour::ink);
        }

        // The unit, in the arc gap. Drawn with the legends because it IS one - same type, same
        // 40px centred box, same ink - and never appended to the control's name.
        if (const auto& unit = Layout::knobArcUnits[k]; unit.text != nullptr)
            Text::drawTracked (g, unit.text, legendFont, Layout::unitTracking,
                               { areaTopLeft.x + unit.left, areaTopLeft.y + unit.top,
                                 Layout::legendBoxWidth, Layout::legendLineHeight },
                               juce::Justification::centred, Colour::ink);

        Text::drawTracked (g, spec.label, labelFont, Layout::controlLabelTracking,
                           { spec.areaCentre.x - 120.0f, spec.labelY, 240.0f, 14.0f },
                           juce::Justification::centred, Colour::ink);
    }
}

void PanelBackground::paintMeterChrome (juce::Graphics& g)
{
    /*  **ONLY `MOVING COIL · 300 ms BALLISTIC` IS DRAWN HERE NOW — the other two moved and were
        being printed TWICE.**

        This block drew all three of the meter's spec lines from `meterX`/`meterY`: the caption above
        the well, the stereo note opposite it on the same row, and the ballistic note below. §2 gives
        the caption its own position at (60, 150) and puts the stereo note at (60, 342) as a second
        CENTRED line under the well rather than as a right-aligned partner on the caption's row — so
        both are drawn by `paintSections` against the prototype's figures.

        Leaving them here as well double-printed each at a different place, which is visible and was:
        the new centred caption sat across the old left-aligned one. Worth naming because the plate
        inversion made double-printing this casting's failure mode too, in the opposite direction to
        the absent-element one — a runtime draw that moves leaves its old site behind. */
    const auto spec = Font::monoBold (Layout::meterSpecSize);
    const float footerY = Layout::meterY + Layout::meterH + Layout::meterSpecGap;

    Text::drawTracked (g, "MOVING COIL " + Text::middleDot() + " 300 ms BALLISTIC", spec,
                       Layout::meterSpecTracking,
                       { Layout::meterX, footerY, Layout::meterW, 14.0f },
                       juce::Justification::left, Colour::ink);
}

void PanelBackground::paintFooter (juce::Graphics& g)
{
    // Scribble strip: marker on tape, hand-torn ends, rotated. It stays a drawn element rather than
    // a bitmap so it scales with the panel.
    {
        const auto markerFont = Font::marker (Layout::scribbleSize);
        const juce::String text { "CH 24 " + juce::String::charToString ((juce::juce_wchar) 0x2014)
                                  + " MIX BUS / GLUE" };
        // Tracked width, not the plain glyph width: the 0.5px letter-spacing is part of what the
        // tape has to be wide enough for, so measuring without it undersizes the strip AND crowds
        // the text inside it.
        const float textWidth = Text::trackedWidth (text, markerFont, Layout::scribbleTracking);
        const juce::Rectangle<float> tape { Layout::contentX + 6.0f, Layout::footerY - 10.0f,
                                            textWidth + Layout::scribblePadLeft + Layout::scribblePadRight,
                                            Layout::scribbleSize + Layout::scribblePadTop
                                                + Layout::scribblePadBottom };

        juce::Graphics::ScopedSaveState state { g };
        g.addTransform (juce::AffineTransform::rotation (
            juce::degreesToRadians (Layout::scribbleRotationDegrees),
            tape.getCentreX(), tape.getCentreY()));

        // Hand-torn ends: straight long edges, ragged short ones.
        juce::Path torn;
        const std::array<juce::Point<float>, 10> pts { {
            { 0.02f, 0.10f }, { 0.06f, 0.02f }, { 0.97f, 0.00f }, { 1.00f, 0.34f },
            { 0.98f, 0.72f }, { 1.00f, 0.96f }, { 0.55f, 1.00f }, { 0.08f, 0.97f },
            { 0.02f, 0.66f }, { 0.00f, 0.28f } } };

        for (size_t i = 0; i < pts.size(); ++i)
        {
            const juce::Point<float> p { tape.getX() + tape.getWidth() * pts[i].x,
                                         tape.getY() + tape.getHeight() * pts[i].y };
            if (i == 0) torn.startNewSubPath (p); else torn.lineTo (p);
        }
        torn.closeSubPath();

        {
            juce::DropShadow shadow { juce::Colours::black.withAlpha (0.30f), 4, { 0, 2 } };
            shadow.drawForPath (g, torn);
        }

        auto tapeFill = Paint::vertical (tape, Colour::tapeTop, Colour::tapeBottom);
        tapeFill.addColour (0.55, Colour::tapeMid);
        g.setGradientFill (tapeFill);
        g.fillPath (torn);

        Text::drawTracked (g, text, markerFont, Layout::scribbleTracking,
                           tape.withTrimmedLeft (Layout::scribblePadLeft)
                               .withTrimmedRight (Layout::scribblePadRight)
                               .withTrimmedTop (Layout::scribblePadTop)
                               .withTrimmedBottom (Layout::scribblePadBottom),
                           juce::Justification::left, Colour::markerInk, false);
    }

    /*  **TWO footer strings, not one running the width of the panel.**

        This drew a single right-aligned run — `GL-87 · CONSOLE MODULE · SN 0871   ·   v1.0` — into
        a box spanning the whole content width. Right-aligned in a full-width box, its ink ended up
        under the OUTPUT column, so `SN 0871` printed straight through that group's `%` and `100`
        numerals. Two elements claiming one band, and neither could know about the other.

        The delivered prototype splits it: `GL-87 · SN 0042` at **x 26**, and the version alone
        right-aligned at **x 914** over 400 px. Each occupies its own end of the panel and neither
        crosses the middle, which is what stops it colliding with whatever the body puts there.

        `CONSOLE MODULE` is dropped with the split — the prototype's left string is the model and the
        serial only, and re-adding it is what would push the left run back under the TIMING column. */
    const auto footFont = Font::monoBold (Layout::footerTextSize);
    const auto dot = Text::middleDot();

    Text::drawTracked (g, "GL-87 " + dot + " SN 0871", footFont, Layout::footerTracking,
                       { Layout::footerLeftX, Layout::footerY, 400.0f, Layout::footerLineBox },
                       juce::Justification::left, Colour::ink);

    Text::drawTracked (g, "v" NF_VERSION_SHORT, footFont, Layout::footerTracking,
                       { Layout::footerRightX, Layout::footerY, 400.0f, Layout::footerLineBox },
                       juce::Justification::right, Colour::ink);
}
