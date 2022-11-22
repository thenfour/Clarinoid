
#include "boiler.hpp"

Adafruit_SSD1306 display(128, 64, &SPI, 40 /*DC*/, 41 /*RST*/, 10 /*CS*/, 88 * 1000000UL);
ADS1115Device mADS1115{Wire, 0x48};
PCA9554 mPCA9554;

Real slider1 = 0;
Real slider2 = 0;
Real slider3 = 0;
Real slider4 = 0;

//////////////////////////////////////////////////////////////////////////////
struct CustomOsc : public AudioStream
{
    CustomOsc() : AudioStream(0, nullptr)
    {
    }

    Real fbN1 = 1.0f; // because when phase = 0, sample = 1.
    Real a = 0;
    Real da = 0;
    Real freq = 0;

    virtual void update() override
    {
        Real buf[AUDIO_BLOCK_SAMPLES] = {0};

        freq = 10.0 + 5000 * slider3;
        da = freq / 44100;

        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            Real s = Sample();
            fbN1 = s;
            buf[i] = Clamp(s * slider4, -1, 1);
        }

        audio_block_t *out = allocate();
        if (!out)
            return;
        Sample32To16Buffer(buf, out->data);
        transmit(out, 0);
        release(out);
    }

    Real Sample()
    {
        a += da;
        Real b;
        a -= floorf(a);

        b = a + ((fbN1)*slider1); // phase with variable feedback

        b += slider2 * arm_cos_f32(pi2 * a); // FM carrier
        Real s = arm_cos_f32(pi2 * b);       // sine + fb
        return s;
    }
};

// AudioSynthWaveform waveform1;
CustomOsc waveform1; // xy=171,84
Oscilloscope gOscilloscope;
AudioOutputI2S i2s1; // xy=360,98
AudioConnection patchCord1(waveform1, 0, i2s1, 0);
AudioConnection patchCord2(waveform1, 0, gOscilloscope, 0);
AudioConnection patchCord3(waveform1, 0, i2s1, 1);

void setup()
{
    Serial.begin(9600);
    Wire.begin();
    Wire.setClock(400000);

    mADS1115.Begin();

    //  waveform1.begin(WAVEFORM_SAWTOOTH);
    //  waveform1.amplitude(.3f);
    //  waveform1.frequency(440.0f);

    display.begin(SSD1306_SWITCHCAPVCC);
    AudioMemory(10);

    mPCA9554.begin();
}

void loop()
{
    mADS1115.Update();
    mPCA9554.update();

    // if (mPCA9554.IsTriggered(4))
    // {
    //     waveform1.fbResetMode = (waveform1.fbResetMode + 1) % 4;
    //     Serial.println(String("pressed 4; resetmode=") + waveform1.fbResetMode);
    // }

    AudioNoInterrupts();
    slider1 = mADS1115.mAnalogControls[0].mMyVal * 0.5f;
    slider2 = mADS1115.mAnalogControls[1].mMyVal * 10.0f;
    slider3 = mADS1115.mAnalogControls[2].mMyVal;
    gOscilloscope.oscScale = mADS1115.mAnalogControls[2].mMyVal * 8.0f + 1.0f;
    slider4 = mADS1115.mAnalogControls[3].mMyVal;
    AudioInterrupts();

    display.clearDisplay();

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

    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.println(String("") + slider1 + " " + slider2 + " " + slider3 + " " + slider4);

    display.display();

    delay(8);
}
