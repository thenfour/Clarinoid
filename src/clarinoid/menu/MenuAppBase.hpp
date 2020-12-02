#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/application/Display.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

/////////////////////////////////////////////////////////////////////////////////////////////////
// provides utils for display apps.
// the system works like this: each app has a front page which you scroll through using the encoder. rendered using RenderFrontpage().
// clicking the encoder will enter the app, rendered with Render().
struct DisplayApp : 
  IDisplayApp
{
  bool mShowingFrontPage = true;

  CCDisplay& mDisplay;
  AppSettings* mAppSettings;
  IControlMapper* mControlMapper;

  SwitchControlReader mOK;
  SwitchControlReader mBack;
  EncoderReader mEnc;

  DisplayApp(CCDisplay& d) :
    mDisplay(d)
  {}

  virtual void DisplayAppInit() override {
    mAppSettings = mDisplay.mAppSettings;
    mControlMapper = mDisplay.mControlMapper;
    mOK.SetSource(mControlMapper->MenuOK());
    mBack.SetSource(mControlMapper->MenuBack());
    mEnc.SetSource(mControlMapper->MenuEncoder());
  }

  virtual void UpdateApp() = 0;
  virtual void RenderApp() = 0;
  virtual void RenderFrontPage() = 0;

  bool IsShowingFrontPage() const { return mShowingFrontPage; }

  virtual void DisplayAppOnSelected() override {
    //Serial.println(String("clearing state for appobj ") + ((uintptr_t)this) + " delta=" + mEnc.GetIntDelta());
    mEnc.ClearState(); // Necessary because we haven't been tracking state while our app is not selected. So if you select this app your encoder will read a huge delta and switch immediately to some other app.
    //Serial.println(String("cleared state ; delta=") + mEnc.GetIntDelta());
    GoToFrontPage();
  }

  void GoToFrontPage() {
    mShowingFrontPage = true;
  }

  virtual void DisplayAppRender() {
    if (!mShowingFrontPage) {
      this->RenderApp();
      return;
    }

    this->RenderFrontPage();
  }

  virtual void DisplayAppUpdate()
  {
    //Serial.println(String("") + micros() + " DisplayApp Update [" + this->DisplayAppGetName() + "]");
    mOK.Update();
    mBack.Update();
    mEnc.Update();

    //=Serial.println(String("OK for appobj ") + ((uintptr_t)this) + " delta: " + mEnc.GetIntDelta() + ", val=" + mEnc.GetIntValue());

    if (!mShowingFrontPage) {
      this->UpdateApp();
      return;
    }

    mDisplay.ScrollApps(mEnc.GetIntDelta());

    if (mOK.IsNewlyPressed()) {
      mShowingFrontPage = false;
    }
  }
};

} // namespace clarinoid
