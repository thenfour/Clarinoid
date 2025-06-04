
// arduino-ish libraries
#define AUDIO_BLOCK_SAMPLES 128 // other values are currently producing artifacts.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <Arduino.h>

#include <Wire.h>
#include <Bounce.h>
#include <SPI.h>
#include <MIDI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Audio.h>
#include <Encoder.h>
#include <WS2812Serial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_MCP23017.h>

#pragma GCC diagnostic pop

#include "testDeviceApp.hpp"

using namespace clarinoid;

// OLED
// SMD LED
// USB HOST MIDI
// PCA9554
// MPR121 (touched + slider)
// ENCODER

#include <limits>
#include <algorithm>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include "Adafruit_MPR121.h"
#include <clarinoid/components/MPR121.hpp>
#include <Encoder.h>
#include "CapacitiveSlider.hpp"
#include "CapGraphs.hpp"
#include <clarinoid/components/HoneywellABPI2C.hpp>

MPR121::MPR121Device cap;
static constexpr int kNumElectrodes = 10;
BipolarCapacitiveSlider<kNumElectrodes> gSlider;

Encoder myEnc(26, 9);

CCHoneywellAPB gPressureSensor{ Wire };
SimpleMovingAverage<10> gPressureAvg;

struct PCA9554APW_118
{
  PCA9554APW_118()
  {
    Wire.begin(400000);
    Wire.beginTransmission(0x38); // Choose the PCA9554A
    Wire.write(byte(0x03));       // port direction command
    Wire.write(byte(0xff));       // 1 = input,  0 = output
    Wire.endTransmission();       // End I2C connection
  }
  void Loop()
  {
    String s = "";
    Wire.beginTransmission(0x38); // Choose the PCA9554A
    Wire.write(byte(0));          // command 0 = read inputs
    Wire.endTransmission();       // End I2C connection
    Wire.requestFrom(0x38, 1);    // Request 1 byte
    byte inputs = Wire.read();    // Copy values to variable inputs
    for (int i = 0; i < 8; ++i) {
      s += (!!(inputs & (1 << i))) ? 1 : 0;
    }
    clarinoid::gDisplay.PrintLine(s);
  }
};

// void
// setup()
// {
//   xDisplay.begin(SSD1306_SWITCHCAPVCC);
//   cap.begin(0x5A);

//   gSlider.Init(cap);
//   // myusb.begin();
//   // midi1.setHandleNoteOff(OnNoteOff);
//   // midi1.setHandleNoteOn(OnNoteOn);
// }

PCA9554APW_118 buttons;

int lastNote = 0;
int lastVelocity = 0;
int keyRefcount = 0;

// void OnNoteOn(byte channel, byte note, byte velocity)
// {
//   lastNote = note;
//   lastVelocity = velocity;
//   keyRefcount ++;
// }

// void OnNoteOff(byte channel, byte note, byte velocity)
// {
//   keyRefcount --;
//   if (keyRefcount <= 0) {
//     keyRefcount = 0;
//     lastNote = 0;
//     lastVelocity = 0;
//   }
// }

int pins[] = { 2, 3, 4, 5 };
int vals[] = { 1, 3, 9, 25 };
const int valCount = 4;

int displayTime = 0;
int frame = 0;

// void loopX()
// {
//     auto m1 = micros();

//     //   myusb.Task();
//     //   midi1.read();

//     for (int pin : pins)
//     {
//         pinMode(pin, OUTPUT);
//         analogWrite(pin, vals[frame % valCount]);
//     }

//     display.clearDisplay();
//     display.setTextSize(1);
//     display.setTextColor(WHITE);
//     display.setCursor(0, 0);
//     display.println(String("t:") + displayTime + ", " + frame);
//     String notes = "C:";
//     notes += keyRefcount;
//     notes += " ";
//     if (keyRefcount > 0)
//     {
//         notes += String(lastNote) + " @" + lastVelocity;
//     }
//     display.println(notes);

//     buttons.Loop();

//     gSlider.Update();
//     gSlider.GetValue01();
//     display.println(String("slider:") + gSlider.GetValue01());

//     // CAP TOUCH 5x5x1 uses 11 pads
//     const size_t capCount = 10;
//     int data[capCount];
//     int dataMin = std::numeric_limits<int>::max();
//     int dataMax = std::numeric_limits<int>::min();

//     for (uint8_t i = 0; i < capCount; i++)
//     {
//         data[i] = cap.filteredData(i);
//         dataMin = std::min(data[i], dataMin);
//         dataMax = std::max(data[i], dataMax);
//     }
//     int dataRange = dataMax - dataMin;
//     if (dataRange < 1)
//         dataRange = 1;
//     // display.print(String("cap:") + dataMin);
//     // display.print(String("-") + dataMax);
//     // display.println(String(" (") + dataRange + ")");

//     // scale 0-1
//     const char asciigray[] = " .:-=+*%@#";
//     float dataf[capCount];
//     for (uint8_t i = 0; i < capCount; i++)
//     {
//         dataf[i] = (float)(data[i] - dataMin) / dataRange;
//         int idx = 10 - int(dataf[i] * 9.99f);
//         if (idx < 0)
//             idx = 0;
//         if (idx > 9)
//             idx = 9;
//         char t[2] = {asciigray[idx], 0};
//         display.print(t);
//     }
//     display.println();

