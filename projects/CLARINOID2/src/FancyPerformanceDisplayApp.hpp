#pragma once

#include <array>
#include <algorithm>
#include <cmath>

#include "MeshDemoAppBase.hpp"
#include "clarinoid2BigPerfDisplayApp.hpp"
#include "GeodesicSphereDemoApp.hpp"

namespace clarinoid
{

struct FancyPerformanceDisplayApp : DisplayApp
{
    GeodesicSphereSimulation<1> mGeodesicSphereSim;
    MusicalStateTask &mMusicalStateTask;
    ISysInfoProvider &mSysInfoProvider;

    FancyPerformanceDisplayApp(IDisplay &display,
                               MusicalStateTask &musicalStateTask,
                               ISysInfoProvider &sysInfoProvider,
                               uint32_t rngSeed = 0x51F00DF5u)
        : DisplayApp(display), mGeodesicSphereSim(display, musicalStateTask), mMusicalStateTask(musicalStateTask),
          mSysInfoProvider(sysInfoProvider)
    {
    }

    virtual void UpdateApp() override
    {
        if (mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }

    virtual void DisplayAppUpdate() override
    {
        DisplayApp::DisplayAppUpdate();
    }

    virtual const char *DisplayAppGetName() override
    {
        return "mesh demo";
    }

    virtual void RenderApp() override
    {
    }

    bool AllowOverlayIndicators() const override
    {
        return false;
    }

    virtual void RenderFrontPage() override
    {
        mGeodesicSphereSim.StepMeshSimulation();
        mGeodesicSphereSim.SetScreenOffsetPixels(13, -17);
        static constexpr int kSphereWidth = 64;
        mGeodesicSphereSim.RenderMeshFrame({MAX_DISPLAY_WIDTH - kSphereWidth - 1, 0, kSphereWidth, kSphereWidth});

        auto &appSettings = *mMusicalStateTask.mAppSettings;
        auto &perf = appSettings.GetCurrentPerformancePatch();

        static constexpr int kFingeredNoteRowY = 12;
        static constexpr int kTextAreaWidth = 72;
        static constexpr int kPlayingNotesRowWidth = 80;
        static constexpr int kPlayingNotesRowY = 31;
        auto kPadding = RectI::Construct(1, 1, 1, 1);

        if (perf.mSynthAEnabled && (perf.mSynthPresetA != -1))
        {
            mDisplay.setCursor(1, 1);
            mDisplay.PrintInvertedText("A", kPadding);
        }

        if (perf.mSynthBEnabled && (perf.mSynthPresetB != -1))
        {
            mDisplay.setCursor(12, 1);
            mDisplay.PrintInvertedText("B", kPadding);
        }

        if (perf.mHarmEnabled)
        {
            mDisplay.setCursor(24, 1);
            mDisplay.PrintInvertedText("H", kPadding);
            // String s = String("H") + perf.mHarmPreset + ":" + perf.mGlobalScale.ToShortString();
            mDisplay.setCursor(36, 1);
            mDisplay.PrintInvertedText(perf.mGlobalScale.ToShortString().substring(0, 6),
                                       kPadding); // ppSettings.GetHarmPatchName(perf.mHarmPreset));
        }

        if (perf.mTranspose != 0)
        {
            mDisplay.setCursor(1, kFingeredNoteRowY);
            mDisplay.PrintInvertedText(String(perf.mTranspose > 0 ? "+" : "") + perf.mTranspose, kPadding);
        }

        if (perf.mSynthATranspose != 0)
        {
            mDisplay.setCursor(kTextAreaWidth - 14, kFingeredNoteRowY);
            mDisplay.PrintInvertedText(String(perf.mSynthATranspose > 0 ? "+" : "") + perf.mSynthATranspose, kPadding);
        }
        if (perf.mSynthBTranspose != 0)
        {
            mDisplay.setCursor(kTextAreaWidth - 14, kFingeredNoteRowY + 9);
            mDisplay.PrintInvertedText(String(perf.mSynthBTranspose > 0 ? "+" : "") + perf.mSynthBTranspose, kPadding);
        }

        mDisplay.setTextColor(SSD1306_WHITE); // normal text

        const auto fingeredNote = mMusicalStateTask.mMusicalState.mFingeredNote;
        auto fingeredNoteName = fingeredNote.ToStringWithOctave();
        auto fingeredNoteBounds = mDisplay.GetTextBounds(fingeredNoteName);
        // center in text area
        mDisplay.setCursor((kTextAreaWidth - fingeredNoteBounds.width * 2) / 2, kFingeredNoteRowY);
        mDisplay.SetFontScale(2, 2);
        mDisplay.print(fingeredNoteName);
        mDisplay.SetFontScale(1, 1);

        // now playing notes.
        {
            auto voices = mSysInfoProvider.ISysInfoProvider_GetVoiceState();

            fixed_vector<SynthVoiceState, MAX_SYNTH_VOICES> voiceState;
            for (auto &v : voices)
            {
                if (v.mIsPlaying)
                {
                    voiceState.push_back(v);
                }
            }

            std::sort(voiceState.begin(),
                      voiceState.end(), //
                      [](const SynthVoiceState &a, const SynthVoiceState &b) {
                          return a.mNote.GetMidiValue() < b.mNote.GetMidiValue();
                      });

            fixed_vector<MidiNote, MAX_SYNTH_VOICES> chordNotes;
            fixed_vector<SynthVoiceState, MAX_SYNTH_VOICES> prunedVoiceState; // maintain same indices as chordNotes.
            MidiNote lastNote{0};
            for (auto &v : voiceState)
            {
                if (v.mNote.GetMidiValue() == lastNote.GetMidiValue())
                {
                    continue;
                }
                chordNotes.push_back(v.mNote);
                prunedVoiceState.push_back(v);
                lastNote = v.mNote;
            }
            auto spelledChord = spellChord(chordNotes).notes;

            const auto voiceWidth = std::min(18, kPlayingNotesRowWidth / static_cast<int>(spelledChord.size()));

            for (int i = 0; i < (int)spelledChord.size(); ++i)
            {
                auto &tone = spelledChord[i];
                auto &voiceState = prunedVoiceState[i];
                const int x = voiceWidth * i;
                mDisplay.setCursor(x, kPlayingNotesRowY);
                const bool invertText = voiceState.mVoiceSource == VoiceSource::Live;
                mDisplay.PrintInvertedText(tone.ToString(false),
                                           kPadding,
                                           invertText); // becasue we eliminate duplicates, and they're always sorted
                                                        // from low to high, not necessary to include octave.
            }
        }

        BigPerfDisplayApp::RenderPitchbendBar(
            mDisplay, mMusicalStateTask.mMusicalState.mCurrentPitchN11.GetValue(), 51, kTextAreaWidth);
    }
};

} // namespace clarinoid
