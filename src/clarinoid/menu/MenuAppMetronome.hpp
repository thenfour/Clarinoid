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
    virtual const char *DisplayAppGetName() override { return "MetronomeSettingsApp"; }

    Metronome* mpMetronome;
    AppSettings* mpAppSettings;

    MetronomeSettingsApp(Metronome* pm, AppSettings* pas, CCDisplay &d) :
        SettingsMenuApp(d),
        mpMetronome(pm),
        mpAppSettings(pas)
    {
    }

    Property<bool> EnabledIfSoundOn = Property<bool> {
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeSoundOn; },
                                        this };

    Property<bool> EnabledIfLEDOn = Property<bool> {
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeLED; },
                                        this };

    FloatSettingItem mBPM = { "BPM", StandardRangeSpecs::gBPMRange, Property<float>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mBPM; },
                                        [](void *cap, const float &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mBPM = v; },
                                        this },
                                        AlwaysEnabled };

    BoolSettingItem mSoundEnable = { "SoundEnable", "On", "Off", Property<bool>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeSoundOn; },
                                        [](void *cap, const bool &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mMetronomeSoundOn = v; },
                                        this },
                                        AlwaysEnabled };

    BoolSettingItem mLEDEnable = { "LED enable", "On", "Off", Property<bool>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeLED; },
                                        [](void *cap, const bool &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mMetronomeLED = v; },
                                        this },
                                        AlwaysEnabled };

    FloatSettingItem mLEDDecay = { "LED decay", StandardRangeSpecs::gFloat_0_1, Property<float>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeLEDDecay; },
                                        [](void *cap, const float &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mMetronomeLEDDecay = v; },
                                        this },
                                        AlwaysEnabled };

    IntSettingItem mLEDBrightness = { "LED brightness", NumericEditRangeSpec<int> { 1, 255 }, Property<int>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeBrightness; },
                                        [](void *cap, const int &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mMetronomeBrightness = v; },
                                        this },
                                        AlwaysEnabled };

    FloatSettingItem mGain = { "Gain", StandardRangeSpecs::gFloat_0_1, Property<float>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeGain; },
                                        [](void *cap, const float &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mMetronomeGain = v; },
                                        this },
                                        EnabledIfSoundOn};

    IntSettingItem mNote = { "Note", StandardRangeSpecs::gMetronomeNoteRange, Property<int>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeNote; },
                                        [](void *cap, const int &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mMetronomeNote = v; },
                                        this },
                                        EnabledIfSoundOn };

    IntSettingItem mDecay = { "Decay", StandardRangeSpecs::gMetronomeDecayRange, Property<int>{
                                        [](void *cap) { auto* pThis = (MetronomeSettingsApp*)cap; return pThis->mpAppSettings->mMetronomeDecayMS; },
                                        [](void *cap, const int &v) { auto* pThis = (MetronomeSettingsApp*)cap; pThis->mpAppSettings->mMetronomeDecayMS = v; },
                                        this },
                                        EnabledIfSoundOn };

    ISettingItem* mArray[8] =
    {
        &mBPM, &mLEDEnable, &mLEDDecay, &mLEDBrightness, &mSoundEnable, &mGain, &mNote, &mDecay
    };
    SettingsList mRootList = { mArray };


  virtual SettingsList* GetRootSettingsList()
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
    //mDisplay.mDisplay.setTextSize(1);
    //mDisplay.mDisplay.setTextColor(WHITE);
    if (highlight) {
      mDisplay.mDisplay.fillScreen(WHITE);
    }
    mDisplay.mDisplay.setTextColor(highlight ? BLACK : WHITE);
    //mDisplay.mDisplay.setCursor(0,0);

    mDisplay.mDisplay.println(String("METRONOME SETTINGS"));
    mDisplay.mDisplay.print(mpAppSettings->mMetronomeSoundOn ? "SoundOn" : "SoundOff");
    mDisplay.mDisplay.print(" ");
    mDisplay.mDisplay.println(mpAppSettings->mMetronomeSoundOn ? "LEDOn" : "LEDOff");
    mDisplay.mDisplay.println(String(" bpm=") + mpAppSettings->mBPM);

    const int r = 4;
    int x = beatFrac * (MAX_DISPLAY_WIDTH - r*2);
    if (altBeat)
      x = mDisplay.mDisplay.width() - x;
    mDisplay.mDisplay.fillCircle(x, mDisplay.mDisplay.getCursorY() + r, r, highlight ? BLACK : WHITE);
    
    mDisplay.mDisplay.println(String(""));
    mDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

  // virtual ISettingItemEditor* GetBackEditor() {
  //   return mBPM.GetEditor(0);
  // }
};

} // namespace clarinoid
