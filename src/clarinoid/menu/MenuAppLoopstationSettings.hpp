#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

namespace clarinoid
{

// struct LoopSettingsApp : public SettingsMenuApp
// {
//   EnumSettingItem<LooperTrigger> mTrigger = { "Trigger", gLooperTriggerTypeInfo,
//   gAppSettings.mLooperSettings.mTrigger, AlwaysEnabled }; TriggerSettingItem mClearAll = { "Clear All", [](void*){
//   gMusicalState.mLooper.Clear(); }, nullptr, AlwaysEnabled }; TriggerSettingItem mLoopIt = { "Loop it", [](void*){
//   gMusicalState.mLooper.LoopIt(gMusicalState.mLiveVoice); }, nullptr, AlwaysEnabled };

//   LabelSettingItem mStatus = { []() {return (String)(String("Status:") + (int)gMusicalState.mLooper.mStatus.mState);
//   }, AlwaysEnabled }; LabelSettingItem mLayer = { []() {return (String)(String("Layer:") +
//   gMusicalState.mLooper.mCurrentlyWritingLayer); }, AlwaysEnabled }; LabelSettingItem mLoopTime = { []() {return
//   (String)(String("LoopTimeMS:") + gMusicalState.mLooper.mStatus.mCurrentLoopTimeMS); }, [](){ return
//   gMusicalState.mLooper.mStatus.mState == LooperState::StartSet; } }; LabelSettingItem mLoopLen = { []() {return
//   (String)(String("LoopLengthMS:") + gMusicalState.mLooper.mStatus.mLoopDurationMS); }, [](){ return
//   gMusicalState.mLooper.mStatus.mState == LooperState::DurationSet; } };
//   //LabelSettingItem mLoopPolyphony = { []() {return String("LoopPoly :") + gMusicalState.mLooper.mCurrentPolyphony;
//   }, AlwaysEnabled }; LabelSettingItem mSynthPolyphony = { []() {return (String)(String("SynthPoly:") +
//   gSynth.mCurrentPolyphony); }, AlwaysEnabled };

//   LabelSettingItem mMemUsage = { []() { return (String)(String("Memusage ():") + GetMemUsagePercent() + "%"); },
//   AlwaysEnabled }; // memusage 32% LabelSettingItem mMemUsage2 = { []() { return (String)(String("") +
//   GetMemUsageKB() + "/" + GetTotalMemKB() + "kb"); }, AlwaysEnabled }; // 32/128kb LabelSettingItem mDuration = {
//   []() { return (String)(String("Rec len: ") + GetRecordedDurationSec() + "sec"); }, AlwaysEnabled };
//   LabelSettingItem mMaxDuration = { []() { return (String)(String("Est max len: ") + GetMaxRecordedDurationMin() +
//   "min"); }, AlwaysEnabled };

//   LabelSettingItem mLayer1Info = { []() { return (String)(String("Layer1: ") + GetMemUsageBytes(0) + " bytes"); },
//   AlwaysEnabled }; LabelSettingItem mLayer1Status = { []() { return (String)(String("   Status: ") +
//   GetLayerStatus(0)); }, AlwaysEnabled }; TriggerSettingItem mLayer1Clear = { " > Clear", [](void*){
//   gMusicalState.mLooper.ClearLayer(0); }, nullptr, AlwaysEnabled };

//   LabelSettingItem mLayer2Info = { []() { return (String)(String("Layer2: ") + GetMemUsageBytes(1) + " bytes"); },
//   AlwaysEnabled }; LabelSettingItem mLayer2Status = { []() { return (String)(String("   Status: ") +
//   GetLayerStatus(1)); }, AlwaysEnabled }; TriggerSettingItem mLayer2Clear = { " > Clear", [](void*){
//   gMusicalState.mLooper.ClearLayer(1); }, nullptr, AlwaysEnabled };

//   LabelSettingItem mLayer3Info = { []() { return (String)(String("Layer3: ") + GetMemUsageBytes(2) + " bytes"); },
//   AlwaysEnabled }; LabelSettingItem mLayer3Status = { []() { return (String)(String("   Status: ") +
//   GetLayerStatus(2)); }, AlwaysEnabled }; TriggerSettingItem mLayer3Clear = { " > Clear", [](void*){
//   gMusicalState.mLooper.ClearLayer(2); }, nullptr, AlwaysEnabled };

//   static float GetMemUsagePercent() {
//     return (float)GetMemUsageKB() * 100.0f / GetTotalMemKB();
//   }

//   static const char *GetLayerStatus(size_t layer)
//   {
//     return gMusicalState.mLooper.mLayers[layer].GetStateString();
//   }

//   static int GetMemUsageBytes(size_t layer)
//   {
//     int ret = 0;
//     for (auto& l : gMusicalState.mLooper.mLayers) {
//       ret += l.GetMemoryUsage();
//     }
//     return ret / 1024;
//   }

//   static int GetMemUsageKB()
//   {
//     int ret = 0;
//     for (auto& l : gMusicalState.mLooper.mLayers) {
//       ret += l.GetMemoryUsage();
//     }
//     return ret / 1024;
//   }

//   static int GetTotalMemKB()
//   {
//     return LOOPER_MEMORY_TOTAL_BYTES / 1024;
//   }

//   static float GetRecordedDurationSec()
//   {
//     size_t playingLayers = 0;
//     for (auto& l : gMusicalState.mLooper.mLayers) {
//       if (l.mIsPlaying) {
//         ++playingLayers;
//       }
//     }
//     return (float)playingLayers * gMusicalState.mLooper.mStatus.mLoopDurationMS / 1000.0f;
//   }

//   static float GetMaxRecordedDurationMin()
//   {
//     float memUsed = (float)GetMemUsageKB();
//     float memMax = (float)GetTotalMemKB();
//     if (memUsed < 0.01) {
//       return 0.0f;
//     }
//     float multi = memMax / memUsed; // how many times can we fit used into max
//     return multi * GetRecordedDurationSec() / 60.0f;
//   }

//   ISettingItem* mArray[21] =
//   {
//     &mTrigger, &mClearAll, &mLoopIt, &mStatus, &mLayer, &mLoopTime, &mLoopLen, &mSynthPolyphony,
//     &mMemUsage,
//     &mMemUsage2,
//     &mDuration,
//     &mMaxDuration,
//     &mLayer1Info, &mLayer1Status, &mLayer1Clear,
//     &mLayer2Info, &mLayer2Status, &mLayer2Clear,
//     &mLayer3Info, &mLayer3Status, &mLayer3Clear,
//   };
//   SettingsList mRootList = { mArray };

//   virtual SettingsList* GetRootSettingsList()
//   {
//     return &mRootList;
//   }

//   virtual void RenderFrontPage()
//   {
//     gDisplay.mDisplay.setTextSize(1);
//     gDisplay.mDisplay.setTextColor(WHITE);
//     gDisplay.mDisplay.setCursor(0,0);
//     gDisplay.mDisplay.println("Looper settings");
//     gDisplay.mDisplay.println(String("Scale: "));
//     gDisplay.mDisplay.println(String("Preset: "));
//     gDisplay.mDisplay.println(String("                  -->"));

//     SettingsMenuApp::RenderFrontPage();
//   }

// };

} // namespace clarinoid
