#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"




struct LoopSettingsApp : public SettingsMenuApp
{
  EnumSettingItem<LooperTrigger> mTrigger = { "Trigger", gLooperTriggerTypeInfo, gAppSettings.mLooperSettings.mTrigger, AlwaysEnabled };
  TriggerSettingItem mClearAll = { "Clear All", [](){ gEWIControl.mMusicalState.mLooper.Clear(); }, AlwaysEnabled };
  TriggerSettingItem mLoopIt = { "Loop it", [](){ gEWIControl.mMusicalState.mLooper.LoopIt(gEWIControl.mMusicalState.mLiveVoice); }, AlwaysEnabled };

  LabelSettingItem mStatus = { []() {return String("Status:") + (int)gEWIControl.mMusicalState.mLooper.mStatus.mState; }, AlwaysEnabled };
  LabelSettingItem mLayer = { []() {return String("Layer:") + gEWIControl.mMusicalState.mLooper.mCurrentlyWritingLayer; }, AlwaysEnabled };
  LabelSettingItem mLoopTime = { []() {return String("LoopTimeMS:") + gEWIControl.mMusicalState.mLooper.mStatus.mCurrentLoopTimeMS; }, [](){ return gEWIControl.mMusicalState.mLooper.mStatus.mState == LooperState::StartSet; } };
  LabelSettingItem mLoopLen = { []() {return String("LoopLengthMS:") + gEWIControl.mMusicalState.mLooper.mStatus.mLoopDurationMS; }, [](){ return gEWIControl.mMusicalState.mLooper.mStatus.mState == LooperState::DurationSet; } };
  //LabelSettingItem mLoopPolyphony = { []() {return String("LoopPoly :") + gEWIControl.mMusicalState.mLooper.mCurrentPolyphony; }, AlwaysEnabled };
  LabelSettingItem mSynthPolyphony = { []() {return String("SynthPoly:") + gSynth.mCurrentPolyphony; }, AlwaysEnabled };

  LabelSettingItem mMemUsage = { []() { return String("Memusage ():") + GetMemUsagePercent() + "%"; }, AlwaysEnabled }; // memusage 32%
  LabelSettingItem mMemUsage2 = { []() { return String("") + GetMemUsageKB() + "/" + GetTotalMemKB() + "kb"; }, AlwaysEnabled }; // 32/128kb
  LabelSettingItem mDuration = { []() { return String("Rec len: ") + GetRecordedDurationSec() + "sec"; }, AlwaysEnabled };
  LabelSettingItem mMaxDuration = { []() { return String("Est max len: ") + GetMaxRecordedDurationMin() + "min"; }, AlwaysEnabled };

  static float GetMemUsagePercent() {
    return (float)GetMemUsageKB() * 100.0f / GetTotalMemKB();
  }

  static int GetMemUsageKB()
  {
    int ret = 0;
    for (auto& l : gEWIControl.mMusicalState.mLooper.mLayers) {
      ret += l.GetMemoryUsage();
    }
    return ret / 1024;
  }

  static int GetTotalMemKB()
  {
    return LOOPER_MEMORY_TOTAL_BYTES / 1024;
  }

  static float GetRecordedDurationSec()
  {
    size_t playingLayers = 0;
    for (auto& l : gEWIControl.mMusicalState.mLooper.mLayers) {
      if (l.mIsPlaying) {
        ++playingLayers;
      }
    }
    return (float)playingLayers * gEWIControl.mMusicalState.mLooper.mStatus.mLoopDurationMS / 1000.0f;
  }

  static float GetMaxRecordedDurationMin()
  {
    float memUsed = (float)GetMemUsageKB();
    float memMax = (float)GetTotalMemKB();
    if (memUsed < 0.01) {
      return 0.0f;
    }
    float multi = memMax / memUsed; // how many times can we fit used into max
    return multi * GetRecordedDurationSec() / 60.0f;
  }

  ISettingItem* mArray[12] =
  {
    &mTrigger, &mClearAll, &mLoopIt, &mStatus, &mLayer, &mLoopTime, &mLoopLen, &mSynthPolyphony,
    &mMemUsage,
    &mMemUsage2,
    &mDuration,
    &mMaxDuration
  };
  SettingsList mRootList = { mArray };

  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println(String("Looper [") + (gAppSettings.mHarmSettings.mIsEnabled ? "on" : "off") + "]");
    gDisplay.mDisplay.println(String("Scale: "));
    gDisplay.mDisplay.println(String("Preset: "));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

};

