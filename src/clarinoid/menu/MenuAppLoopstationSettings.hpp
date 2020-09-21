#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"




struct LoopSettingsApp : public SettingsMenuApp
{
  TriggerSettingItem mClearAll = { "Clear All", [](){ gEWIControl.mMusicalState.mLooper.Clear(); }, AlwaysEnabled };
  TriggerSettingItem mLoopIt = { "Loop it", [](){ gEWIControl.mMusicalState.mLooper.LoopIt(gEWIControl.mMusicalState.mLiveVoice); }, AlwaysEnabled };

  LabelSettingItem mStatus = { []() {return String("Status:") + (int)gEWIControl.mMusicalState.mLooper.mStatus.mState; }, AlwaysEnabled };
  LabelSettingItem mLayer = { []() {return String("Layer:") + gEWIControl.mMusicalState.mLooper.mCurrentlyWritingLayer; }, AlwaysEnabled };
  LabelSettingItem mLoopTime = { []() {return String("LoopTimeMS:") + gEWIControl.mMusicalState.mLooper.mStatus.mCurrentLoopTimeMS; }, [](){ return gEWIControl.mMusicalState.mLooper.mStatus.mState == LooperState::StartSet; } };
  LabelSettingItem mLoopLen = { []() {return String("LoopLengthMS:") + gEWIControl.mMusicalState.mLooper.mStatus.mLoopDurationMS; }, [](){ return gEWIControl.mMusicalState.mLooper.mStatus.mState == LooperState::DurationSet; } };
  //LabelSettingItem mLoopPolyphony = { []() {return String("LoopPoly :") + gEWIControl.mMusicalState.mLooper.mCurrentPolyphony; }, AlwaysEnabled };
  LabelSettingItem mSynthPolyphony = { []() {return String("SynthPoly:") + gSynth.mCurrentPolyphony; }, AlwaysEnabled };

  ISettingItem* mArray[7] =
  {
    &mClearAll, &mLoopIt, &mStatus, &mLayer, &mLoopTime, &mLoopLen, &mSynthPolyphony
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

