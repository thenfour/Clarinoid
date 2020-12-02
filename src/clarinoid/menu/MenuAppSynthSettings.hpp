#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

namespace clarinoid
{

// size_t gEditingSynthPatch = 0;

// struct SynthPatchMenuApp
// {
//   static SynthPreset& GetBinding() { return gAppSettings.mSynthSettings.mPresets[gEditingSynthPatch]; }

//   // NumericSettingItem(const String& name, T min_, T max_, const Property<T>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
//   // BoolSettingItem(const String& name, const String& trueCaption, const String& falseCaption, const Property<bool>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
//   FloatSettingItem mDetune = { "mDetune", 0.0f, 1.5f,
//     Property<float>{ [](void*){return GetBinding().mDetune; }, [](void*, const float& v) { GetBinding().mDetune = v; }, this },
//     AlwaysEnabled };
//   FloatSettingItem mPortamentoTime = { "mPortamentoTime", 0.0f, 0.2f, Property<float>{ [](void*){return GetBinding().mPortamentoTime; }, [](void*, const float& v) { GetBinding().mPortamentoTime = v; }, this }, AlwaysEnabled };
//   BoolSettingItem mSync = { "Sync", "On", "Off",
//     Property<bool>{[](void*){return GetBinding().mSync; }, [](void*, const bool& v) { GetBinding().mSync = v; }, this},
//     AlwaysEnabled };

//   // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding, cc::function<bool()>::ptr_t isEnabled) :
//   EnumSettingItem<OscWaveformShape> mOsc1Waveform = { "mOsc1WaveformType", gOscWaveformShapeInfo,
//     Property<OscWaveformShape>{ [](void*){return GetBinding().mOsc1Waveform; }, [](void*, const OscWaveformShape& v) { GetBinding().mOsc1Waveform = v; }, this },
//     AlwaysEnabled};
//   FloatSettingItem mOsc1Gain = { "Osc1Gain", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc1Gain; }, [](void*, const float& v) { GetBinding().mOsc1Gain = v; }, this },
//     AlwaysEnabled };
//   IntSettingItem mOsc1PitchSemis = { "mOsc1PitchSemis", -24, 24,
//     Property<int>{ [](void*){return GetBinding().mOsc1PitchSemis; }, [](void*, const int& v) { GetBinding().mOsc1PitchSemis = v; }, this },
//     AlwaysEnabled };
//   FloatSettingItem mOsc1PitchFine = { "mOsc1PitchFine", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc1PitchFine; }, [](void*, const float& v) { GetBinding().mOsc1PitchFine = v; }, this },
//     AlwaysEnabled };
//   FloatSettingItem mOsc1PulseWidth = { "mOsc1PulseWidth", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc1PulseWidth; }, [](void*, const float& v) { GetBinding().mOsc1PulseWidth = v; }, this },
//     AlwaysEnabled };


//   EnumSettingItem<OscWaveformShape> mOsc2Waveform = { "mOsc2WaveformType", gOscWaveformShapeInfo,
//     Property<OscWaveformShape>{ [](void*){return GetBinding().mOsc2Waveform; }, [](void*, const OscWaveformShape& v) { GetBinding().mOsc2Waveform = v; }, this },
//     AlwaysEnabled};
//   FloatSettingItem mOsc2Gain = { "Osc2Gain", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc2Gain; }, [](void*, const float& v) { GetBinding().mOsc2Gain = v; }, this },
//     AlwaysEnabled };
//   IntSettingItem mOsc2PitchSemis = { "mOsc2PitchSemis", -24, 24,
//     Property<int>{ [](void*){return GetBinding().mOsc2PitchSemis; }, [](void*, const int& v) { GetBinding().mOsc2PitchSemis = v; }, this },
//     AlwaysEnabled };
//   FloatSettingItem mOsc2PitchFine = { "mOsc2PitchFine", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc2PitchFine; }, [](void*, const float& v) { GetBinding().mOsc2PitchFine = v; }, this },
//     AlwaysEnabled };
//   FloatSettingItem mOsc2PulseWidth = { "mOsc2PulseWidth", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc2PulseWidth; }, [](void*, const float& v) { GetBinding().mOsc2PulseWidth = v; }, this },
//     AlwaysEnabled };


//   EnumSettingItem<OscWaveformShape> mOsc3Waveform = { "mOsc3Waveform", gOscWaveformShapeInfo,
//     Property<OscWaveformShape>{ [](void*){return GetBinding().mOsc3Waveform; }, [](void*, const OscWaveformShape& v) { GetBinding().mOsc3Waveform = v; }, this },
//     AlwaysEnabled};
//   FloatSettingItem mOsc3Gain = { "mOsc3Gain", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc3Gain; }, [](void*, const float& v) { GetBinding().mOsc3Gain = v; }, this },
//     AlwaysEnabled };
//   IntSettingItem mOsc3PitchSemis = { "mOsc3PitchSemis", -24, 24,
//     Property<int>{ [](void*){return GetBinding().mOsc3PitchSemis; }, [](void*, const int& v) { GetBinding().mOsc3PitchSemis = v; }, this },
//     AlwaysEnabled };
//   FloatSettingItem mOsc3PitchFine = { "mOsc3PitchFine", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc3PitchFine; }, [](void*, const float& v) { GetBinding().mOsc3PitchFine = v; }, this },
//     AlwaysEnabled };
//   FloatSettingItem mOsc3PulseWidth = { "mOsc3PulseWidth", 0.0f, 1.0f,
//     Property<float>{ [](void*){return GetBinding().mOsc3PulseWidth; }, [](void*, const float& v) { GetBinding().mOsc3PulseWidth = v; }, this },
//     AlwaysEnabled };



//   ISettingItem* mArray[18] =
//   {
//     &mDetune, &mPortamentoTime, &mSync,
//     &mOsc1Waveform, &mOsc1Gain, &mOsc1PitchSemis, &mOsc1PitchFine, &mOsc1PulseWidth,
//     &mOsc2Waveform, &mOsc2Gain, &mOsc2PitchSemis, &mOsc2PitchFine, &mOsc2PulseWidth,
//     &mOsc3Waveform, &mOsc3Gain, &mOsc3PitchSemis, &mOsc3PitchFine, &mOsc3PulseWidth,
//   };
//   SettingsList mRootList = { mArray };


//   SettingsList* Start(size_t iPatch)
//   {
//     gEditingSynthPatch = iPatch;
//     return &mRootList;
//   }
// };

// SynthPatchMenuApp gSynthPatchSettingsApp;

// class SynthSettingsApp : public SettingsMenuApp
// {
//   IntSettingItem mGlobalSynthPreset = { "Global synth P#", 0, SYNTH_PRESET_COUNT - 1, MakePropertyByCasting<int>(&gAppSettings.mGlobalSynthPreset), AlwaysEnabled };
//   IntSettingItem mGlobalHarmPreset = { "Global harm P#", 0, HARM_PRESET_COUNT - 1, MakePropertyByCasting<int>(&gAppSettings.mGlobalHarmPreset), AlwaysEnabled };
//   IntSettingItem mTranspose = { "Transpose", -48, 48, gAppSettings.mTranspose, AlwaysEnabled };
//   FloatSettingItem mReverbGain = { "Reverb gain", 0.0f, 1.0f, gAppSettings.mReverbGain, AlwaysEnabled };
//   MultiSubmenuSettingItem mPatches = { SYNTH_PRESET_COUNT,
//     [](void*, size_t n) {
//       if (gAppSettings.mSynthSettings.mPresets[n].mName.length() == 0) {
//         return String(String("Preset ") + n);
//       }
//       return gAppSettings.mSynthSettings.mPresets[n].mName;
//     }, // name
//     [](void*, size_t n) { return gSynthPatchSettingsApp.Start(n); }, // get submenu list
//     [](void*, size_t n) { return true; },
//     (void*)this // capture
//     };

//   ISettingItem* mArray[5] =
//   {
//     &mGlobalSynthPreset, &mGlobalHarmPreset,
//     &mTranspose, &mReverbGain, &mPatches
//   };
//   SettingsList mRootList = { mArray };

// public:
//   virtual SettingsList* GetRootSettingsList()
//   {
//     return &mRootList;
//   }

//   virtual void RenderFrontPage() 
//   {
//     gDisplay.mDisplay.setTextSize(1);
//     gDisplay.mDisplay.setTextColor(WHITE);
//     gDisplay.mDisplay.setCursor(0,0);

//     gDisplay.mDisplay.println(String("SYNTH SETTINGS"));
//     gDisplay.mDisplay.println(String(""));
//     gDisplay.mDisplay.println(String(""));
//     gDisplay.mDisplay.println(String("                  -->"));

//     SettingsMenuApp::RenderFrontPage();
//   }
// };

} // namespace clarinoid
