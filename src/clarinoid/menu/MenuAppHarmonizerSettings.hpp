#pragma once

#include "BoolSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "FunctionListSettingItem.hpp"
#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"

namespace clarinoid
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct HarmVoiceSettingsApp
{

    HarmVoiceSettingsApp(CCDisplay &d) : mDisplay(d)
    {
    }

    CCDisplay &mDisplay;

    int mEditingHarmVoice = -1;

    HarmVoiceSettings &EditingVoice()
    {
        CCASSERT(mEditingHarmVoice >= 0 && mEditingHarmVoice < (int)clarinoid::HARM_VOICES);
        auto &perf = mDisplay.mAppSettings->GetCurrentPerformancePatch();
        auto &h = mDisplay.mAppSettings->FindHarmPreset(perf.mHarmPreset);
        return h.mVoiceSettings[mEditingHarmVoice];
    }

    EnumSettingItem<HarmSynthPresetRefType> mSynthPresetRef = {"Synth patch ref",
                                                               gHarmSynthPresetRefTypeInfo,
                                                               Property<HarmSynthPresetRefType>{
                                                                   [](void *cap) {
                                                                       auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                                       return pThis->EditingVoice().mSynthPresetRef;
                                                                   }, // getter
                                                                   [](void *cap, const HarmSynthPresetRefType &val) {
                                                                       auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                                       pThis->EditingVoice().mSynthPresetRef = val;
                                                                   },   // setter
                                                                   this // capture val
                                                               },
                                                               AlwaysEnabled};

    IntSettingItem mOwnSynthPatch = {"Own synth patch",
                                     NumericEditRangeSpec<int>{0, clarinoid::SYNTH_PRESET_COUNT - 1},
                                     Property<int>{
                                         [](void *cap) {
                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                             return (int)pThis->EditingVoice().mVoiceSynthPreset;
                                         }, // getter
                                         [](void *cap, const int &val) {
                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                             pThis->EditingVoice().mVoiceSynthPreset = val;
                                         },   // setter
                                         this // capture val
                                     },
                                     [](void *cap, int n) { // formatter
                                         auto *pThis = (HarmVoiceSettingsApp *)cap;
                                         return pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[n].ToString(n);
                                     },
                                     Property<bool>{// enabled only if ref type = voice.
                                                    [](void *cap) {
                                                        auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                        return pThis->EditingVoice().mSynthPresetRef ==
                                                               HarmSynthPresetRefType::Voice;
                                                    },
                                                    this},
                                     this};

    IntSettingItem mSequenceLength = {"Seq length",
                                      NumericEditRangeSpec<int>{0, clarinoid::HARM_SEQUENCE_LEN - 1},
                                      Property<int>{
                                          [](void *cap) {
                                              auto *pThis = (HarmVoiceSettingsApp *)cap;
                                              return (int)pThis->EditingVoice().mSequenceLength;
                                          }, // getter
                                          [](void *cap, const int &val) {
                                              auto *pThis = (HarmVoiceSettingsApp *)cap;
                                              pThis->EditingVoice().mSequenceLength = val;
                                          },   // setter
                                          this // capture val
                                      },
                                      AlwaysEnabled};

    MultiIntSettingItem mSequence = {
        // actually signed int8_t
        [](void *) { return (size_t)clarinoid::HARM_SEQUENCE_LEN; }, // get item count
        NumericEditRangeSpec<int>(-48, 48),
        [](void *, size_t multiIndex) { return String(String("#") + multiIndex); }, // label getter
        [](void *cap, size_t multiIndex) {                                          // value as string getter
            auto *pThis = (HarmVoiceSettingsApp *)cap;
            return String(pThis->EditingVoice().mSequence[multiIndex]);
        },
        [](void *cap, size_t multiIndex) // value getter
        {
            auto *pThis = (HarmVoiceSettingsApp *)cap;
            return (int)pThis->EditingVoice().mSequence[multiIndex];
        },
        [](void *cap, size_t multiIndex, const int &val) { // value setter
            auto *pThis = (HarmVoiceSettingsApp *)cap;
            pThis->EditingVoice().mSequence[multiIndex] = (int8_t)val;
        },
        [](void *cap, size_t multiIndex) { // is enabled
            auto *pThis = (HarmVoiceSettingsApp *)cap;
            return (multiIndex + 1) <= pThis->EditingVoice().mSequenceLength;
        },
        this // capture
    };

    IntSettingItem mMinOutpNote = {"Min note",
                                   NumericEditRangeSpec<int>{0, 127},
                                   Property<int>{
                                       [](void *cap) {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           return (int)pThis->EditingVoice().mMinOutpNote;
                                       }, // getter
                                       [](void *cap, const int &val) {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           pThis->EditingVoice().mMinOutpNote = val;
                                       },   // setter
                                       this // capture val
                                   },
                                   AlwaysEnabled};

    IntSettingItem mMaxOutpNote = {"Max note",
                                   NumericEditRangeSpec<int>{0, 127},
                                   Property<int>{
                                       [](void *cap) {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           return (int)pThis->EditingVoice().mMaxOutpNote;
                                       }, // getter
                                       [](void *cap, const int &val) {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           pThis->EditingVoice().mMaxOutpNote = val;
                                       },   // setter
                                       this // capture val
                                   },
                                   AlwaysEnabled};

    EnumSettingItem<NoteOOBBehavior> mOOBBehavior = {"OOB behav",
                                                     gNoteOOBBehaviorInfo,
                                                     Property<NoteOOBBehavior>{
                                                         [](void *cap) {
                                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                             return pThis->EditingVoice().mNoteOOBBehavior;
                                                         }, // getter
                                                         [](void *cap, const NoteOOBBehavior &val) {
                                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                             pThis->EditingVoice().mNoteOOBBehavior = val;
                                                         },   // setter
                                                         this // capture val
                                                     },
                                                     AlwaysEnabled};

    EnumSettingItem<HarmScaleRefType> mScaleRefType = {"Scale ref",
                                                       gHarmScaleRefTypeInfo,
                                                       Property<HarmScaleRefType>{
                                                           [](void *cap) {
                                                               auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                               return pThis->EditingVoice().mScaleRef;
                                                           }, // getter
                                                           [](void *cap, const HarmScaleRefType &val) {
                                                               auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                               pThis->EditingVoice().mScaleRef = val;
                                                           },   // setter
                                                           this // capture val
                                                       },
                                                       AlwaysEnabled};

    EnumSettingItem<Note> mLocalScaleNote = {
        "My Scale Root",
        gNoteInfo,
        Property<Note>{
            [](void *cap) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                return pThis->EditingVoice().mLocalScale.mRootNoteIndex;
            }, // getter
            [](void *cap, const Note &val) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                pThis->EditingVoice().mLocalScale.mRootNoteIndex = val;
            },   // setter
            this // capture val
        },
        AlwaysEnabled
        //   [](void* cap) {
        //         auto *pThis = (HarmVoiceSettingsApp *)cap;
        //         return pThis->EditingVoice().mScaleRef ==
        //         HarmScaleRefType::Voice;
        //   }
    };

    EnumSettingItem<ScaleFlavorIndex> mLocalScaleFlavor = {
        " ->Flav",
        gScaleFlavorIndexInfo,
        Property<ScaleFlavorIndex>{
            [](void *cap) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                return pThis->EditingVoice().mLocalScale.mFlavorIndex;
            }, // getter
            [](void *cap, const ScaleFlavorIndex &val) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                pThis->EditingVoice().mLocalScale.mFlavorIndex = val;
            },   // setter
            this // capture val
        },
        AlwaysEnabled
        //   [](void* cap) {
        //         auto *pThis = (HarmVoiceSettingsApp *)cap;
        //         return pThis->EditingVoice().mScaleRef ==
        //         HarmScaleRefType::Voice;
        //   }
    };

    EnumSettingItem<NonDiatonicBehavior> mNonDiatonicBehavior = {
        "OOS behav",
        gNonDiatonicBehaviorInfo,
        Property<NonDiatonicBehavior>{
            [](void *cap) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                return pThis->EditingVoice().mNonDiatonicBehavior;
            }, // getter
            [](void *cap, const NonDiatonicBehavior &val) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                pThis->EditingVoice().mNonDiatonicBehavior = val;
            },   // setter
            this // capture val
        },
        AlwaysEnabled};

    EnumSettingItem<PitchBendParticipation> mPitchbendBehav = {
        "PB behav",
        gPitchBendParticipationInfo,
        Property<PitchBendParticipation>{
            [](void *cap) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                return pThis->EditingVoice().mPitchBendParticipation;
            }, // getter
            [](void *cap, const PitchBendParticipation &val) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                pThis->EditingVoice().mPitchBendParticipation = val;
            },   // setter
            this // capture val
        },
        AlwaysEnabled};

    ISettingItem *mArray[12] = {
        &mSynthPresetRef,
        &mOwnSynthPatch,
        &mSequenceLength,
        &mSequence,
        &mMinOutpNote,
        &mMaxOutpNote,
        &mOOBBehavior,
        &mPitchbendBehav,
        &mScaleRefType,
        &mLocalScaleNote,
        &mLocalScaleFlavor,
        &mNonDiatonicBehavior,
    };
    SettingsList mRootList = {mArray};
}; // namespace clarinoid

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct HarmPatchSettingsApp : public SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "HarmPatchSettingsApp";
    }

    HarmPatchSettingsApp(CCDisplay &d) : SettingsMenuApp(d), mDisplay(d), mHarmVoiceSettingsApp(d)
    {
    }

    CCDisplay &mDisplay;
    HarmVoiceSettingsApp mHarmVoiceSettingsApp;

    HarmPreset &EditingPreset()
    {
        auto &perf = this->mDisplay.mAppSettings->GetCurrentPerformancePatch();
        return this->mDisplay.mAppSettings->mHarmSettings.mPresets[perf.mHarmPreset];
    }

    BoolSettingItem mEmitLiveNote = {"Emit live note?",
                                     "Yes",
                                     "No",
                                     Property<bool>{
                                         [](void *cap) { // getter
                                             auto *pThis = (HarmPatchSettingsApp *)cap;
                                             return pThis->EditingPreset().mEmitLiveNote;
                                         },
                                         [](void *cap, const bool &val) { // setter
                                             auto *pThis = (HarmPatchSettingsApp *)cap;
                                             pThis->EditingPreset().mEmitLiveNote = val;
                                         },
                                         this // capture val
                                     },
                                     AlwaysEnabled};

    FloatSettingItem mStereoSeparation = {"Stereo Sep",
                                          StandardRangeSpecs::gFloat_0_1,
                                          Property<float>{[](void *cap) {
                                                              auto *pThis = (HarmPatchSettingsApp *)cap;
                                                              return pThis->EditingPreset().mStereoSeparation;
                                                          },
                                                          [](void *cap, const float &v) {
                                                              auto *pThis = (HarmPatchSettingsApp *)cap;
                                                              pThis->EditingPreset().mStereoSeparation = v;
                                                          },
                                                          this},
                                          AlwaysEnabled};

    IntSettingItem mMinRotationTimeMS = {"Min Rotation MS",
                                         NumericEditRangeSpec<int>{0, 10000},
                                         Property<int>{
                                             [](void *cap) {
                                                 auto *pThis = (HarmPatchSettingsApp *)cap;
                                                 return (int)pThis->EditingPreset().mMinRotationTimeMS;
                                             }, // getter
                                             [](void *cap, const int &val) {
                                                 auto *pThis = (HarmPatchSettingsApp *)cap;
                                                 pThis->EditingPreset().mMinRotationTimeMS = val;
                                             },   // setter
                                             this // capture val
                                         },
                                         AlwaysEnabled};

    IntSettingItem mSynthPreset1 = {"Synth preset 1",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPreset1;
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPreset1 = val;
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[n].ToString(n);
                                    },
                                    AlwaysEnabled,
                                    this};

    IntSettingItem mSynthPreset2 = {"Synth preset 2",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPreset2;
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPreset2 = val;
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[n].ToString(n);
                                    },
                                    AlwaysEnabled,
                                    this};

    IntSettingItem mSynthPreset3 = {"Synth preset 3",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPreset3;
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPreset3 = val;
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[n].ToString(n);
                                    },
                                    AlwaysEnabled,
                                    this};

    IntSettingItem mSynthPreset4 = {"Synth preset 4",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPreset4;
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPreset4 = val;
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[n].ToString(n);
                                    },
                                    AlwaysEnabled,
                                    this};

    EnumSettingItem<Note> mPresetScaleNote = {"Scale Root",
                                              gNoteInfo,
                                              Property<Note>{
                                                  [](void *cap) {
                                                      auto *pThis = (HarmPatchSettingsApp *)cap;
                                                      return pThis->EditingPreset().mPresetScale.mRootNoteIndex;
                                                  }, // getter
                                                  [](void *cap, const Note &val) {
                                                      auto *pThis = (HarmPatchSettingsApp *)cap;
                                                      pThis->EditingPreset().mPresetScale.mRootNoteIndex = val;
                                                  },   // setter
                                                  this // capture val
                                              },
                                              AlwaysEnabled};

    EnumSettingItem<ScaleFlavorIndex> mPresetScaleFlavor = {
        " ->Flav",
        gScaleFlavorIndexInfo,
        Property<ScaleFlavorIndex>{
            [](void *cap) {
                auto *pThis = (HarmPatchSettingsApp *)cap;
                return pThis->EditingPreset().mPresetScale.mFlavorIndex;
            }, // getter
            [](void *cap, const ScaleFlavorIndex &val) {
                auto *pThis = (HarmPatchSettingsApp *)cap;
                pThis->EditingPreset().mPresetScale.mFlavorIndex = val;
            },   // setter
            this // capture val
        },
        AlwaysEnabled};

    MultiSubmenuSettingItem mVoiceSubmenu = {[](void *cap) { return clarinoid::HARM_VOICES; },
                                             [](void *cap, size_t mi) { // name
                                                 auto *pThis = (HarmPatchSettingsApp *)cap;
                                                 // like
                                                 // Voice 5 [-2,2]
                                                 return (String)(
                                                     String("Voice ") + mi + "" +
                                                     pThis->EditingPreset().mVoiceSettings[mi].GetMenuDetailString());
                                             },                         // return string name
                                             [](void *cap, size_t mi) { // get submenu for item
                                                 auto *pThis = (HarmPatchSettingsApp *)cap;
                                                 pThis->mHarmVoiceSettingsApp.mEditingHarmVoice = mi;
                                                 return &pThis->mHarmVoiceSettingsApp.mRootList;
                                             },                                         // return submenu
                                             [](void *cap, size_t mi) { return true; }, // is enabled
                                             this};                                     // capture

    FunctionListSettingItem mCopyPreset = {
        "Copy to ...",
        HARM_PRESET_COUNT,
        [](void *cap, size_t i) { // itemNameGetter,
            auto *pThis = (HarmPatchSettingsApp *)cap;
            return String(String("") + i + ":" + pThis->mDisplay.mAppSettings->mHarmSettings.mPresets[i].mName);
        },
        [](void *cap,
           size_t i) { // cc::function<void(void*,size_t)>::ptr_t onClick,
            auto *pThis = (HarmPatchSettingsApp *)cap;
            pThis->mDisplay.mAppSettings->mHarmSettings.mPresets[i] = pThis->EditingPreset();
            auto &settings = *pThis->mDisplay.mAppSettings;
            auto &perf = settings.GetCurrentPerformancePatch();
            auto fromName = settings.GetHarmPatchName(perf.mHarmPreset);
            auto toName = settings.GetHarmPatchName(i);
            pThis->mDisplay.ShowToast(String("Copied ") + fromName + "\nto\n" + toName);
        },
        AlwaysEnabled,
        this};

    ISettingItem *mArray[11] = {
        &mEmitLiveNote,
        &mStereoSeparation,
        &mMinRotationTimeMS,
        &mPresetScaleNote,
        &mPresetScaleFlavor,
        &mSynthPreset1,
        &mSynthPreset2,
        &mSynthPreset3,
        &mSynthPreset4,
        &mVoiceSubmenu,
        &mCopyPreset,
    };
    SettingsList mRootList = {mArray};

    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.ClearState();
        mDisplay.mDisplay.println(String("Harmonizer"));
        mDisplay.mDisplay.println(String("     patch >>"));
        auto &perf = GetAppSettings()->GetCurrentPerformancePatch();
        auto name = GetAppSettings()->GetHarmPatchName(perf.mHarmPreset);
        mDisplay.mDisplay.println(name);
        SettingsMenuApp::RenderFrontPage();
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct HarmSettingsApp : public SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "HarmSettingsApp";
    }

    HarmPatchSettingsApp mHarmPatchSettings;

    HarmSettingsApp(CCDisplay &d) : SettingsMenuApp(d), mHarmPatchSettings(d)
    {
    }

    MultiSubmenuSettingItem mPatchSubmenu = {
        [](void *cap) { return clarinoid::HARM_PRESET_COUNT; }, // get item count
        [](void *cap, size_t mi) {                              // get item name
            auto *pThis = (HarmSettingsApp *)cap;
            return (String)(String("") + mi + ":" + pThis->GetAppSettings()->mHarmSettings.mPresets[mi].mName);
        },
        [](void *cap, size_t mi) { // get submenu
            auto *pThis = (HarmSettingsApp *)cap;
            pThis->GetAppSettings()->GetCurrentPerformancePatch().mHarmPreset = mi;
            return &pThis->mHarmPatchSettings.mRootList;
        },
        [](void *cap, size_t mi) { return true; }, // is enabled?
        this};                                     // cap

    ISettingItem *mArray[1] = {&mPatchSubmenu};
    SettingsList mRootList = {mArray};

    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        this->mDisplay.ClearState();
        this->mDisplay.mDisplay.println("Harmonizer >");

        auto& perf = this->GetAppSettings()->GetCurrentPerformancePatch();
        auto name = this->GetAppSettings()->GetHarmPatchName(perf.mHarmPreset);
        //auto& h = this->GetAppSettings()->FindHarmPreset(perf.mHarmPreset);

        this->mDisplay.mDisplay.println(name);
        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
