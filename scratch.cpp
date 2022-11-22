#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <algorithm>

using Real = float;

static constexpr Real oversample = 1.0f; // not useful.

Adafruit_SSD1306 display(128, 64, &SPI, 40 /*DC*/, 41 /*RST*/, 10 /*CS*/, 88 * 1000000UL);

Real slider1 = 0.87f;
Real slider2 = 0.0f;
Real slider3 = 0;
Real ggain = 0.5f;

static constexpr uint32_t gWireDataRate = 400000;
static constexpr Real pi2 = Real(2.0) * Real(3.1415926535897932385);
inline bool FloatEquals(Real f1, Real f2, Real eps)
{
    return fabs(f1 - f2) < eps;
}
struct VirtualAxis
{
    VirtualAxis(const char *name) : mName(name)
    {
    }
    const char *mName = nullptr;
    float mMyVal = 0;
    void SetValue(float v)
    {
        mMyVal = v;
    }
};
struct Stopwatch
{
    uint64_t mWhenStarted = micros();
    void Restart()
    {
        mWhenStarted = micros();
    }
    uint64_t ElapsedMicros()
    {
        return (uint64_t)micros() - mWhenStarted;
    }
};

static Real Clamp(Real x, Real low, Real hi)
{
    if (x <= low)
        return low;
    if (x >= hi)
        return hi;
    return x;
}

struct ADS1115Device
{
    static constexpr uint8_t gDefaultAddress = 0x48;
    static constexpr uint16_t ADS1015_REG_CONFIG_CQUE_NONE =
        (0x0003); ///< Disable the comparator and put ALERT/RDY in high state (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_CLAT_NONLAT = (0x0000); ///< Non-latching comparator (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_CPOL_ACTVLOW =
        (0x0000); ///< ALERT/RDY pin is low when active (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_CMODE_TRAD =
        (0x0000); ///< Traditional comparator with hysteresis (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_860SPS = (0x00E0);    ///< 11100000 860 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_MODE_SINGLE = (0x0100);  ///< Power-down single-shot mode (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_PGA_6_144V = (0x0000);   ///< +/-6.144V range = Gain 2/3
    static constexpr uint16_t ADS1015_REG_CONFIG_OS_SINGLE = (0x8000);    ///< Write: Set to start a single-conversion
    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_0 = (0x4000); ///< Single-ended AIN0
    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_1 = (0x5000); ///< Single-ended AIN1
    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_2 = (0x6000); ///< Single-ended AIN2
    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_3 = (0x7000); ///< Single-ended AIN3
    static constexpr uint16_t ADS1015_REG_POINTER_CONVERT = (0x00);       ///< Conversion
    static constexpr uint16_t ADS1015_REG_POINTER_CONFIG = (0x01);        ///< Configuration

  private:
    uint16_t mLastKnownChannelValues[4] = {0};
    TwoWire &mWire;
    uint8_t mAddress;
    const uint32_t mConversionDelayMicros;
    static constexpr uint16_t mValueLowerBound = 11;    // the value at which we interpret it as "0".
    static constexpr uint16_t mValueUpperBound = 17500; // the value at which we interpret it as "1".

    static float ConvertValueTo01Clamped(uint16_t val)
    {
        return Clamp(float(val - mValueLowerBound) / mValueUpperBound, 0.0f, 1.0f);
    }

    enum class ConversionState
    {
        Waiting, // mCurrentlyReadingChannel is set and the timer is running for its conversion to complete.
        Idle,    // mCurrentlyReadingChannel and timer are meaningless; nothing is requested.
    };

    ConversionState mState = ConversionState::Idle;
    uint8_t mCurrentlyReadingChannel = 0;
    Stopwatch mConversionTimer;

    static constexpr uint32_t CalculateConversionDelayMicros(int samplesPerSecond)
    {
        // return 15000; // slower for debugging.
        return 1.05f * (1000000.0f / samplesPerSecond); // seconds per sample, *1000, *1000, with 5% extra for buffer
    }

    void writeRegister(uint8_t reg, uint16_t value)
    {
        mWire.beginTransmission(mAddress);
        mWire.write(reg);
        mWire.write((uint8_t)(value >> 8));
        mWire.write((uint8_t)(value & 0xFF));
        mWire.endTransmission();
    }

    uint16_t readRegister(uint8_t reg)
    {
        mWire.beginTransmission(mAddress);
        mWire.write(reg);
        mWire.endTransmission();
        mWire.requestFrom(mAddress, (uint8_t)2);
        uint8_t buf[2];
        mWire.readBytes(buf, 2);
        return (buf[0] << 8) | buf[1];
    }

