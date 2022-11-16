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

    HarmVoiceSettingsApp(IDisplay &d, AppSettings &appSettings, InputDelegator &input)
        : mDisplay(d), mAppSettings(appSettings)
    {
    }

    IDisplay &mDisplay;
    AppSettings &mAppSettings;

    int mEditingHarmVoice = -1;

    HarmVoiceSettings &EditingVoice()
    {
        CCASSERT(mEditingHarmVoice >= 0 && mEditingHarmVoice < (int)clarinoid::HARM_VOICES);
        auto &perf = mAppSettings.GetCurrentPerformancePatch();
        auto &h = mAppSettings.FindHarmPreset(perf.mHarmPreset.GetValue());
        return h.mVoiceSettings[mEditingHarmVoice];
    }

    EnumSettingItem<HarmSynthPresetRefType> mSynthPresetRef = {
        "Synth patch ref",
        gHarmSynthPresetRefTypeInfo,
        Property<HarmSynthPresetRefType>{
            [](void *cap) FLASHMEM {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                return pThis->EditingVoice().mSynthPatchRef.GetValue();
            }, // getter
            [](void *cap, const HarmSynthPresetRefType &val) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                pThis->EditingVoice().mSynthPatchRef.SetValue(val);
            },   // setter
            this // capture val
        },
        AlwaysEnabled};

    IntSettingItem mOwnSynthPatch = {"Own synth patch",
                                     NumericEditRangeSpec<int>{0, clarinoid::SYNTH_PRESET_COUNT - 1},
                                     Property<int>{
                                         [](void *cap) FLASHMEM {
                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                             return (int)pThis->EditingVoice().mSynthPatch.GetValue();
                                         }, // getter
                                         [](void *cap, const int &val) {
                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                             pThis->EditingVoice().mSynthPatch.SetValue(val);
                                         },   // setter
                                         this // capture val
                                     },
                                     [](void *cap, int n) { // formatter
                                         auto *pThis = (HarmVoiceSettingsApp *)cap;
                                         return pThis->mAppSettings.mSynthSettings.mPatches[n].ToString();
                                     },
                                     Property<bool>{// enabled only if ref type = voice.
                                                    [](void *cap) FLASHMEM {
                                                        auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                        return pThis->EditingVoice().mSynthPatchRef.GetValue() ==
                                                               HarmSynthPresetRefType::Voice;
                                                    },
                                                    this},
                                     this};

    IntSettingItem mSequenceLength = {"Seq length",
                                      NumericEditRangeSpec<int>{0, clarinoid::HARM_SEQUENCE_LEN - 1},
                                      Property<int>{
                                          [](void *cap) FLASHMEM {
                                              auto *pThis = (HarmVoiceSettingsApp *)cap;
                                              return (int)pThis->EditingVoice().mSequence.mLength;
                                          }, // getter
                                          [](void *cap, const int &val) {
                                              auto *pThis = (HarmVoiceSettingsApp *)cap;
                                              pThis->EditingVoice().mSequence.mLength = val;
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
            return (multiIndex + 1) <= pThis->EditingVoice().mSequence.mLength;
        },
        this // capture
    };

    IntSettingItem mMinOutpNote = {"Min note",
                                   NumericEditRangeSpec<int>{0, 127},
                                   Property<int>{
                                       [](void *cap) FLASHMEM {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           return (int)pThis->EditingVoice().mOutpRange.mMin.GetMidiValue();
                                       }, // getter
                                       [](void *cap, const int &val) {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           pThis->EditingVoice().mOutpRange.mMin = MidiNote{(uint8_t)val};
                                       },   // setter
                                       this // capture val
                                   },
                                   AlwaysEnabled};

    IntSettingItem mMaxOutpNote = {"Max note",
                                   NumericEditRangeSpec<int>{0, 127},
                                   Property<int>{
                                       [](void *cap) FLASHMEM {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           return (int)pThis->EditingVoice().mOutpRange.mMax.GetMidiValue();
                                       }, // getter
                                       [](void *cap, const int &val) {
                                           auto *pThis = (HarmVoiceSettingsApp *)cap;
                                           pThis->EditingVoice().mOutpRange.mMax = MidiNote{(uint8_t)val};
                                       },   // setter
                                       this // capture val
                                   },
                                   AlwaysEnabled};

    EnumSettingItem<NoteOOBBehavior> mOOBBehavior = {"OOB behav",
                                                     gNoteOOBBehaviorInfo,
                                                     Property<NoteOOBBehavior>{
                                                         [](void *cap) FLASHMEM {
                                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                             return pThis->EditingVoice().mNoteOOBBehavior.GetValue();
                                                         }, // getter
                                                         [](void *cap, const NoteOOBBehavior &val) {
                                                             auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                             pThis->EditingVoice().mNoteOOBBehavior.SetValue(val);
                                                         },   // setter
                                                         this // capture val
                                                     },
                                                     AlwaysEnabled};

    EnumSettingItem<HarmScaleRefType> mScaleRefType = {"Scale ref",
                                                       gHarmScaleRefTypeInfo,
                                                       Property<HarmScaleRefType>{
                                                           [](void *cap) FLASHMEM {
                                                               auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                               return pThis->EditingVoice().mScaleRef.GetValue();
                                                           }, // getter
                                                           [](void *cap, const HarmScaleRefType &val) {
                                                               auto *pThis = (HarmVoiceSettingsApp *)cap;
                                                               pThis->EditingVoice().mScaleRef.SetValue(val);
                                                           },   // setter
                                                           this // capture val
                                                       },
                                                       AlwaysEnabled};

    EnumSettingItem<Note> mLocalScaleNote = {
        "My Scale Root",
        gNoteInfo,
        Property<Note>{
            [](void *cap) FLASHMEM {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                return pThis->EditingVoice().mLocalScale.mValue.mRootNoteIndex;
            }, // getter
            [](void *cap, const Note &val) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                pThis->EditingVoice().mLocalScale.mValue.mRootNoteIndex = val;
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
            [](void *cap) FLASHMEM {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                return pThis->EditingVoice().mLocalScale.mValue.mFlavorIndex;
            }, // getter
            [](void *cap, const ScaleFlavorIndex &val) {
                auto *pThis = (HarmVoiceSettingsApp *)cap;
                pThis->EditingVoice().mLocalScale.mValue.mFlavorIndex = val;
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

    // EnumSettingItem<NonDiatonicBehavior> mNonDiatonicBehavior = {
    //     "OOS behav",
    //     gNonDiatonicBehaviorInfo,
    //     Property<NonDiatonicBehavior>{
    //         [](void *cap) FLASHMEM {
    //             auto *pThis = (HarmVoiceSettingsApp *)cap;
    //             return pThis->EditingVoice().mNonDiatonicBehavior;
    //         }, // getter
    //         [](void *cap, const NonDiatonicBehavior &val) {
    //             auto *pThis = (HarmVoiceSettingsApp *)cap;
    //             pThis->EditingVoice().mNonDiatonicBehavior = val;
    //         },   // setter
    //         this // capture val
    //     },
    //     AlwaysEnabled};

    // EnumSettingItem<PitchBendParticipation> mPitchbendBehav = {
    //     "PB behav",
    //     gPitchBendParticipationInfo,
    //     Property<PitchBendParticipation>{
    //         [](void *cap) FLASHMEM {
    //             auto *pThis = (HarmVoiceSettingsApp *)cap;
    //             return pThis->EditingVoice().mPitchBendParticipation;
    //         }, // getter
    //         [](void *cap, const PitchBendParticipation &val) {
    //             auto *pThis = (HarmVoiceSettingsApp *)cap;
    //             pThis->EditingVoice().mPitchBendParticipation = val;
    //         },   // setter
    //         this // capture val
    //     },
    //     AlwaysEnabled};

    ISettingItem *mArray[10] = {
        &mSynthPresetRef,
        &mOwnSynthPatch,
        &mSequenceLength,
        &mSequence,
        &mMinOutpNote,
        &mMaxOutpNote,
        &mOOBBehavior,
        //&mPitchbendBehav,
        &mScaleRefType,
        &mLocalScaleNote,
        &mLocalScaleFlavor,
        //&mNonDiatonicBehavior,
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

    HarmPatchSettingsApp(IDisplay &d, AppSettings &appSettings, InputDelegator &input)
        : SettingsMenuApp(d, appSettings, input), mDisplay(d), mHarmVoiceSettingsApp(d, appSettings, input)
    {
    }

    IDisplay &mDisplay;
    HarmVoiceSettingsApp mHarmVoiceSettingsApp;

    HarmPatch &EditingPreset()
    {
        auto &perf = this->mAppSettings->GetCurrentPerformancePatch();
        return mAppSettings->FindHarmPreset(perf.mHarmPreset.GetValue());
    }

    BoolSettingItem mEmitLiveNote = {"Emit live note?",
                                     "Yes",
                                     "No",
                                     Property<bool>{
                                         [](void *cap) FLASHMEM { // getter
                                             auto *pThis = (HarmPatchSettingsApp *)cap;
                                             return pThis->EditingPreset().mEmitLiveNote.GetValue();
                                         },
                                         [](void *cap, const bool &val) { // setter
                                             auto *pThis = (HarmPatchSettingsApp *)cap;
                                             pThis->EditingPreset().mEmitLiveNote.SetValue(val);
                                         },
                                         this // capture val
                                     },
                                     AlwaysEnabled};

    FloatSettingItem mStereoSeparation = {
        "Stereo Sep",
        StandardRangeSpecs::gFloat_0_1,
        Property<float>{[](void *cap) FLASHMEM {
                            auto *pThis = (HarmPatchSettingsApp *)cap;
                            return pThis->EditingPreset().mStereoSeparation.GetValue();
                        },
                        [](void *cap, const float &v) {
                            auto *pThis = (HarmPatchSettingsApp *)cap;
                            pThis->EditingPreset().mStereoSeparation.SetValue(v);
                        },
                        this},
        AlwaysEnabled};

    IntSettingItem mMinRotationTimeMS = {"Min Rotation MS",
                                         NumericEditRangeSpec<int>{0, 10000},
                                         Property<int>{
                                             [](void *cap) FLASHMEM {
                                                 auto *pThis = (HarmPatchSettingsApp *)cap;
                                                 return (int)pThis->EditingPreset().mMinRotationTimeMS.GetValue();
                                             }, // getter
                                             [](void *cap, const int &val) {
                                                 auto *pThis = (HarmPatchSettingsApp *)cap;
                                                 pThis->EditingPreset().mMinRotationTimeMS.SetValue(val);
                                             },   // setter
                                             this // capture val
                                         },
                                         AlwaysEnabled};

    IntSettingItem mSynthPreset1 = {"Synth preset 1",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) FLASHMEM {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPatch1.GetValue();
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPatch1.SetValue(val);
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mAppSettings->mSynthSettings.mPatches[n].ToString();
                                    },
                                    AlwaysEnabled,
                                    this};

    IntSettingItem mSynthPreset2 = {"Synth preset 2",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) FLASHMEM {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPatch2.GetValue();
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPatch1.SetValue(val);
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mAppSettings->mSynthSettings.mPatches[n].ToString();
                                    },
                                    AlwaysEnabled,
                                    this};

    IntSettingItem mSynthPreset3 = {"Synth preset 3",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) FLASHMEM {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPatch3.GetValue();
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPatch3.SetValue(val);
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mAppSettings->mSynthSettings.mPatches[n].ToString();
                                    },
                                    AlwaysEnabled,
                                    this};

    IntSettingItem mSynthPreset4 = {"Synth preset 4",
                                    NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                    Property<int>{
                                        [](void *cap) FLASHMEM {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            return (int)pThis->EditingPreset().mSynthPatch4.GetValue();
                                        }, // getter
                                        [](void *cap, const int &val) {
                                            auto *pThis = (HarmPatchSettingsApp *)cap;
                                            pThis->EditingPreset().mSynthPatch4.SetValue(val);
                                        },   // setter
                                        this // capture val
                                    },
                                    [](void *cap, int n) { // formatter
                                        auto *pThis = (HarmPatchSettingsApp *)cap;
                                        return pThis->mAppSettings->mSynthSettings.mPatches[n].ToString();
                                    },
                                    AlwaysEnabled,
                                    this};

    EnumSettingItem<Note> mPresetScaleNote = {"Scale Root",
                                              gNoteInfo,
                                              Property<Note>{
                                                  [](void *cap) FLASHMEM {
                                                      auto *pThis = (HarmPatchSettingsApp *)cap;
                                                      return pThis->EditingPreset().mPatchScale.mValue.mRootNoteIndex;
                                                  }, // getter
                                                  [](void *cap, const Note &val) {
                                                      auto *pThis = (HarmPatchSettingsApp *)cap;
                                                      pThis->EditingPreset().mPatchScale.mValue.mRootNoteIndex = val;
                                                  },   // setter
                                                  this // capture val
                                              },
                                              AlwaysEnabled};

    EnumSettingItem<ScaleFlavorIndex> mPresetScaleFlavor = {
        " ->Flav",
        gScaleFlavorIndexInfo,
        Property<ScaleFlavorIndex>{
            [](void *cap) FLASHMEM {
                auto *pThis = (HarmPatchSettingsApp *)cap;
                return pThis->EditingPreset().mPatchScale.mValue.mFlavorIndex;
            }, // getter
            [](void *cap, const ScaleFlavorIndex &val) {
                auto *pThis = (HarmPatchSettingsApp *)cap;
                pThis->EditingPreset().mPatchScale.mValue.mFlavorIndex = val;
            },   // setter
            this // capture val
        },
        AlwaysEnabled};

    MultiSubmenuSettingItem mVoiceSubmenu = {
        [](void *cap) FLASHMEM { return clarinoid::HARM_VOICES; },
        [](void *cap, size_t mi) { // name
            auto *pThis = (HarmPatchSettingsApp *)cap;
            // like
            // Voice 5 [-2,2]
            return (String)(String("Voice ") + mi + "" +
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
            return String(String("") + i + ":" + pThis->mAppSettings->mHarmSettings.mPatches[i].mName.GetValue());
        },
        [](void *cap,
           size_t i) { // cc::function<void(void*,size_t)>::ptr_t onClick,
            auto *pThis = (HarmPatchSettingsApp *)cap;
            pThis->mAppSettings->mHarmSettings.mPatches[i].CopyFrom(pThis->EditingPreset());
            auto &settings = *pThis->mAppSettings;
            auto &perf = settings.GetCurrentPerformancePatch();
            auto fromName = settings.GetHarmPatchName(perf.mHarmPreset.GetValue());
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
        mDisplay.println(String("Harmonizer"));
        mDisplay.println(String("     patch >>"));
        auto &perf = GetAppSettings()->GetCurrentPerformancePatch();
        auto name = GetAppSettings()->GetHarmPatchName(perf.mHarmPreset.GetValue());
        mDisplay.println(name);
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

    HarmSettingsApp(IDisplay &d, AppSettings &appSettings, InputDelegator &input)
        : SettingsMenuApp(d, appSettings, input), mHarmPatchSettings(d, appSettings, input)
    {
    }

    MultiSubmenuSettingItem mPatchSubmenu = {
        [](void *cap) FLASHMEM { return clarinoid::HARM_PRESET_COUNT; }, // get item count
        [](void *cap, size_t mi) {                                       // get item name
            auto *pThis = (HarmSettingsApp *)cap;
            return (String)(String("") + mi + ":" +
                            pThis->GetAppSettings()->mHarmSettings.mPatches[mi].mName.GetValue());
        },
        [](void *cap, size_t mi) { // get submenu
            auto *pThis = (HarmSettingsApp *)cap;
            pThis->GetAppSettings()->GetCurrentPerformancePatch().mHarmPreset.SetValue(mi);
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
        this->mDisplay.println("Harmonizer >");

        auto &perf = this->GetAppSettings()->GetCurrentPerformancePatch();
        auto name = this->GetAppSettings()->GetHarmPatchName(perf.mHarmPreset.GetValue());
        // auto& h = this->GetAppSettings()->FindHarmPreset(perf.mHarmPreset);

        this->mDisplay.println(name);
        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