//     // touched pads
//     int currtouched = cap.touched();

//     for (uint8_t i = 0; i < capCount; i++)
//     {
//         display.print((currtouched & (1 << i)) ? "^" : " ");
//     }
//     display.println();

//     display.println(String("enc/alpha:") + (myEnc.read() / 4) + " / " + gSlider.GetAlpha());

//     gSlider.SetAlpha((myEnc.read() / 400.0f));

//     display.display();

//     auto m2 = micros();

//     frame++;
//     displayTime = m2 - m1;

//     delay(20);
// }

void
setup()
{
  clarinoid::gDisplay.begin(SSD1306_SWITCHCAPVCC);

  clarinoid::gCrashHandlers[0] = &clarinoid::gSerialCrashHandler;
  clarinoid::gCrashHandlers[1] = &clarinoid::gDisplay;
  clarinoid::CheckCrashReport();

  // ModCurveMain();
  // ModBenchmarkMain();
  Serial.begin(9600);
  // while (!Serial) {
  // } // when you are debugging with serial, uncomment this to ensure you see startup msgs
  // Serial.println("starting normally....");

  Wire.begin();
  Wire.setClock(400000);

  cap.begin(0x5A, &Wire, 12, 6, kNumElectrodes, true);
  gSlider.Init(cap);
  //   // myusb.begin();

  // auto *app = new clarinoid::TestDeviceApp;
  // app->Main();
}

void
loop()
{
  auto& display = clarinoid::gDisplay;
  // Serial.println(String("t:") + displayTime + ", " + frame);

  gPressureSensor.Update();

  for (int pin : pins) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, vals[frame % valCount]);
  }

  clarinoid::gDisplay.FillScreen(SSD1306_BLACK);
  clarinoid::gDisplay.SetTextSize(SizeI::Square(1));
  clarinoid::gDisplay.SetTextColor(WHITE);
  display.SetTextLeftMargin(0);
  clarinoid::gDisplay.SetCursor({ 0, 0 });

  // {
  //   gSlider.Update();

  //   auto sliderValN11 = gSlider.GetValueN11();
  //   auto sliderSlice = display.ScreenRect().VerticalSlice(0, 8);
  //   display.FillRectWithBrightness(sliderSlice, 24);
  //   display.FillRectWithBrightness(sliderSlice.WithBipolarVerticalFill(sliderValN11), 255);

  //   int textWidth = 64;
  //   auto rcBarArea = display.ScreenRect().WithOffsetLeft(10).WithOffsetRight(-textWidth);

  //   display.SetTextLeftMargin(8);
  //   clarinoid::gDisplay.SetCursor({ 8, 0 });
  //   display.PrintLine(String("") + (gSlider.IsTouched() ? "Touched" : "Untouched"));
  //   display.PrintLine(String("") + sliderValN11);

  //   display.SetTextLeftMargin(display.ScreenRect().Right() - textWidth);
  //   display.SetCursor({ display.GetTextLeftMargin(), 0 });

  //   for (int i = 0; i < kNumElectrodes; i++) {
  //     auto str = gSlider.mSlider.computeTouchStrength(i);
  //     display.PrintLine(String(i) + ":" + str);
  //     auto rc = rcBarArea.Cell(kNumElectrodes, 1, i, 0);
  //     display.FillRectWithBrightness(rc.BottomFraction(sqrtf(RemapTo01(str, 0, 600))), 128);
  //   }
  // }

  float pressureRaw01 = gPressureSensor.CurrentValue01();

  // Breath unsmoothed
  {
    // float pressureScaled01 = map(pressureRaw01, kRawPressureMin, kRawPressureMax, 0, 1);
    //  get slice of the top 8 pixels of the screen.
    auto breathSlice = display.ScreenRect().CellWithSize(128, 8, 0, 0);
    display.FillRectWithBrightness(breathSlice, 24);
    display.FillRectWithBrightness(breathSlice.LeftFraction(pressureRaw01), 255);

    auto rawPressureTextRect = display.ScreenRect().CellWithSize(128, 8, 0, 1);
    display.SetCursor(rawPressureTextRect.TopLeft());
    display.FillRect(rawPressureTextRect, SSD1306_BLACK);
    display.PrintLine(String("") + pressureRaw01 * 100);
  }

  // breath smoothed
  gPressureAvg.Update(pressureRaw01);
  {
    static constexpr float kRawPressureMin = 0.10f;
    static constexpr float kRawPressureMax = 0.35f;
    float pressureSmoothed01 = Clamp01(gPressureAvg.GetValue());
    float pressureScaled01 = map(pressureSmoothed01, kRawPressureMin, kRawPressureMax, 0, 1);
    auto graphSlice = display.ScreenRect().CellWithSize(128, 8, 0, 3);
    display.FillRectWithBrightness(graphSlice, 24);
    display.FillRectWithBrightness(graphSlice.LeftFraction(pressureScaled01), 255);

    auto rawPressureTextRect = display.ScreenRect().CellWithSize(128, 8, 0, 4);
    display.SetCursor(rawPressureTextRect.TopLeft());
    display.FillRect(rawPressureTextRect, SSD1306_BLACK);
    display.PrintLine(String("") + pressureScaled01 * 100 + " - " + pressureSmoothed01 * 100);
  }

  clarinoid::gDisplay.PresentToDevice();

  frame++;
  delay(3);
}
