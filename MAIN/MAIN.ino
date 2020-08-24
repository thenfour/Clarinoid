// Teensy 4.0
// 600mhz
// "Faster"

// Check synth & CCEWIControl for parameters

// This main sketch pretty much just glues the application to the display & LEDs

// https://forum.pjrc.com/threads/57932-Latency-what-s-new
//#define AUDIO_BLOCK_SAMPLES 1024 // this seems to have no effect; may need to fork

//============================================================
/////////////////////////////////////////////////////////////////////////////////////////////////
static const size_t HARMONIZER_VOICES = 4;
static const size_t MAX_VOICES = 12;

class PresetName
{
  char buf[16];
};

struct Scale
{
  // notes
};

struct PerfSettings
{
  PresetName mName;
  uint8_t mHarmonizerPreset = 0;
  uint8_t mSynthPreset[HARMONIZER_VOICES] = {};
  int8_t mTranspose = 0;
  Scale mGlobalScale;
  float mBPM = 90.0f;
};
struct SystemSettings
{
  //
};
struct AppSettings
{
  float mPortamentoTime = 0.005f;
  
  bool mMetronomeOn = true;
  float mMetronomeGain = 0.8f;
  int mMetronomeNote = 80;
  int mMetronomeDecayMS= 15;
  
  float mReverbGain = .2f;
  int mTranspose = 0;

  PerfSettings mPerfSettings;
};

AppSettings gAppSettings;


//#include "AppSettings.h"

#include "Shared_CCSwitch.h"
#include "Shared_CCLeds.h"
#include "Shared_CCTxRx.h"

#include "CCEWIApplication.h"
#include "CCDisplay.h"
#include "CCMenuDebug.h"

CCEWIApp gApp;
CCDisplay gDisplay(gApp);

MetronomeSettingsApp gMetronomeApp(gDisplay, gApp);
DebugMenuApp gDebugApp(gDisplay, gApp);
SynthSettingsApp gSynthSettingsApp(gDisplay, gApp);

CCLeds leds(10, 2, 10, true);
CCThrottlerT<20> ledThrottle;


void setup() {
  Serial.begin(9600);
  SetupUpdateObjects();
}

bool firstLoop = true;
uint32_t gLoopExitMicros = micros();

void loop() {
  uint32_t m = micros();
  if (firstLoop) {
    firstLoop = false;
  } else {
    if (m > gLoopExitMicros && (m - gLoopExitMicros) > gLongestBetweenLoopMicros) {
      gLongestBetweenLoopMicros = m - gLoopExitMicros;
    }
  }

  if (UpdateObjectCount > MaxUpdateObjects) {
    CCPlot(String("Too many update objects!") + UpdateObjectCount);
  }

  UpdateUpdateObjects();

  if (ledThrottle.IsReady())
  {
    
    leds.setPixelColor(0, 0, 0, 0);
    leds.setPixelColor(1, col(gLHRXErrorIndicator.GetState()), col(gLHTXIndicator.GetState()), col(gLHRXIndicator.GetState()));
    leds.setPixelColor(2, col(gRHRXErrorIndicator.GetState()), col(gRHTXIndicator.GetState()), col(gRHRXIndicator.GetState()));
    leds.setPixelColor(3, 0, 0, col(gMidiActivityIndicator.GetState()));
    
    leds.setPixelColor(4, col(gAppSettings.mTranspose != 0), 0, 0);
    leds.setPixelColor(5, col(gAppSettings.mTranspose < 0), col(gAppSettings.mTranspose > 0), 0);

    // 6 = off
    // 7 = off

    leds.setPixelColor(8, col(gEncButton.IsCurrentlyPressed()), 0, col(gEncIndicator.GetState()));
    leds.setPixelColor(9, 0, gVolumePot.GetValue01() * 6, col(gVolIndicator.GetState()));
    leds.show();
  }

  uint32_t n = micros();
  if (n > m && (n - m) > gLongestLoopMicros) {
    gLongestLoopMicros = n - m;
  }
  
  gLoopExitMicros = micros();
}