    void beginRead(uint8_t channel)
    {
        uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
                          ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
                          ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                          ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
                          ADS1015_REG_CONFIG_DR_860SPS |    // data rate
                          ADS1015_REG_CONFIG_MODE_SINGLE |  // Single-shot mode (default)
                          ADS1015_REG_CONFIG_PGA_6_144V |   // gain mode
                          ADS1015_REG_CONFIG_OS_SINGLE      //  'start single-conversion' bit
            ;

        switch (channel)
        {
        case (0):
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
            break;
        case (1):
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
            break;
        case (2):
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
            break;
        case (3):
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
            break;
        }

        // Write config register to the ADC
        writeRegister(ADS1015_REG_POINTER_CONFIG, config);
    }

  public:
    ADS1115Device(TwoWire &wire = Wire, uint8_t i2cAddress = gDefaultAddress)
        : mWire(wire),                                                //
          mAddress(i2cAddress),                                       //
          mConversionDelayMicros(CalculateConversionDelayMicros(860)) //
    {
    }

    VirtualAxis mAnalogControls[4] = {
        {"knob1: "},
        {"knob2: "},
        {"knob3: "},
        {"knob4: "},
    };

    uint16_t GetValueForChannel(uint8_t channel) const
    {
        return mLastKnownChannelValues[channel];
    }

    void Begin(uint8_t channel = 0)
    {
        beginRead(channel);
        mCurrentlyReadingChannel = channel;
        mState = ConversionState::Waiting;
        // Serial.println(String("ADC restarting timer."));
        mConversionTimer.Restart();
    }

    // check whether the previous conversion is done; if so call the completion routine.
    void Update()
    {
        switch (mState)
        {
        case ConversionState::Idle:
            return;
        case ConversionState::Waiting:
            break;
        }

        // has the conversion completed? We deduce this by the timer. We could also request whether the conversion is
        // complete but the timing is fixed therefore faster.
        if (mConversionTimer.ElapsedMicros() < mConversionDelayMicros)
        {
            return;
        }

        uint16_t val = readRegister(ADS1015_REG_POINTER_CONVERT);
        mLastKnownChannelValues[mCurrentlyReadingChannel] = val;
        mAnalogControls[mCurrentlyReadingChannel].SetValue(ConvertValueTo01Clamped(val));
        Begin((mCurrentlyReadingChannel + 1) % 4);
    }
};

struct PCA9554
{
    void begin()
    {
        Wire.beginTransmission(0x38); // Choose the PCA9554A
        Wire.write(byte(0x03));       // port direction command
        Wire.write(byte(0xff));       // 1 = input,  0 = output
        Wire.endTransmission();       // End I2C connection
    }

    bool buttons[8] = {false};
    bool prevButtons[8] = {false};

    void update()
    {
        Wire.beginTransmission(0x38); // Choose the PCA9554A
        Wire.write(byte(0));          // command 0 = read inputs
        Wire.endTransmission();       // End I2C connection
        Wire.requestFrom(0x38, 1);    // Request 1 byte
        byte inputs = Wire.read();    // Copy values to variable inputs
        for (int i = 0; i < 8; ++i)
        {
            prevButtons[i] = buttons[i];
            buttons[i] = (!!(inputs & (1 << i)));
        }
    }
    bool IsTriggered(size_t btn) const
    {
        return prevButtons[btn] && !buttons[btn];
    }
};

static inline int16_t Sample32To16(Real s)
{
    return saturate16(int32_t(s * Real(32767)));
}

static inline void Sample32To16Buffer(const Real *in, int16_t *out)
{
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
    {
        out[i] = Sample32To16(in[i]);
    }
}

inline Real ccsin(Real x)
{
    return arm_sin_f32(x);
}

ADS1115Device mADS1115{Wire, 0x48};

struct Oscilloscope : public AudioStream
{
    audio_block_t *inputQueueArray[1];

    Oscilloscope() : AudioStream(1, inputQueueArray)
    {
    }
    Real osc[129] = {0};
    Real oscCursor = 0;
    Real lastOscCursor = 0;
    Real oscScale = 1.0f;
    Real rmin = 1;
    Real rmax = -1;
    bool hitMin = false;
    bool hitMax = false;

    Real fbN1 = 0;

