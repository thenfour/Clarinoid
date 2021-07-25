
#pragma once

#include "Display.hpp"
#include "Metronome.hpp"

namespace clarinoid
{
struct ISysInfoProvider
{
    virtual uint8_t ISysInfoProvider_GetPolyphony() = 0;
    virtual float ISysInfoProvider_GetAudioCPUUsage() = 0;       // in % (0-100)
    virtual float ISysInfoProvider_GetTaskManagerCPUUsage() = 0; // in % (0-100)
    virtual float ISysInfoProvider_GetPeak() = 0;                // in amplitude.
    virtual MidiNote ISysInfoProvider_GetNote() = 0;
    virtual float ISysInfoProvider_GetTempo() = 0; // bpm
    virtual Metronome *ISysInfoProvider_GetMetronome() = 0;
    virtual float ISysInfoProvider_GetPitchBendN11() = 0;
    virtual float ISysInfoProvider_GetBreath01() = 0;
    virtual AppSettings *ISysInfoProvider_GetSettings() = 0;
};

// this renders a hud so the formatting is not defined by the display object itself.
struct DefaultHud : IHudProvider
{
    CCDisplay &mDisplay;
    ISysInfoProvider *mpInfo = nullptr;
    DefaultHud(CCDisplay &display, ISysInfoProvider *infoProvider) : mDisplay(display), mpInfo(infoProvider)
    {
    }

    static constexpr int16_t HUD_LINE_SEPARATOR_HEIGHT = 2;

    virtual int16_t IHudProvider_GetHudHeight() override
    {
        return mDisplay.mDisplay.GetLineHeight() + HUD_LINE_SEPARATOR_HEIGHT - 1; // -1 because the bottom line of the font will not really be used.
    }

    virtual void IHudProvider_RenderHud(int16_t displayWidth, int16_t displayHeight) override
    {
        int16_t hudYStart = displayHeight - IHudProvider_GetHudHeight();
        mDisplay.mDisplay.SetClipRect(0, hudYStart, displayWidth, displayHeight);
        mDisplay.mDisplay.drawFastHLine(0, hudYStart, displayWidth, SSD1306_BLACK);
        mDisplay.mDisplay.drawFastHLine(0, hudYStart + 1, displayWidth, SSD1306_WHITE);
        mDisplay.mDisplay.setCursor(0, hudYStart + HUD_LINE_SEPARATOR_HEIGHT);
        mDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text

        int cpu = (int)std::ceil(std::max(mpInfo->ISysInfoProvider_GetAudioCPUUsage(), mpInfo->ISysInfoProvider_GetTaskManagerCPUUsage()));
        mDisplay.mDisplay.print(
            String(mpInfo->ISysInfoProvider_GetPolyphony()) + "v " +
            cpu + "% " +
            (int)(std::ceil(LinearToDecibels(mpInfo->ISysInfoProvider_GetPeak()))) + "db " +
            mpInfo->ISysInfoProvider_GetNote().ToString() + " d=" + (int) std::round(mpInfo->ISysInfoProvider_GetTempo())
            );

        static constexpr int metronomeFlashWidth = 32;
        if (mpInfo->ISysInfoProvider_GetMetronome()->GetBeatFrac() < 0.10f) {
            mDisplay.mDisplay.fillRect(displayWidth - metronomeFlashWidth, hudYStart, metronomeFlashWidth, IHudProvider_GetHudHeight(), SSD1306_INVERSE);
        }

        // todo: one day make special narrow characters for db & "quarter=" and use :
        // void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w,
        //                               int16_t h, uint16_t color) {
    }
};

} // namespace clarinoid
