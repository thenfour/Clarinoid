
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

// // SET 4: hard sync
// Real gOsc1SyncFreq = 0; // slider4
// Real gModAmount = 0; // slider3

// // SET 5: shape
// Real gOsc1Shape = 0; // slider4
// Real gOsc2Shape = 0; // slider3

int gOsc1Waveform = WAVEFORM_SINE;
int gOsc2Waveform = WAVEFORM_SINE;
bool gOsc1SyncEnabled = false;

float FFTMultiplier = 6.0f;

// enum class ControlSet
// {
//     Basic = 0,
//     PM,
//     COUNT
// };
// ControlSet gControlSet = ControlSet::Basic;
// void ToggleControlSet()
// {
//     gControlSet = (ControlSet)((int(gControlSet)) + 1 % int(ControlSet::COUNT));
// }
// const char* gControlSetNames[] = {
//     "Base",
//     "Ph mod",
// };

enum class VisType
{
    FFT,
    Oscilloscope,
};
VisType gVisType = VisType::FFT;

// //////////////////////////////////////////////////////////////////////////////
// struct CustomOsc : public AudioStream
// {
//     CustomOsc() : AudioStream(0, nullptr)
//     {
//     }

//     Real fbN1 = 1.0f; // because when phase = 0, sample = 1.
//     Real a = 0;
//     Real da = 0;
//     Real freq = 0;

//     virtual void update() override
//     {
//         Real buf[AUDIO_BLOCK_SAMPLES] = {0};

//         freq = 10.0 + 5000 * slider3;
//         da = freq / 44100;

//         for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
//         {
//             Real s = Sample();
//             fbN1 = s;
//             buf[i] = Clamp(s * slider4, -1, 1);
//         }

//         audio_block_t *out = allocate();
//         if (!out)
//             return;
//         Sample32To16Buffer(buf, out->data);
//         transmit(out, 0);
//         release(out);
//     }

//     Real Sample()
//     {
//         a += da;
//         Real b;
//         a -= floorf(a);

//         b = a + ((fbN1)*slider1); // phase with variable feedback

//         b += slider2 * arm_cos_f32(pi2 * a); // FM carrier
//         Real s = arm_cos_f32(pi2 * b);       // sine + fb
//         return s;
//     }
// };

// AudioSynthWaveform waveform1;
// CustomOsc waveform1; // xy=171,84
AudioAnalyzeFFT256 gFFT;
AudioSynthWaveformModulated2 wfmod2; // modulator
AudioSynthWaveformModulated2 wfmod1;
AudioConnection patchCordx(wfmod2, 0, wfmod1, 0);
Oscilloscope gOscilloscope;
AudioOutputI2S i2s1; // xy=360,98
AudioConnection patchCord1(wfmod1, 0, i2s1, 0);
AudioConnection patchCord2(wfmod1, 0, gOscilloscope, 0);
AudioConnection patchCordaeu2(wfmod1, 0, gFFT, 0);
AudioConnection patchCord3(wfmod1, 0, i2s1, 1);

int16_t arbitraryWave[256] = {0};

void setup()
{
    Serial.begin(9600);
    Wire.begin();
    Wire.setClock(400000);

    mADS1115.Begin();

    //  waveform1.begin(WAVEFORM_SAWTOOTH);
    //  waveform1.amplitude(.3f);
    //  waveform1.frequency(440.0f);

    // wfmod1.begin(WAVEFORM_SINE);
    // wfmod2.begin(WAVEFORM_SINE);

    display.begin(SSD1306_SWITCHCAPVCC);
    AudioMemory(10);

    for (size_t i = 0; i < 256; ++ i) {
        float f = arm_cos_f32(float(i) / 256 * pi2);
        float s = f >= 0 ? 1 : -1;
        f = sqrtf(sqrtf(fabs(f)));
        f *= s;
        arbitraryWave[i] = Sample32To16(f);
    }
    wfmod1.arbitraryWaveform(arbitraryWave);
    wfmod2.arbitraryWaveform(arbitraryWave);

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
        //wfmod1.begin(gOsc1Waveform);
    }

    if (mPCA9554.IsTriggered(1))
    {
        gOsc2Waveform = ToggleWaveform(gOsc2Waveform);
        //wfmod2.begin(gOsc2Waveform);
    }


    if (mPCA9554.IsTriggered(2))
    {
        gSyncEnable = !gSyncEnable;
        //wfmod1.SetHardSyncEnable(!wfmod1.IsHardSyncEnabled());
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

    wfmod1.phaseModulationFeedback(gOsc1PMFeedbackAmt * 360.0f * 2.0f);
    wfmod1.phaseModulation(gOsc1PMModAmount * 360 * 10.0f);
    // slider3 = mADS1115.mAnalogControls[2].mMyVal;
    gOscilloscope.oscScale = mADS1115.mAnalogControls[2].mMyVal * 8.0f + 1.0f;
    // slider4 = mADS1115.mAnalogControls[3].mMyVal;

    Real mainFreq = 2500 * slider3;
    mainFreq = floorf(mainFreq/30)*30; // stabilize?
    mainFreq += 20;
    Real syncFreq = mainFreq * (1 + slider4 * 8);
    wfmod1.SetParams(mainFreq, syncFreq, gSyncEnable, gOsc1Waveform);
    wfmod2.SetParams(mainFreq, 0, false, gOsc2Waveform);
    //wfmod2.frequency(freq);
    //wfmod1.SetHardSyncFreq();

    // wfmod1.amplitude(Clamp(slider4, 0, 1));
    // wfmod2.amplitude(0.5f);

    wfmod1.amplitude(0.25f); // account for gibbs
    wfmod2.amplitude(0.25f); // account for gibbs

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
    display.println(gWaveformNames[gOsc1Waveform] + (wfmod1.IsHardSyncEnabled() ? "SYNC" : "") + " - " + gWaveformNames[gOsc2Waveform]);

    display.display();

    delay(5);
}
