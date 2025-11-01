#pragma once

#include <clarinoid/menu/MenuSettings.hpp>
#include <clarinoid/application/DefaultHud.hpp>
#include "clarinoid2MusicalStateTask.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct BigPerfDisplayApp : DisplayApp
{
    MusicalStateTask &mMusicalStateTask;
    ISysInfoProvider &mSysInfoProvider;
    BigPerfDisplayApp(IDisplay &d, MusicalStateTask &musicalStateTask, ISysInfoProvider &sysInfoProvider)
        : DisplayApp(d), mMusicalStateTask(musicalStateTask), mSysInfoProvider(sysInfoProvider)
    {
    }

    virtual const char *DisplayAppGetName() override
    {
        return "Big Performance Display";
    }

    virtual void UpdateApp() override
    {
        if (mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }
    virtual void RenderApp() override
    {
    }

    virtual bool AllowOverlayIndicators() const override
    {
        return false;
    }

    virtual void RenderFrontPage() override
    {
        auto &appSettings = *mMusicalStateTask.mAppSettings;
        auto &perf = appSettings.GetCurrentPerformancePatch();

        mDisplay.setCursor(0, 0);
        mDisplay.println(String("P") + appSettings.mCurrentPerformancePatch + ":" + String(perf.mName));
        auto padding = RectI::Construct(1, 1, 1, 1);

        if (perf.mSynthAEnabled && (perf.mSynthPresetA != -1))
        {
            mDisplay.setCursor(1, 10);
            mDisplay.PrintInvertedText(String("A") + perf.mSynthPresetA,
                                       padding); // + appSettings.GetSynthPatchName(perf.mSynthPresetA));
        }

        if (perf.mSynthBEnabled && (perf.mSynthPresetB != -1))
        {
            mDisplay.setCursor(29, 10);
            mDisplay.PrintInvertedText(String("B") + perf.mSynthPresetB,
                                       padding); // + appSettings.GetSynthPatchName(perf.mSynthPresetB));
        }

        if (perf.mHarmEnabled)
        {
            mDisplay.setCursor(59, 10);
            String s = String("H") + perf.mHarmPreset + ":" + perf.mGlobalScale.ToShortString();
            mDisplay.PrintInvertedText(s.substring(0, 9),
                                       padding); // ppSettings.GetHarmPatchName(perf.mHarmPreset));
        }

        mDisplay.setTextColor(SSD1306_WHITE); // normal text

        //
        auto voices = mSysInfoProvider.ISysInfoProvider_GetVoiceState();

        fixed_vector<SynthVoiceState, MAX_SYNTH_VOICES> voiceState;
        fixed_vector<MidiNote, MAX_SYNTH_VOICES> chordNotes;
        for (auto &v : voices)
        {
            if (v.mIsPlaying)
            {
                voiceState.push_back(v);
                chordNotes.push_back(v.mNote);
            }
        }
        auto spelledChord = spellChord(chordNotes).notes;

        // live note
        // find the live note
        for (size_t i = 0; i < voiceState.size(); ++i)
        {
            auto &v = voiceState[i];
            auto &spelledNote = spelledChord[i];
            if (v.mVoiceSource == VoiceSource::Live)
            {
                mDisplay.setCursor(3, 21);
                mDisplay.SetFontScale(2, 2);
                mDisplay.println(String("") + spelledNote.ToString());
                mDisplay.SetFontScale(1, 1);
                break;
            }
        }

        // all other notes will be drawn in a grid. up to 6 are shown,
        std::array<PointI, 6> notePositions = {
            PointI::Construct(51, 22),
            PointI::Construct(81, 22),
            PointI::Construct(109, 22),
            PointI::Construct(51, 31),
            PointI::Construct(81, 31),
            PointI::Construct(109, 31),
        };
        size_t shownNoteIndex = 0;
        for (size_t i = 0; i < voiceState.size(); ++i)
        {
            auto &v = voiceState[i];
            auto &spelledNote = spelledChord[i];
            if (v.mVoiceSource != VoiceSource::Live)
            {
                if (i >= notePositions.size())
                    break;
                mDisplay.setCursor(notePositions[shownNoteIndex].x, notePositions[shownNoteIndex].y);
                mDisplay.println(String("") + spelledNote.ToString());
                shownNoteIndex++;
            }
        }

        // pitch bend bar
        // pitch bend range is usually +/- 2 semitones, but can be confirgured PER OSCILLATOR.
        // so there's no way to guarantee this display is accurate. therefore no labels; just show -100%, -50%, 0%,
        // +50%, +100% ticks.
        {
            float pb = mMusicalStateTask.mMusicalState.mCurrentPitchN11.GetValue();
            constexpr int marginX = 4; // leave room for indicator
            constexpr int circleRadius = 5;
            constexpr int circleCenterY = 44;
            constexpr int barYStart = 51;
            constexpr int barHeight = 5;
            constexpr int pbBarWidth = MAX_DISPLAY_WIDTH - marginX * 2;
            constexpr int pbQuarterWidth = pbBarWidth / 4;
            constexpr int pbHalfWidth = pbBarWidth / 2;

            // given pb range is -1 to 1, but normally it's -2 to 2 semitones.
            // so these values are 1/2 what you'd expect at semitone scale.
            constexpr float kNoteIndicatorMarginHalfSemisCourse =
                0.15f; // how many semitones away from a pitch to be considered "at" that pitch. -- coarse; where the
                       // indicator is drawn at all
            constexpr float kNoteIndicatorMarginHalfSemisFine = 0.07f; // fine; highlighting the indicator.

            auto pbN11ToX = [&](float pbN11) -> int {
                return marginX + (int)((((pbN11 + 1.0f) / 2.0f) * (float)pbBarWidth)); // map -1..1 to 0..pbBarWidth
            };
            // are we close enough to -100% to draw the indicator?
            auto drawIndicatorAtPB = [&](float targetPBN11) -> void {
                if (FloatDistance(pb, targetPBN11) > kNoteIndicatorMarginHalfSemisCourse)
                {
                    return;
                }
                bool fine = FloatDistance(pb, targetPBN11) <= kNoteIndicatorMarginHalfSemisFine;
                // draw indicator
                int indicatorCenterX = pbN11ToX(targetPBN11);
                mDisplay.FillCircleWithBrightness(
                    PointI::Construct(indicatorCenterX, circleCenterY), circleRadius, fine ? 255 : 32);
            };

            drawIndicatorAtPB(-1.0f); // -100%
            drawIndicatorAtPB(-0.5f); // -50%
            drawIndicatorAtPB(0.0f);  // 0%
            drawIndicatorAtPB(0.5f);  // +50%
            drawIndicatorAtPB(1.0f);  // +100%

            // draw bar.
            int pbX = pbN11ToX(pb);
            if (pb < 0) // negative bend
            {
                // fill from pbX to center
                mDisplay.FillRectWithBrightness(
                    RectI::Construct(pbX, barYStart, pbHalfWidth - (pbX - marginX), barHeight), 128);
            }
            else
            {
                // positive bend
                int pbBarStartX = marginX + pbHalfWidth;
                mDisplay.FillRectWithBrightness(RectI::Construct(pbBarStartX, barYStart, pbX - pbBarStartX, barHeight),
                                                128);
            }

            {
                // draw indicator ticks at -100%, -50%, 0%, +50%, +100%
                int x = marginX;
                mDisplay.drawFastVLine(x, barYStart, barHeight, SSD1306_WHITE); // -100
                x += pbQuarterWidth;
                mDisplay.drawFastVLine(x, barYStart, barHeight, SSD1306_WHITE); // -50
                x += pbQuarterWidth;
                mDisplay.drawFastVLine(x, barYStart, barHeight, SSD1306_WHITE); // 0
                x += pbQuarterWidth;
                mDisplay.drawFastVLine(x, barYStart, barHeight, SSD1306_WHITE); // +50
                x += pbQuarterWidth;
                mDisplay.drawFastVLine(x, barYStart, barHeight, SSD1306_WHITE); // +100
            }
        }
    }

    virtual void DisplayAppUpdate() override
    {
        DisplayApp::DisplayAppUpdate(); // update input
    }
};

} // namespace clarinoid
