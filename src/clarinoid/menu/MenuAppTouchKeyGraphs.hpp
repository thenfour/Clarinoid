#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "Plotter.hpp"

class TouchKeyGraphs : public MenuAppBaseWithUtils
{
  int mKeyIndex = 0;
  Plotter<3, 2> mPlotter;
  CCThrottlerT<5> mThrottle;
  bool isPlaying = true;

  virtual void RenderFrontPage() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println(String("TOUCH KEYS STATE"));
    gDisplay.mDisplay.println(String("LH k:") +
      (gEWIControl.mPhysicalState.key_lh1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_lh2.IsCurrentlyPressed() ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_lh3.IsCurrentlyPressed() ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_lh4.IsCurrentlyPressed() ? "4" : "-") +
      " o:" +
      (gEWIControl.mPhysicalState.key_octave1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_octave2.IsCurrentlyPressed() ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_octave3.IsCurrentlyPressed() ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_octave4.IsCurrentlyPressed() ? "4" : "-") +
      " b:" +
      (gEWIControl.mPhysicalState.key_lhExtra1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_lhExtra2.IsCurrentlyPressed() ? "2" : "-"));

    gDisplay.mDisplay.println(String("RH k:") +
      (gEWIControl.mPhysicalState.key_rh1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_rh2.IsCurrentlyPressed() ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_rh3.IsCurrentlyPressed() ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_rh4.IsCurrentlyPressed() ? "4" : "-") +
      "       " +
      " b:" +
      (gEWIControl.mPhysicalState.key_rhExtra1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_rhExtra2.IsCurrentlyPressed() ? "2" : "-"));

  }
  virtual void RenderApp() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println(gKeyDesc[mKeyIndex].mName);
    mPlotter.Render();
  }
  virtual void UpdateApp() {
    if (BackButton().IsNewlyPressed()) {
      GoToFrontPage();
      return;
    }

    gTouchKeyGraphsIsRunning = true;
    
    if (gEncButton.IsNewlyPressed()) {
      isPlaying = !isPlaying;
    }

    if (mThrottle.IsReady() && isPlaying) {
      LHRHChecksummablePayload* pData = nullptr;
      if (mKeyIndex == gRHSerial.mReceivedData.data.focusedKey) {
        pData = &gRHSerial.mReceivedData.data;
      } else if (mKeyIndex == gLHSerial.mReceivedData.data.focusedKey) {
        pData = &gLHSerial.mReceivedData.data;
      }
      if (pData) {
        mPlotter.Plot3b(pData->focusedTouchReadMicros,
          pData->focusedTouchReadUntouchedMicros,
          pData->focusedTouchReadThresholdMicros,
          gEWIControl.mPhysicalState.mOrderedKeys[mKeyIndex]->IsCurrentlyPressed()
          );
      }
    }

    auto oldI = mKeyIndex;
    mKeyIndex = AddConstrained(mKeyIndex, gEnc.GetIntDelta(), 0, SizeofStaticArray(gKeyDesc) - 1);
    gApp.FocusKeyDebug(mKeyIndex);
    if (mKeyIndex != oldI) {
      mPlotter.clear();
    }
  }
};
