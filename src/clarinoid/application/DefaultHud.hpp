
#pragma once

#include <clarinoid/Gui/Bitmaps.hpp>
#include "Display.hpp"
#include "Metronome.hpp"
#include <clarinoid/harmonizer/MusicalVoice.hpp>
#include <clarinoid/synth/Analysis.hpp>
#include <clarinoid/application/VUMeter.hpp>

namespace clarinoid
{
struct SynthVoiceState
{
    bool mIsPlaying = false;
    MidiNote mNote;
    int16_t mSynthPatchIndex;
    VoiceSource mVoiceSource;
};

struct ISysInfoProvider
{
    virtual uint8_t ISysInfoProvider_GetPolyphony() = 0;
    virtual float ISysInfoProvider_GetAudioCPUUsage() = 0;       // in % (0-100)
    virtual float ISysInfoProvider_GetTaskManagerCPUUsage() = 0; // in % (0-100)
    // virtual float ISysInfoProvider_GetPeak() = 0;                // in amplitude.
    virtual MidiNote ISysInfoProvider_GetNote() = 0;
    virtual std::array<SynthVoiceState, MAX_MUSICAL_VOICES> ISysInfoProvider_GetVoiceState() = 0;
    virtual float ISysInfoProvider_GetTempo() = 0; // bpm
    virtual Metronome *ISysInfoProvider_GetMetronome() = 0;
    virtual float ISysInfoProvider_GetPitchBendN11() = 0;
    virtual float ISysInfoProvider_GetBreath01() = 0;
    virtual AppSettings *ISysInfoProvider_GetSettings() = 0;
};

// this renders a hud so the formatting is not defined by the display object itself.
struct DefaultHud : IHudProvider
{
    GenericPeakMeterUtility<1000, 250> mCPUPeakMeter;
    IDisplay &mDisplay;
    ISysInfoProvider *mpInfo = nullptr;
    DefaultHud(IDisplay &display, ISysInfoProvider *infoProvider) : mDisplay(display), mpInfo(infoProvider)
    {
    }

    static constexpr int16_t HUD_LINE_SEPARATOR_HEIGHT = 2;

    virtual int16_t IHudProvider_GetHudHeight() override
    {
        return mDisplay.GetLineHeight() + HUD_LINE_SEPARATOR_HEIGHT -
               1; // -1 because the bottom line of the font will not really be used.
    }

    virtual String IHudProvider_GetHudTransientIndicator(InputDelegator *input) override
    {
        String s("");
        int pbN100100 = FloatRoundToInt(mpInfo->ISysInfoProvider_GetPitchBendN11() * 100);
        if (pbN100100 != 0)
        {
            s += signbit(pbN100100) ? "-" : "+";
            s += String(abs(pbN100100));
        }
        return s;
    }

    virtual void IHudProvider_RenderHud(int16_t displayWidth, int16_t displayHeight) override
    {
        int16_t hudYStart = displayHeight - IHudProvider_GetHudHeight();
        mDisplay.SetClipRect(RectI::Construct(0, hudYStart, displayWidth, displayHeight));
        mDisplay.drawFastHLine(0, hudYStart, displayWidth, SSD1306_BLACK);
        mDisplay.drawFastHLine(0, hudYStart + 1, displayWidth, SSD1306_WHITE);
        mDisplay.setCursor(0, hudYStart + HUD_LINE_SEPARATOR_HEIGHT);
        mDisplay.setTextColor(SSD1306_WHITE); // normal text

        String dbpeak = DecibelsToIntString(LinearToDecibels(gAnalysisStateC.heldPeakLinear));

        float cpu = mCPUPeakMeter.Update(
            std::max(mpInfo->ISysInfoProvider_GetAudioCPUUsage(), mpInfo->ISysInfoProvider_GetTaskManagerCPUUsage()));
        int icpu = (int)std::ceil(cpu);
        mDisplay.print(String(mpInfo->ISysInfoProvider_GetPolyphony()) + "v " + icpu + "% " + dbpeak + " " +
                       mpInfo->ISysInfoProvider_GetNote().ToStringWithOctave());

        String bpmStr = String(CHARSTR_QEQ) + (int)std::round(mpInfo->ISysInfoProvider_GetTempo());
        auto rcbpm = mDisplay.GetTextBounds(bpmStr);

        mDisplay.setCursor(mDisplay.width() - rcbpm.width, mDisplay.getCursorY());
        if (mpInfo->ISysInfoProvider_GetSettings()->mMetronomeSoundOn)
        {
            mDisplay.DrawBitmap(PointI::Construct((mDisplay.width() - rcbpm.width - gPatchDisabledSpec.widthPixels) - 1,
                                                  mDisplay.GetClientRect().bottom() + 2),
                                gPatchEnabledSpec);
        }
        mDisplay.print(bpmStr);

        static constexpr int metronomeFlashWidth = 48;
        if (mpInfo->ISysInfoProvider_GetMetronome()->GetBeatFrac() < 0.10f)
        {
            mDisplay.fillRect(displayWidth - metronomeFlashWidth,
                              hudYStart,
                              metronomeFlashWidth,
                              IHudProvider_GetHudHeight(),
                              SSD1306_INVERSE);
        }
    }
};

} // namespace clarinoid
