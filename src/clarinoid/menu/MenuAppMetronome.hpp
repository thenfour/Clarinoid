#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"


struct MetronomeSettingsApp : public SettingsMenuApp
{
  BoolSettingItem mOnOff = { "Enabled?", "On", "Off", gAppSettings.mMetronomeOn, AlwaysEnabled };
  FloatSettingItem mBPM = { "BPM", 30.0f, 200.0f, gAppSettings.mBPM, AlwaysEnabled };
  FloatSettingItem mGain = { "Gain", 0.0f, 1.0f, gAppSettings.mMetronomeGain, [](){ return gAppSettings.mMetronomeOn; } };
  IntSettingItem mNote = { "Note", 20, 120, gAppSettings.mMetronomeNote, [](){ return gAppSettings.mMetronomeOn; } };
  IntSettingItem mDecay = { "Decay", 1, 120, gAppSettings.mMetronomeDecayMS, [](){ return gAppSettings.mMetronomeOn; } };

  ISettingItem* mArray[5] =
  {
    &mOnOff, &mBPM, &mGain, &mNote, &mDecay
  };
  SettingsList mRootList = { mArray };


  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    float beatFloat = gMetronome.GetBeatFloat();// gSynthGraphControl.mMetronomeTimer.GetBeatFloat(60000.0f / gAppSettings.mBPM);
    float beatFrac = beatFloat - floor(beatFloat);
    int beatInt = (int)floor(beatFloat);
    //CCPlot(beatInt);
    bool altBeat = (beatInt & 1) != 0;

    bool highlight = beatFrac < 0.1;
    gDisplay.mDisplay.setTextSize(1);
    if (highlight) {
      gDisplay.mDisplay.fillScreen(WHITE);
    }
    gDisplay.mDisplay.setTextColor(highlight ? BLACK : WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    gDisplay.mDisplay.println(String("METRONOME SETTINGS"));
    gDisplay.mDisplay.print(gAppSettings.mMetronomeOn ? "ENABLED" : "disabled");
    gDisplay.mDisplay.println(String(" bpm=") + gAppSettings.mBPM);

    const int r = 4;
    int x = beatFrac * (RESOLUTION_X - r*2);
    if (altBeat)
      x = gDisplay.mDisplay.width() - x;
    gDisplay.mDisplay.fillCircle(x, gDisplay.mDisplay.getCursorY() + r, r, highlight ? BLACK : WHITE);
    
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

  virtual ISettingItemEditor* GetBackEditor() {
    return mBPM.GetEditor(0);
  }
};
