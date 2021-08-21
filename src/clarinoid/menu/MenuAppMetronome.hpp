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

    MetronomeSettingsApp(Metronome *pm, AppSettings *pas, IDisplay &d)
        : SettingsMenuApp(d), mpMetronome(pm), mpAppSettings(pas)
    {
    }

    Property<bool> EnabledIfSoundOn = Property<bool>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (MetronomeSettingsApp *)cap;
                                                         return pThis->mpAppSettings->mMetronomeSoundOn;
                                                     },
                                                     this};

    Property<bool> EnabledIfLEDOn = Property<bool>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       return pThis->mpAppSettings->mMetronomeLED;
                                                   },
                                                   this};

    FloatSettingItem mBPM = {"BPM",
                             StandardRangeSpecs::gBPMRange,
                             Property<float>{[](void *cap) FLASHMEM {
                                                 auto *pThis = (MetronomeSettingsApp *)cap;
                                                 return pThis->mpAppSettings->GetCurrentPerformancePatch().mBPM;
                                             },
                                             [](void *cap, const float &v) {
                                                 auto *pThis = (MetronomeSettingsApp *)cap;
                                                 pThis->mpAppSettings->GetCurrentPerformancePatch().mBPM = v;
                                                 pThis->mpMetronome->OnBPMChanged();
                                             },
                                             this},
                             AlwaysEnabled};

    BoolSettingItem mSoundEnable = {"SoundEnable",
                                    "On",
                                    "Off",
                                    Property<bool>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       return pThis->mpAppSettings->mMetronomeSoundOn;
                                                   },
                                                   [](void *cap, const bool &v) {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       pThis->mpAppSettings->mMetronomeSoundOn = v;
                                                   },
                                                   this},
                                    AlwaysEnabled};

    BoolSettingItem mLEDEnable = {"LED enable",
                                  "On",
                                  "Off",
                                  Property<bool>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (MetronomeSettingsApp *)cap;
                                                     return pThis->mpAppSettings->mMetronomeLED;
                                                 },
                                                 [](void *cap, const bool &v) {
                                                     auto *pThis = (MetronomeSettingsApp *)cap;
                                                     pThis->mpAppSettings->mMetronomeLED = v;
                                                 },
                                                 this},
                                  AlwaysEnabled};

    FloatSettingItem mLEDDecay = {"LED decay",
                                  StandardRangeSpecs::gFloat_0_1,
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (MetronomeSettingsApp *)cap;
                                                      return pThis->mpAppSettings->mMetronomeLEDDecay;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (MetronomeSettingsApp *)cap;
                                                      pThis->mpAppSettings->mMetronomeLEDDecay = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    IntSettingItem mLEDBrightness = {"LED brightness",
                                     NumericEditRangeSpec<int>{1, 255},
                                     Property<int>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       return pThis->mpAppSettings->mMetronomeBrightness;
                                                   },
                                                   [](void *cap, const int &v) {
                                                       auto *pThis = (MetronomeSettingsApp *)cap;
                                                       pThis->mpAppSettings->mMetronomeBrightness = v;
                                                   },
                                                   this},
                                     AlwaysEnabled};

    FloatSettingItem mGain = {"Gain",
                              StandardRangeSpecs::gFloat_0_1,
                              Property<float>{[](void *cap) FLASHMEM {
                                                  auto *pThis = (MetronomeSettingsApp *)cap;
                                                  return pThis->mpAppSettings->mMetronomeGain;
                                              },
                                              [](void *cap, const float &v) {
                                                  auto *pThis = (MetronomeSettingsApp *)cap;
                                                  pThis->mpAppSettings->mMetronomeGain = v;
                                              },
                                              this},
                              EnabledIfSoundOn};

    IntSettingItem mNote = {"Note",
                            StandardRangeSpecs::gMetronomeNoteRange,
                            Property<int>{[](void *cap) FLASHMEM {
                                              auto *pThis = (MetronomeSettingsApp *)cap;
                                              return pThis->mpAppSettings->mMetronomeNote;
                                          },
                                          [](void *cap, const int &v) {
                                              auto *pThis = (MetronomeSettingsApp *)cap;
                                              pThis->mpAppSettings->mMetronomeNote = v;
                                          },
                                          this},
                            EnabledIfSoundOn};

    IntSettingItem mDecay = {"Decay",
                             StandardRangeSpecs::gMetronomeDecayRange,
                             Property<int>{[](void *cap) FLASHMEM {
                                               auto *pThis = (MetronomeSettingsApp *)cap;
                                               return pThis->mpAppSettings->mMetronomeDecayMS;
                                           },
                                           [](void *cap, const int &v) {
                                               auto *pThis = (MetronomeSettingsApp *)cap;
                                               pThis->mpAppSettings->mMetronomeDecayMS = v;
                                           },
                                           this},
                             EnabledIfSoundOn};

    ISettingItem *mArray[8] = {&mBPM, &mGain, &mSoundEnable, &mLEDEnable, &mLEDDecay, &mLEDBrightness, &mNote, &mDecay};
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
        mDisplay.print(mpAppSettings->mMetronomeSoundOn ? "SoundOn" : "SoundOff");
        mDisplay.print(" ");
        mDisplay.println(mpAppSettings->mMetronomeSoundOn ? "LEDOn" : "LEDOff");
        mDisplay.println(String(" bpm=") + mpAppSettings->GetCurrentPerformancePatch().mBPM);

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
