#pragma once

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/components/AdafruitSSD1366Wrapper.hpp"
#include "DisplayDefs.hpp"

#include <array>

namespace clarinoid
{

struct KeyStates
{
    bool isPressed[12] = {false};
};

namespace KeyboardDisplayDetail
{

struct KeySpec
{
    // each key has up to 3 areas to fill to indicate state. two are pretty obvious; the 3rd is necessary for the
    // rounded rectangle of black keys.
    const RectI fillAreas[3];
};

static constexpr int kBorderSize = 1;
static constexpr PointI kKeyboardSize = {78, 33};
static constexpr int kWhiteKeyStride = 11;
static constexpr int kWhiteKeyFillWidth = 10;
static constexpr int kBlackKeyFillHeight = 18;
static constexpr int kWhiteKeyFillHeight = 31;

static constexpr int kWhiteKeyFillRectHeight1 = kBlackKeyFillHeight;
static constexpr int kWhiteKeyFillRectHeight2 = 1;
static constexpr int kWhiteKeyFillRectHeight3 =
    kWhiteKeyFillHeight - kWhiteKeyFillRectHeight1 - kWhiteKeyFillRectHeight2;

static constexpr int kWhiteKeyY1 = kBorderSize;
static constexpr int kWhiteKeyY2 = kWhiteKeyY1 + kWhiteKeyFillRectHeight1;
static constexpr int kWhiteKeyY3 = kWhiteKeyY2 + kWhiteKeyFillRectHeight2;

constexpr int WhiteKeyX(int keyIndex)
{
    return kBorderSize + keyIndex * kWhiteKeyStride;
}

static const KeySpec gKeySpecs[12] = {
    // C
    {{
        RectI::Construct(WhiteKeyX(0), kWhiteKeyY1, 5, kWhiteKeyFillRectHeight1),
        RectI::Construct(WhiteKeyX(0), kWhiteKeyY2, 6, kWhiteKeyFillRectHeight2),
        RectI::Construct(WhiteKeyX(0), kWhiteKeyY3, kWhiteKeyFillWidth, kWhiteKeyFillRectHeight3),
    }},
    // C#
    {{
        RectI::Construct(7, 1, 7, 18),
        RectI::Construct(0, 0, 0, 0),
    }},
    // D
    {{
        RectI::Construct(WhiteKeyX(1) + 3, kWhiteKeyY1, 4, kWhiteKeyFillRectHeight1),
        RectI::Construct(WhiteKeyX(1) + 2, kWhiteKeyY2, 6, kWhiteKeyFillRectHeight2),
        RectI::Construct(WhiteKeyX(1), kWhiteKeyY3, kWhiteKeyFillWidth, kWhiteKeyFillRectHeight3),
    }},
    // D#
    {{
        RectI::Construct(20, 1, 7, 18),
        RectI::Construct(0, 0, 0, 0),
    }},
    // E
    {{
        RectI::Construct(WhiteKeyX(2) + 5, kWhiteKeyY1, 5, kWhiteKeyFillRectHeight1),
        RectI::Construct(WhiteKeyX(2) + 4, kWhiteKeyY2, 6, kWhiteKeyFillRectHeight2),
        RectI::Construct(WhiteKeyX(2), kWhiteKeyY3, kWhiteKeyFillWidth, kWhiteKeyFillRectHeight3),
    }},
    // F
    {{
        RectI::Construct(WhiteKeyX(3), kWhiteKeyY1, 5, kWhiteKeyFillRectHeight1),
        RectI::Construct(WhiteKeyX(3), kWhiteKeyY2, 6, kWhiteKeyFillRectHeight2),
        RectI::Construct(WhiteKeyX(3), kWhiteKeyY3, kWhiteKeyFillWidth, kWhiteKeyFillRectHeight3),
    }},
    // F#
    {{
        RectI::Construct(40, 1, 7, 18),
        RectI::Construct(0, 0, 0, 0),
    }},
    // G
    {{
        RectI::Construct(WhiteKeyX(4) + 3, kWhiteKeyY1, 3, kWhiteKeyFillRectHeight1),
        RectI::Construct(WhiteKeyX(4) + 2, kWhiteKeyY2, 5, kWhiteKeyFillRectHeight2),
        RectI::Construct(WhiteKeyX(4), kWhiteKeyY3, kWhiteKeyFillWidth, kWhiteKeyFillRectHeight3),
    }},
    // G#
    {{
        RectI::Construct(52, 1, 7, 18),
        RectI::Construct(0, 0, 0, 0),
    }},
    // A
    {{
        RectI::Construct(WhiteKeyX(5) + 4, kWhiteKeyY1, 3, kWhiteKeyFillRectHeight1),
        RectI::Construct(WhiteKeyX(5) + 3, kWhiteKeyY2, 5, kWhiteKeyFillRectHeight2),
        RectI::Construct(WhiteKeyX(5), kWhiteKeyY3, kWhiteKeyFillWidth, kWhiteKeyFillRectHeight3),
    }},
    // A#
    {{
        RectI::Construct(64, 1, 7, 18),
        RectI::Construct(0, 0, 0, 0),
    }},
    // B
    {{
        RectI::Construct(WhiteKeyX(6) + 5, kWhiteKeyY1, 5, kWhiteKeyFillRectHeight1),
        RectI::Construct(WhiteKeyX(6) + 4, kWhiteKeyY2, 6, kWhiteKeyFillRectHeight2),
        RectI::Construct(WhiteKeyX(6), kWhiteKeyY3, kWhiteKeyFillWidth, kWhiteKeyFillRectHeight3),
    }},
};

void RenderKeyboard(IDisplay &display, const PointI &offset, const KeyStates &keyStates)
{
    // draw white rect, and then fill keys. result will be the correct outline.
    display.fillRect(offset.x, offset.y, kKeyboardSize.x, kKeyboardSize.y, SSD1306_WHITE);
    for (size_t i = 0; i < 12; i++)
    {
        const KeySpec &ks = gKeySpecs[i];
        uint8_t fillColor = keyStates.isPressed[i] ? 48 : 0;
        for (const RectI &r : ks.fillAreas)
        {
            if (r.width == 0 || r.height == 0)
                continue;
            display.FillRectWithBrightness(RectI::Construct(offset.x + r.x, offset.y + r.y, r.width, r.height),
                                           fillColor);
        }
    }
}

} // namespace KeyboardDisplayDetail

void RenderKeyboard(IDisplay &display, const PointI &offset, const KeyStates &keyStates)
{
    KeyboardDisplayDetail::RenderKeyboard(display, offset, keyStates);
}

KeyStates GetKeyStatesForScale(const Scale &scale)
{
    KeyStates ks;
    for (size_t i = 0; i < 12; i++)
    {
        ks.isPressed[i] = scale.IsNoteInScale((Note)i);
    }
    return ks;
}

} // namespace clarinoid