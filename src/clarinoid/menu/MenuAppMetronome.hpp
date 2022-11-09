#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "BoolSettingItem.hpp"
#include <clarinoid/application/Metronome.hpp>

namespace clarinoid
{

struct MetronomeSettingsApp : public SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "MetronomeSettingsApp";
    }

    Metronome *mpMetronome;
    AppSettings *mpAppSettings;

    MetronomeSettingsApp(Metronome *pm, AppSettings *pas, IDisplay &d, InputDelegator &input)
        : SettingsMenuApp(d, *pas, input), mpMetronome(pm), mpAppSettings(pas)
    {
    }

    Property<bool> EnabledIfSoundOn = Property<bool>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (MetronomeSettingsApp *)cap;
                                                         return pThis->mpAppSettings->mMetronome.mSoundOn.GetValue();
                                                     },
                                                     this};

    Property<bool> EnabledIfLEDOn = Property<bool>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       return pThis->mpAppSettings->mMetronome.mLEDOn.GetValue();
                                                   },
                                                   this};

    FloatSettingItem mBPM = {
        "BPM",
        StandardRangeSpecs::gBPMRange,
        Property<float>{[](void *cap) FLASHMEM {
                            auto *pThis = (MetronomeSettingsApp *)cap;
                            return pThis->mpAppSettings->GetCurrentPerformancePatch().mBPM.GetValue();
                        },
                        [](void *cap, const float &v) {
                            auto *pThis = (MetronomeSettingsApp *)cap;
                            pThis->mpAppSettings->GetCurrentPerformancePatch().mBPM.SetValue(v);
                            pThis->mpMetronome->OnBPMChanged();
                        },
                        this},
        AlwaysEnabled};

    BoolSettingItem mSoundEnable = {"SoundEnable",
                                    "On",
                                    "Off",
                                    Property<bool>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       return pThis->mpAppSettings->mMetronome.mSoundOn.GetValue();
                                                   },
                                                   [](void *cap, const bool &v) {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       pThis->mpAppSettings->mMetronome.mSoundOn.SetValue(v);
                                                   },
                                                   this},
                                    AlwaysEnabled};

    BoolSettingItem mLEDEnable = {"LED enable",
                                  "On",
                                  "Off",
                                  Property<bool>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (MetronomeSettingsApp *)cap;
                                                     return pThis->mpAppSettings->mMetronome.mLEDOn.GetValue();
                                                 },
                                                 [](void *cap, const bool &v) {
                                                     auto *pThis = (MetronomeSettingsApp *)cap;
                                                     pThis->mpAppSettings->mMetronome.mLEDOn.SetValue(v);
                                                 },
                                                 this},
                                  AlwaysEnabled};

    // FloatSettingItem mLEDDecay = {"LED decay",
    //                               StandardRangeSpecs::gFloat_0_1,
    //                               Property<float>{[](void *cap) FLASHMEM {
    //                                                   auto *pThis = (MetronomeSettingsApp *)cap;
    //                                                   return pThis->mpAppSettings->mMetronome.mle;
    //                                               },
    //                                               [](void *cap, const float &v) {
    //                                                   auto *pThis = (MetronomeSettingsApp *)cap;
    //                                                   pThis->mpAppSettings->mMetronome.mMetronomeLEDDecay = v;
    //                                               },
    //                                               this},
    //                               AlwaysEnabled};

    // IntSettingItem mLEDBrightness = {"LED brightness",
    //                                  NumericEditRangeSpec<int>{1, 255},
    //                                  Property<int>{[](void *cap) FLASHMEM {
    //                                                    auto *pThis = (MetronomeSettingsApp *)cap;
    //                                                    return pThis->mpAppSettings->mMetronome.mMetronomeBrightness;
    //                                                },
    //                                                [](void *cap, const int &v) {
    //                                                    auto *pThis = (MetronomeSettingsApp *)cap;
    //                                                    pThis->mpAppSettings->mMetronome.mMetronomeBrightness = v;
    //                                                },
    //                                                this},
    //                                  AlwaysEnabled};

    FloatSettingItem mGain = {"Gain",
                              StandardRangeSpecs::gFloat_0_1,
                              Property<float>{[](void *cap) FLASHMEM {
                                                  auto *pThis = (MetronomeSettingsApp *)cap;
                                                  return pThis->mpAppSettings->mMetronome.mGain.GetValue();
                                              },
                                              [](void *cap, const float &v) {
                                                  auto *pThis = (MetronomeSettingsApp *)cap;
                                                  pThis->mpAppSettings->mMetronome.mGain.SetValue(v);
                                              },
                                              this},
                              EnabledIfSoundOn};

    IntSettingItem mNote = {"Note",
                            StandardRangeSpecs::gMetronomeNoteRange,
                            Property<int>{[](void *cap) FLASHMEM {
                                              auto *pThis = (MetronomeSettingsApp *)cap;
                                              return (int)pThis->mpAppSettings->mMetronome.mMidiNote.GetValue();
                                          },
                                          [](void *cap, const int &v) {
                                              auto *pThis = (MetronomeSettingsApp *)cap;
                                              pThis->mpAppSettings->mMetronome.mMidiNote.SetValue(v);
                                          },
                                          this},
                            EnabledIfSoundOn};

    IntSettingItem mDecay = {"Decay",
                             StandardRangeSpecs::gMetronomeDecayRange,
                             Property<int>{[](void *cap) FLASHMEM {
                                               auto *pThis = (MetronomeSettingsApp *)cap;
                                               return pThis->mpAppSettings->mMetronome.mDecayMS.GetValue();
                                           },
                                           [](void *cap, const int &v) {
                                               auto *pThis = (MetronomeSettingsApp *)cap;
                                               pThis->mpAppSettings->mMetronome.mDecayMS.SetValue(v);
                                           },
                                           this},
                             EnabledIfSoundOn};

    ISettingItem *mArray[6] = {
        //
        &mBPM,
        &mGain,        //
        &mSoundEnable, //
        &mLEDEnable,   //
        &mNote,
        &mDecay, //
    };
    SettingsList mRootList = {mArray};

    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        float beatFloat = mpMetronome->GetBeatFloat();
        float beatFrac = beatFloat - floor(beatFloat);
        int beatInt = (int)floor(beatFloat);
        bool altBeat = (beatInt & 1) != 0;

        bool highlight = beatFrac < 0.1;

        mDisplay.ClearState();
        if (highlight)
        {
            mDisplay.fillScreen(WHITE);
        }
        mDisplay.setTextColor(highlight ? BLACK : WHITE);

        mDisplay.println(String("METRONOME"));
        mDisplay.print(mpAppSettings->mMetronome.mSoundOn.GetValue() ? "SoundOn" : "SoundOff");
        mDisplay.print(" ");
        mDisplay.println(mpAppSettings->mMetronome.mSoundOn.GetValue() ? "LEDOn" : "LEDOff");
        mDisplay.println(String(" bpm=") + mpAppSettings->GetCurrentPerformancePatch().mBPM.GetValue());

        const int r = 4;
        int x = beatFrac * (MAX_DISPLAY_WIDTH - r * 2);
        if (altBeat)
            x = mDisplay.width() - x;
        mDisplay.fillCircle(x, mDisplay.getCursorY() + r, r, highlight ? BLACK : WHITE);

        mDisplay.println(String(""));
        mDisplay.println(String("                  -->"));

        SettingsMenuApp::RenderFrontPage();
    }

    // virtual ISettingItemEditor* GetBackEditor() {
    //   return mBPM.GetEditor(0);
    // }
};

} // namespace clarinoid
