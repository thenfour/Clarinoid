#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"



/////////////////////////////////////////////////////////////////////////////////////////////////
class DebugMenuApp : public MenuAppBaseWithUtils
{
public:
  virtual void RenderFrontPage() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println("DEBUG PAGES");
    gDisplay.mDisplay.println(String("Uptime: ") + (int)((float)millis() / 1000) + " sec");
    gDisplay.mDisplay.println(String("Notes played: ") + gEWIControl.mMusicalState.noteOns);
    gDisplay.mDisplay.println(String("                  -->"));
  }

  int mPage = 0;
  const int pageCount = 7;

  virtual void UpdateApp()
  {
    if (BackButton().IsNewlyPressed()) {
      GoToFrontPage();
      return;
    }
    mPage = AddConstrained(mPage, gEnc.GetIntDelta(), 0, pageCount - 1);
    if (mPage == 0 || mPage == 1) {
      if (gEncButton.IsNewlyPressed()) {
        gLHSerial.mRxSuccess = 0;
        gLHSerial.mChecksumErrors = 0;
        gLHSerial.mSkippedPayloads = 0;
        gRHSerial.mRxSuccess = 0;
        gRHSerial.mChecksumErrors = 0;
        gRHSerial.mSkippedPayloads = 0;
      }
    }
  }
  
  static void pageRX(CCMainTxRx& rx, const char *title)
  {
      //CCMainTxRx& rx = (gEncButton.IsPressed()) ? gLHSerial : gRHSerial;
      gDisplay.mDisplay.println(String(title) + " ok:" + rx.mRxSuccess + " #" + rx.mReceivedData.serial);
      gDisplay.mDisplay.print(String("Err:") + rx.mChecksumErrors);
      gDisplay.mDisplay.println(String(" Skip: ") + rx.mSkippedPayloads);
      gDisplay.mDisplay.println(String("fps:") + (int)rx.mReceivedData.framerate + " skip%:" + (int)((float)rx.mSkippedPayloads * 100 / max(1,(int)rx.mRxSuccess)));
      gDisplay.mDisplay.println(String("rxfps:") + (int)rx.mRxRate.getFPS());
  };

  virtual void RenderApp()
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    auto pageLHRX = [](){
      pageRX(gLHSerial, "LH");
    };
    
    auto pageRHRX = [](){
      pageRX(gRHSerial, "RH");
    };
    
    auto pageMusicalState = []() {
      gDisplay.mDisplay.println(String("#:") + gEWIControl.mMusicalState.mLiveVoice.mMidiNote + " (" + (gEWIControl.mMusicalState.mLiveVoice.IsPlaying() ? "ON" : "off" ) + ") " + (int)MIDINoteToFreq(gEWIControl.mMusicalState.mLiveVoice.mMidiNote) + "hz");
      gDisplay.mDisplay.println(String("transpose:") + gAppSettings.mTranspose);
      gDisplay.mDisplay.println(String("breath:") + gEWIControl.mMusicalState.breath01.GetValue());
      gDisplay.mDisplay.print(String("pitch:") + gEWIControl.mMusicalState.pitchBendN11.GetValue());
    };
    
    auto pagePhysicalState = []() {
      // LH: k:1234 o:1234 b:12
      // RH: k:1234 b:12
      // wind:0.455 // bite:0.11
      // pitch:
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
      gDisplay.mDisplay.print(String("breath: ") + gEWIControl.mPhysicalState.breath01 + "  " +
        "bite: " + gEWIControl.mPhysicalState.bite01);
      gDisplay.mDisplay.print(String("  pitch: ") + gEWIControl.mPhysicalState.pitchDown01);
      gDisplay.mDisplay.print(String("  tristate: ") + ToString(gEWIControl.mPhysicalState.key_triState));
    };

    auto pageDebugMain = []() {
      gDisplay.mDisplay.println(String("Max frame ms: ") + ((float)gLongestLoopMicros / 1000));
      gDisplay.mDisplay.println(String("Max idle ms:  ") + ((float)gLongestBetweenLoopMicros / 1000));
      gDisplay.mDisplay.println(String("") + (int)gFramerate.getFPS() + "fps szRx=" + sizeof(LHRHPayload));
      gDisplay.mDisplay.println(String("nUO:") + gUpdateObjectCount + " szTx=" + sizeof(MainChecksummablePayload));
    };
    
    auto pageAudioStatus = []() {
      gDisplay.mDisplay.println(String("AM Curr: ") + AudioMemoryUsage());
      gDisplay.mDisplay.println(String("AM Max: ") + AudioMemoryUsageMax() + " ALLOC:" + AUDIO_MEMORY_TO_ALLOCATE);
      gDisplay.mDisplay.println(String("CPU: ") + AudioProcessorUsage());
      gDisplay.mDisplay.println(String("CPU Max: ") + AudioProcessorUsageMax());
    };
    
    auto pageMemStatus = []() {
      // https://forum.pjrc.com/threads/58839-Teensy-4-0-memory-allocation
      // there are 2 locations we care about:
      // [......zeroed variables][local variables / stack][DMAMEM][ram2.........]0x20280000
      //                         ^^^^^^^^^^^^^^^^^^^^^^^^^        ^^^^^^^^^^^^^^^
      // the end of RAM is always 0x20280000, so if we assume that the memory allocator always returns the top location...
      // but these values look pretty garbage so ...
      uintptr_t n = (uintptr_t)malloc(1);
      free((void*)n);
      uintptr_t ram2End = 0x20280000;
      uintptr_t ram1End = ram2End - 524288;
      uintptr_t stackPtr = (uintptr_t)__builtin_frame_address(0);

      gDisplay.mDisplay.println(String("DMAfree:   ") + (ram2End - n));
      gDisplay.mDisplay.println(String("StackFree: ") + (ram1End - stackPtr));
    };

    switch(mPage) {
      case 0:
        pageLHRX();
        break;
      case 1:
        pageRHRX();
        break;
      case 2:
        pageMusicalState();
        break;
      case 3:
        pagePhysicalState();
        break;
      case 4:
        pageAudioStatus();
        break;
//      case 5:
//        pageDebugLHRX();
//        break;
//      case 6:
//        pageDebugRHRX();
//        break;
      case 5:
        pageDebugMain();
        break;
      case 6:
        pageMemStatus();
        break;
    }
  }
};
