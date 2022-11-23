
#include "boiler.hpp"
#include "wf.hpp"

Adafruit_SSD1306 display(128, 64, &SPI, 40 /*DC*/, 41 /*RST*/, 10 /*CS*/, 88 * 1000000UL);
ADS1115Device mADS1115{Wire, 0x48};
PCA9554 mPCA9554;

/*
triggers:
- vis type
- control set

- osc1 waveform
- osc2 waveform
- osc1 hard sync enable
*/

bool gSyncEnable = false;

// SET 1
Real gOscScale = 0;        // slider1
Real gOutpGain = 0;        // slider2
Real gBothFrequency = 440; // slide1
Real gOsc2FreqMul = 1;     // slider2

// SET 2: fm
Real gOsc1PMFeedbackAmt = 0;
Real gOsc2PMFeedbackAmt = 0;
Real gOsc1PMModAmount = 0; // slider3

// // SET 3: ring mod
// Real gOsc1FeedbackAmt = 0;
// Real gOsc2FeedbackAmt = 0;
// Real gModAmount = 0; // slider3

// // SET 5: shape
// Real gOsc1Shape = 0; // slider4
// Real gOsc2Shape = 0; // slider3

int gOsc1Waveform = WAVEFORM_SINE;
int gOsc2Waveform = WAVEFORM_SINE;
bool gOsc1SyncEnabled = false;

float FFTMultiplier = 6.0f;

enum class VisType
{
    FFT,
    Oscilloscope,
};
VisType gVisType = VisType::FFT;

AudioAnalyzeFFT256 gFFT;
AudioSynthWaveformModulated2<2> wfmod1;
Oscilloscope gOscilloscope;
AudioOutputI2S i2s1; // xy=360,98
AudioConnection patchCord2(wfmod1, 0, gOscilloscope, 0);
AudioConnection patchCordaeu2(wfmod1, 0, gFFT, 0);
AudioConnection patchCord1(wfmod1, 0, i2s1, 0);
AudioConnection patchCord3(wfmod1, 0, i2s1, 1);

int16_t arbitraryWave[256] = {0};

void setup()
{
    Serial.begin(9600);
    CheckCrashReport();

    Wire.begin();
    Wire.setClock(400000);

    mADS1115.Begin();

    display.begin(SSD1306_SWITCHCAPVCC);
    AudioMemory(10);

    for (size_t i = 0; i < 256; ++i)
    {
        float f = arm_cos_f32(float(i) / 256 * pi2);
        float s = f >= 0 ? 1 : -1;
        f = sqrtf(sqrtf(fabs(f)));
        f *= s;
        arbitraryWave[i] = Sample32To16(f);
    }
    wfmod1.mOscillators[0].arbitraryWaveform(arbitraryWave);
    wfmod1.mOscillators[1].arbitraryWaveform(arbitraryWave);

    mPCA9554.begin();
    gFFT.averageTogether(8);
}

void loop()
{
    mADS1115.Update();
    mPCA9554.update();

    AudioNoInterrupts();

    if (mPCA9554.IsTriggered(0))
    {
        gOsc1Waveform = ToggleWaveform(gOsc1Waveform);
    }

    if (mPCA9554.IsTriggered(1))
    {
        gOsc2Waveform = ToggleWaveform(gOsc2Waveform);
    }

    if (mPCA9554.IsTriggered(2))
    {
        gSyncEnable = !gSyncEnable;
    }

    if (mPCA9554.IsTriggered(5))
    {
        switch (gVisType)
        {
        case VisType::FFT:
            gVisType = VisType::Oscilloscope;
            break;
        case VisType::Oscilloscope:
            gVisType = VisType::FFT;
            break;
        }
    }

    Real slider1 = mADS1115.mAnalogControls[0].mMyVal;
    Real slider2 = mADS1115.mAnalogControls[1].mMyVal;
    Real slider3 = mADS1115.mAnalogControls[2].mMyVal;
    Real slider4 = mADS1115.mAnalogControls[3].mMyVal;

    gOsc1PMFeedbackAmt = slider1;
    gOsc2PMFeedbackAmt = 0;
    gOsc1PMModAmount = mADS1115.mAnalogControls[1].mMyVal;

    wfmod1.mOscillators[0].SetFMMatrix(0, gOsc1PMFeedbackAmt * 360.0f * 2.0f);
    wfmod1.mOscillators[0].SetFMMatrix(1, gOsc1PMModAmount * 360 * 10.0f);

    gOscilloscope.oscScale = mADS1115.mAnalogControls[2].mMyVal * 8.0f + 1.0f;

    Real mainFreq = 2500 * slider3;
    mainFreq = floorf(mainFreq / 30) * 30; // stabilize?
    mainFreq += 20;
    Real syncFreq = mainFreq * (1 + slider4 * 8);
    wfmod1.mOscillators[0].SetParams(mainFreq, syncFreq, gSyncEnable, gOsc1Waveform);
    wfmod1.mOscillators[1].SetParams(mainFreq, 0, false, gOsc2Waveform);

    display.clearDisplay();

    switch (gVisType)
    {
    case VisType::Oscilloscope: {
        int prevY = 32;
        for (size_t x = 0; x < 128; ++x)
        {
            int y = (gOscilloscope.osc[x] * 32) + 32;
            y = Clamp(y, 0, 63);

            display.drawPixel(x, y, WHITE);
            if (prevY > y)
                display.writeFastVLine(x, y, abs(prevY - y), WHITE);
            else
                display.writeFastVLine(x, prevY, abs(prevY - y), WHITE);
            prevY = y;
        }
        break;
    }
    case VisType::FFT: {
        float maxHeight = 56;

        for (size_t x = 0; x < 128; ++x)
        {
            float yf = gFFT.read(x); // 0-1 scale
            int y = (yf * FFTMultiplier * maxHeight);
            y = maxHeight - Clamp(y, 0, maxHeight);
            display.writeFastVLine(x, y, maxHeight - y + 1, WHITE);
        }
        break;
    }
    }

    AudioInterrupts();

    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.println(String("") + slider1 + " " + slider2 + " " + slider3 + " " + slider4);
    display.setCursor(0, 56);
    display.println(gWaveformNames[gOsc1Waveform] + (wfmod1.mOscillators[0].IsHardSyncEnabled() ? "SYNC" : "") + " - " +
                    gWaveformNames[gOsc2Waveform]);

    display.display();

    delay(5);
}