    virtual void update() override
    {
        audio_block_t *inputBuf = receiveReadOnly(0);
        if (!inputBuf)
            return;

        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            Real s = Real(inputBuf->data[i]) / 32767;
            if (oscCursor > 192)
            {
                // when searching for a zero-crossing, only trig when we've encountered cycle extents. to prevent
                // triggering on harmonics.
                if (!hitMin)
                {
                    hitMin = (s <= rmin);
                    rmin *= 0.9999;
                }
                if (!hitMax)
                {
                    hitMax = (s >= rmax);
                    rmax *= 0.9999;
                }
                if (hitMin && hitMax && (fbN1 < 0 && s >= 0))
                { // zero crossing ascending
                    oscCursor = 0;
                    rmin = 1;
                    rmax = -1;
                    hitMin = false;
                }
            }
            else
            {
                // during the recording session, check min & max
                rmin = std::min(rmin, s);
                rmax = std::max(rmax, s);
            }
            if (oscCursor <= 129)
            {
                int old = (int)(floorf(lastOscCursor));
                int n = (int)(floorf(oscCursor));
                for (int i = old; i <= n; ++i)
                {
                    osc[i] = s;
                }
            }
            lastOscCursor = oscCursor;
            oscCursor += 1.0f / oscScale;

            fbN1 = s;
        }
        release(inputBuf);
    }
};

struct CycleRecorder
{
    // to stabilize floating point accumulation, let's record 1 cycle of our own waveform
    // and just replay it. it won't fit precisely into an exact integral sample amt though so have to consider that.

    // don't replay the actual samples though, just record the first feedbacky part of 'b'.
    static constexpr Real MinimumFrequency = 10;
    Real mBuffer[(size_t)(AUDIO_SAMPLE_RATE / MinimumFrequency)] = {0};
    Real mFrequency = -1; // so recording always triggers on 1st update
    size_t mSamplesPerCycle = 0;
    size_t mBufferWriteCursor = 0;
    size_t mBufferReadCursor = 0;
    Real mPrevPhase = 0;

    // returns false if b was not available and the caller should calculate it. caller should make a subsequent call to Record()
    // f = frequency
    // a = current phase, 0-1. if this crosses 1, we detect as a cycle loop.
    // b =
    bool OnPhaseUpdate(Real f, Real a, Real &b)
    {
        if (!FloatEquals(mFrequency, f, 0.00001)) // nice small eps for smooth modulated freqs
        {
            mFrequency = f;
            if (mFrequency < MinimumFrequency)
            {
                b = 0;
                return true;
            }
            mSamplesPerCycle = Real(AUDIO_SAMPLE_RATE_EXACT) / f;
            mBufferWriteCursor = 0;
            mBufferReadCursor = 0;
        }
        if (mFrequency < MinimumFrequency)
        {
            b = 0;
            return true;
        }

        bool isRecording = mBufferWriteCursor < mSamplesPerCycle;
        return false;

        if (a > 1)
        {
            a -= floorf(a);
            // phase crossing.
        }
    }

    void Record(Real b){
        mBuffer[mBufferWriteCursor] = b;
        ++ mBufferWriteCursor;
    }
};

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

    CycleRecorder mCycle;

    virtual void update() override
    {
        Real buf[AUDIO_BLOCK_SAMPLES] = {0};

        freq = 10.0 + 1000 * slider3;
        da = freq / 44100;

        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            Real s = Sample();
            fbN1 = s;
            buf[i] = Clamp(s * ggain, -1, 1);
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
        bool needsCalc = !mCycle.OnPhaseUpdate(freq, a, b); // returns the value, or requests it.
        a -= floorf(a);

        if (needsCalc)
        {
            b = a + ((fbN1)*slider1); // phase with variable feedback
            mCycle.Record(b);
        }

        b += slider2 * ccsin(pi2 * a); // FM carrier
        Real s = ccsin(pi2 * b);       // sine + fb
        return s;
    }
};

PCA9554 mPCA9554;

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
    Wire.setClock(gWireDataRate);

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

    if (mPCA9554.IsTriggered(4))
    {
        waveform1.fbResetMode = (waveform1.fbResetMode + 1) % 4;
        Serial.println(String("pressed 4; resetmode=") + waveform1.fbResetMode);
    }

    AudioNoInterrupts();
    slider1 = mADS1115.mAnalogControls[0].mMyVal * 0.5f;
    slider2 = mADS1115.mAnalogControls[1].mMyVal * 10.0f;
    slider3 = mADS1115.mAnalogControls[2].mMyVal;
    gOscilloscope.oscScale = mADS1115.mAnalogControls[2].mMyVal * 8.0f + 1.0f;
    ggain = mADS1115.mAnalogControls[3].mMyVal;
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
    display.println(String("fb:") + slider1 + " gain:" + ggain + " harm:" + slider2);

    display.display();

    delay(8);
}
