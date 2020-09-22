#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"


class SynthSettingsApp : public SettingsMenuApp
{
  IntSettingItem mGlobalSynthPreset = { "Global synth preset", 0, SYNTH_PRESET_COUNT - 1, MakePropertyByCasting<int>(gAppSettings.mGlobalSynthPreset), AlwaysEnabled };
  //FloatSettingItem mPortamentoTime = { "Portamento", 0.0f, 0.25f, gAppSettings.mPortamentoTime, AlwaysEnabled };
  IntSettingItem mTranspose = { "Transpose", -48, 48, gAppSettings.mTranspose, AlwaysEnabled };
  FloatSettingItem mReverbGain = { "Reverb gain", 0.0f, 1.0f, gAppSettings.mReverbGain, AlwaysEnabled };
  
  ISettingItem* mArray[3] =
  {
    &mGlobalSynthPreset, &mTranspose, &mReverbGain
  };
  SettingsList mRootList = { mArray };

public:
  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    gDisplay.mDisplay.println(String("SYNTH SETTINGS"));
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }
};
