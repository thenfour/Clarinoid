#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "BoolSettingItem.hpp"
#include <clarinoid/application/MetronomeVis.hpp>
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
    MetronomeVis mMetronomeVis;

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
                                             [](void *cap, const float &v) FLASHMEM {
                                                 auto *pThis = (MetronomeSettingsApp *)cap;
                                                 pThis->mpAppSettings->GetCurrentPerformancePatch().mBPM = v;
                                                 pThis->mpMetronome->OnBPMChanged();
                                             },
                                             this},
                             AlwaysEnabled};

    // beats per bar
    IntSettingItem mBeatsPerBar = {
        "Beats per bar",
        NumericEditRangeSpec<int>{1, kMaxBeatsPerBar},
        Property<int>{[](void *cap) FLASHMEM {
                          auto *pThis = (MetronomeSettingsApp *)cap;
                          return (int)pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatsPerBar;
                      },
                      [](void *cap, const int &v) FLASHMEM {
                          auto *pThis = (MetronomeSettingsApp *)cap;
                          pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatsPerBar = v;
                      },
                      this},
        AlwaysEnabled};

    IntSettingItem mBeatSubdivisions = {
        "Beat subdivs",
        NumericEditRangeSpec<int>{1, kMaxSubdivisionsPerBeat},
        Property<int>{[](void *cap) FLASHMEM {
                          auto *pThis = (MetronomeSettingsApp *)cap;
                          return (int)pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatSubdivisions;
                      },
                      [](void *cap, const int &v) FLASHMEM {
                          auto *pThis = (MetronomeSettingsApp *)cap;
                          pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatSubdivisions = v;
                      },
                      this},
        AlwaysEnabled};

    // MetronomeVis::kRadius
    FloatSettingItem mRadius = {"Radius",
                                NumericEditRangeSpec{1.0f, 50.0f},
                                Property<float>{[](void *cap) FLASHMEM {
                                                    // auto *pThis = (MetronomeSettingsApp *)cap;
                                                    return MetronomeVis::kRadius;
                                                },
                                                [](void *cap, const float &v) {
                                                    MetronomeVis::kRadius = v;
                                                    MetronomeVis::SettingsChangedTrigger = true;
                                                },
                                                this},
                                AlwaysEnabled};

    // MetronomeVis::kSubdivisionRadius
    FloatSettingItem mSubdivisionRadius = {"SubdivRadius",
                                           NumericEditRangeSpec{1.0f, 20.0f},
                                           Property<float>{[](void *cap) FLASHMEM {
                                                               // auto *pThis = (MetronomeSettingsApp *)cap;
                                                               return MetronomeVis::kSubdivisionRadius;
                                                           },
                                                           [](void *cap, const float &v) {
                                                               MetronomeVis::kSubdivisionRadius = v;
                                                               MetronomeVis::SettingsChangedTrigger = true;
                                                           },
                                                           this},
                                           AlwaysEnabled};

    // kMainFlashHoldMs
    IntSettingItem mFlashHoldMs = {"MainHold",
                                   NumericEditRangeSpec<int>{1, 1000},
                                   Property<int>{[](void *cap) FLASHMEM {
                                                     // auto *pThis = (MetronomeSettingsApp *)cap;
                                                     return (int)MetronomeVis::kMainFlashHoldMs;
                                                 },
                                                 [](void *cap, const int &v) FLASHMEM {
                                                     // auto *pThis = (MetronomeSettingsApp *)cap;
                                                     MetronomeVis::kMainFlashHoldMs = v;
                                                     MetronomeVis::SettingsChangedTrigger = true;
                                                 },
                                                 this},
                                   AlwaysEnabled};

    // kMainFlashDecayMs
    IntSettingItem mFlashDecayMs = {"MainDecay",
                                    NumericEditRangeSpec<int>{1, 5000},
                                    Property<int>{[](void *cap) FLASHMEM {
                                                      // auto *pThis = (MetronomeSettingsApp *)cap;
                                                      return (int)MetronomeVis::kMainFlashDecayMs;
                                                  },
                                                  [](void *cap, const int &v) FLASHMEM {
                                                      // auto *pThis = (MetronomeSettingsApp *)cap;
                                                      MetronomeVis::kMainFlashDecayMs = v;
                                                      MetronomeVis::SettingsChangedTrigger = true;
                                                  },
                                                  this},
                                    AlwaysEnabled};

    // subdiv hold
    IntSettingItem mSubdivisionFlashHoldMs = {"SubdivHold",
                                              NumericEditRangeSpec<int>{1, 1000},
                                              Property<int>{[](void *cap) FLASHMEM {
                                                                // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                return (int)MetronomeVis::kSubdivisionFlashHoldMs;
                                                            },
                                                            [](void *cap, const int &v) FLASHMEM {
                                                                // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                MetronomeVis::kSubdivisionFlashHoldMs = v;
                                                                MetronomeVis::SettingsChangedTrigger = true;
                                                            },
                                                            this},
                                              AlwaysEnabled};

    // subdiv decay
    IntSettingItem mSubdivisionFlashDecayMs = {"SubdivDecay",
                                               NumericEditRangeSpec<int>{1, 5000},
                                               Property<int>{[](void *cap) FLASHMEM {
                                                                 // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                 return (int)MetronomeVis::kSubdivisionFlashDecayMs;
                                                             },
                                                             [](void *cap, const int &v) FLASHMEM {
                                                                 // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                 MetronomeVis::kSubdivisionFlashDecayMs = v;
                                                                 MetronomeVis::SettingsChangedTrigger = true;
                                                             },
                                                             this},
                                               AlwaysEnabled};

    // sweep radians
    FloatSettingItem mSweepWidthRadians = {"SweepWidth",
                                           NumericEditRangeSpec{0.0f, kTwoPI_f},
                                           Property<float>{[](void *cap) FLASHMEM {
                                                               // auto *pThis = (MetronomeSettingsApp *)cap;
                                                               return MetronomeVis::kSweepWidthRadians;
                                                           },
                                                           [](void *cap, const float &v) {
                                                               MetronomeVis::kSweepWidthRadians = v;
                                                               MetronomeVis::SettingsChangedTrigger = true;
                                                           },
                                                           this},
                                           AlwaysEnabled};

    // kSweepMaxBrightnessQp8
    IntSettingItem mSweepMaxBrightnessQp8 = {"SweepQp8",
                                             NumericEditRangeSpec<int>{1, 255},
                                             Property<int>{[](void *cap) FLASHMEM {
                                                               // auto *pThis = (MetronomeSettingsApp *)cap;
                                                               return MetronomeVis::kSweepMaxBrightnessQp8;
                                                           },
                                                           [](void *cap, const int &v) FLASHMEM {
                                                               // auto *pThis = (MetronomeSettingsApp *)cap;
                                                               MetronomeVis::kSweepMaxBrightnessQp8 = v;
                                                               MetronomeVis::SettingsChangedTrigger = true;
                                                           },
                                                           this},
                                             AlwaysEnabled};

    // kSubdivFlashMaxBrightnessQp8
    IntSettingItem mSubdivFlashMaxBrightnessQp8 = {"SubdivQp8",
                                                   NumericEditRangeSpec<int>{1, 255},
                                                   Property<int>{[](void *cap) FLASHMEM {
                                                                     // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                     return MetronomeVis::kSubdivFlashMaxBrightnessQp8;
                                                                 },
                                                                 [](void *cap, const int &v) FLASHMEM {
                                                                     // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                     MetronomeVis::kSubdivFlashMaxBrightnessQp8 = v;
                                                                     MetronomeVis::SettingsChangedTrigger = true;
                                                                 },
                                                                 this},
                                                   AlwaysEnabled};

    // kBeatFlashMaxBrightnessQp8
    IntSettingItem mBeatFlashMaxBrightnessQp8 = {"BeatQp8",
                                                 NumericEditRangeSpec<int>{1, 255},
                                                 Property<int>{[](void *cap) FLASHMEM {
                                                                   // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                   return MetronomeVis::kBeatFlashMaxBrightnessQp8;
                                                               },
                                                               [](void *cap, const int &v) FLASHMEM {
                                                                   // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                   MetronomeVis::kBeatFlashMaxBrightnessQp8 = v;
                                                                   MetronomeVis::SettingsChangedTrigger = true;
                                                               },
                                                               this},
                                                 AlwaysEnabled};

    // kBigFlashMaxBrightnessQp8
    IntSettingItem mBigFlashMaxBrightnessQp8 = {"BigQp8",
                                                NumericEditRangeSpec<int>{1, 255},
                                                Property<int>{[](void *cap) FLASHMEM {
                                                                  // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                  return MetronomeVis::kBigFlashMaxBrightnessQp8;
                                                              },
                                                              [](void *cap, const int &v) FLASHMEM {
                                                                  // auto *pThis = (MetronomeSettingsApp *)cap;
                                                                  MetronomeVis::kBigFlashMaxBrightnessQp8 = v;
                                                                  MetronomeVis::SettingsChangedTrigger = true;
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
                                                   [](void *cap, const bool &v) FLASHMEM {
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
                                                 [](void *cap, const bool &v) FLASHMEM {
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

    ISettingItem *mArray[21] = {
        &mBPM, //

        &mBeatsPerBar, //
        &mBeatSubdivisions,
        &mRadius,                 //
        &mSubdivisionRadius,      //
        &mFlashHoldMs,            //
        &mFlashDecayMs,           //
        &mSubdivisionFlashHoldMs, //
        &mSubdivisionFlashDecayMs,
        &mSweepWidthRadians, //
        &mSweepMaxBrightnessQp8,
        &mSubdivFlashMaxBrightnessQp8,
        &mBeatFlashMaxBrightnessQp8,
        &mBigFlashMaxBrightnessQp8,

        &mGain,
        &mSoundEnable, //
        &mLEDEnable,
        &mLEDDecay, //
        &mLEDBrightness,
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

        mMetronomeVis.Render(*mpMetronome, mDisplay, *mpAppSettings, {});

        // float beatFloat = mpMetronome->GetBeatFloat();
        // float beatFrac = beatFloat - floor(beatFloat);
        // int beatInt = (int)floor(beatFloat);
        // bool altBeat = (beatInt & 1) != 0;

        // bool highlight = beatFrac < 0.1;

        // mDisplay.ClearState();
        // if (highlight)
        // {
        //     mDisplay.fillScreen(WHITE);
        // }
        // mDisplay.setTextColor(highlight ? BLACK : WHITE);

        mDisplay.println(String("METRONOME"));
        // mDisplay.print(mpAppSettings->mMetronomeSoundOn ? "SoundOn" : "SoundOff");
        // mDisplay.print(" ");
        // mDisplay.println(mpAppSettings->mMetronomeSoundOn ? "LEDOn" : "LEDOff");
        mDisplay.println(String("bpm ") + (int)std::round(mpAppSettings->GetCurrentPerformancePatch().mBPM));
        mDisplay.println(String("n   ") + mpAppSettings->GetCurrentPerformancePatch().mBeatsPerBar);
        mDisplay.println(String("sub ") + mpAppSettings->GetCurrentPerformancePatch().mBeatSubdivisions);

        // const int r = 4;
        // int x = beatFrac * (MAX_DISPLAY_WIDTH - r * 2);
        // if (altBeat)
        //     x = mDisplay.width() - x;
        // mDisplay.fillCircle(x, mDisplay.getCursorY() + r, r, highlight ? BLACK : WHITE);

        // mDisplay.println(String(""));
        // mDisplay.println(String("                  -->"));

        SettingsMenuApp::RenderFrontPage();
    }

    // virtual ISettingItemEditor* GetBackEditor() {
    //   return mBPM.GetEditor(0);
    // }
};

} // namespace clarinoid
