#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/components/Display.hpp>
#include <clarinoid/settings/AppSettings.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////
// provides utils for menu apps.
// the system works like this: each app has a front page which you scroll through using the encoder. rendered using RenderFrontpage().
// clicking the encoder will enter the app, rendered with Render().
class MenuAppBaseWithUtils : public MenuAppBase
{
  bool mShowingFrontPage = true;  
public:
  virtual void UpdateApp() = 0;
  virtual void RenderApp() = 0;
  virtual void RenderFrontPage() = 0;

  bool IsShowingFrontPage() const { return mShowingFrontPage; }

  ICCSwitch& BackButton() const { return gEWIControl.mPhysicalState.key_back; }
  
  virtual void OnSelected() {
    GoToFrontPage();
  }

  void GoToFrontPage() {
    mShowingFrontPage = true;
  }

  int EncoderIntDelta() {
    return gEnc.GetIntDelta();
  }

  virtual void Render() {
    if (!mShowingFrontPage) {
      this->RenderApp();
      return;
    }

    this->RenderFrontPage();
  }

  virtual void Update()
  {
    if (!mShowingFrontPage) {
      this->UpdateApp();
      return;
    }

    gDisplay.ScrollApps(gEnc.GetIntDelta());

    if (gEncButton.IsNewlyPressed()) {
      mShowingFrontPage = false;
    }
  }
};

