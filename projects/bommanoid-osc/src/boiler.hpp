#pragma once

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <algorithm>
#include <array>

#include "boiler.hpp"

using Real = float;

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

template <typename T>
struct OnScopeExitHelper
{
    T mRoutine;
    explicit OnScopeExitHelper(T &&fn) : mRoutine(std::move(fn))
    {
    }
    ~OnScopeExitHelper()
    {
        mRoutine();
    }
};

template <typename T>
OnScopeExitHelper<T> OnScopeExit(T &&fn)
{
    return OnScopeExitHelper<T>{std::move(fn)};
}

inline void CheckCrashReport()
{
    if (CrashReport)
    {
        while (!Serial)
            ;
        while (true)
        {
            CrashReport.printTo(Serial);
            delay(2500);
        }
    }
}

//////////////////////////////////////////////////////////////////////
template <uint32_t TperiodMS>
class CCThrottlerT
{
    uint32_t mPeriodStartMS;
    uint32_t mFirstPeriodStartMS;

  public:
    CCThrottlerT()
    {
        mPeriodStartMS = mFirstPeriodStartMS = millis();
    }

    void Reset()
    {
        mPeriodStartMS = mFirstPeriodStartMS = millis();
    }

    bool IsReady()
    {
        return IsReady(TperiodMS);
    }

    float GetBeatFloat(uint32_t periodMS) const
    {
        auto now = millis(); // minus is more theoretically accurate but this serves the purpose just as well.
        float f = abs(float(now - mFirstPeriodStartMS) / periodMS);
        return f;
    }
    // returns 0-1 the time since the last "beat".
    float GetBeatFrac(uint32_t periodMS) const
    {
        float f = GetBeatFloat(periodMS);
        return f - floor(f); // fractional part only.
    }
    int GetBeatInt(uint32_t periodMS) const
    {
        float f = GetBeatFloat(periodMS);
        return (int)floor(f);
    }

    bool IsReady(uint32_t periodMS)
    {
        auto now = millis(); // minus is more theoretically accurate but this serves the purpose just as well.
        if (now - mPeriodStartMS < periodMS)
        {
            return false;
        }
        mPeriodStartMS +=
            periodMS * ((now - mPeriodStartMS) /
                        periodMS); // this potentially advances multiple periods if needed so we don't get backed up.
        return true;
    }
};

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
static inline float Sample16To32(int16_t s)
{
    if (s == -32768)
        return -1.0f;
    return s / 32767.0f;
}

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

struct SyncedOscilloscope : public AudioStream
{
    audio_block_t *inputQueueArray[1];

    SyncedOscilloscope() : AudioStream(1, inputQueueArray)
    {
        SetFrequency(100);
    }
    Real osc[129] = {0}; // pixels

    Real GetPeak() const
    {
        return oldPeak;
    }

    void GetNormalizedData(std::array<Real, 129> &outp)
    {
        Real peak = GetPeak();
        peak = std::max(0.05f, peak);
        Real mul = 1.0f / peak;
        for (size_t i = 0; i < 129; ++i)
        {
            outp[i] = 1.0f - (osc[i] * mul);
        }
    }

    Real peak = 0;
    Real oldPeak = 0;

    Real mFrequency = 0;
    int mSamplesPerCycle = 0;
    int mSamplesPerPixel = 0;
    bool lookForTrigger = false;

    Real fbN1 = 0;
    uint32_t mSampleCount = 0;

    static constexpr Real gMinCyclesPerDisplay = 1.2f;

    void SetFrequency(float hz)
    {
        hz /= gMinCyclesPerDisplay;

        if (FloatEquals(hz, mFrequency, 0.01f))
        {
            return;
        }
        mFrequency = hz;
        Real samplesPerCycle = AUDIO_SAMPLE_RATE / hz; // nyquist=2, 30hz = 1470, 17.5. 20hz = 2205
        mSamplesPerCycle = ceilf((128.0f / samplesPerCycle) + 0.5f) * samplesPerCycle; // 2205
        // ensure cycle is long enough to cover a window
        mSamplesPerPixel = std::max(1, mSamplesPerCycle / 128); // 20hz = 17
        // make samples per cycle be longer than a window full
        Real t = mSamplesPerPixel * 128;
        // round up to a multiple of hz
        mSamplesPerCycle = ceilf((t / mSamplesPerCycle) + 0.5f) * mSamplesPerCycle;
    }

    CCThrottlerT<1000> mth;

    void Trigger()
    {
        if (mth.IsReady())
        {
            int oscCursor = mSampleCount / mSamplesPerPixel;
            // Serial.println(String("trigger @ sample ") + mSampleCount + " .. oscCursor = " + oscCursor + "
            // mSamplesPerCycle=" +mSamplesPerCycle + " mSamplesPerPixel=" + mSamplesPerPixel + " mFrequency=" +
            // mFrequency);
        }
        oldPeak = peak;
        // mOscCursor = 0;
        peak = 0;
        // mSubCursor = 0;
        mSampleCount = 0;
        lookForTrigger = false;
        // mSamplesUntilTrigger = mSamplesPerCycle;
    }

    virtual void update() override
    {
        audio_block_t *inputBuf = receiveReadOnly(0);
        if (!inputBuf)
            return;

        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            Real s = Real(inputBuf->data[i]) / 32767;

            if ((mSampleCount % mSamplesPerCycle) == 0)
            {
                lookForTrigger = true;
            }

            if (lookForTrigger && (fbN1 < 0 && s >= 0))
            {
                Trigger();
            }

            int oscCursor = mSampleCount / mSamplesPerPixel;

            if (oscCursor < 129)
            {
                osc[oscCursor] = s;
            }

            peak = std::max(peak, fabsf(s));
            fbN1 = s;

            // mSubCursor++;
            // if (mSubCursor >= mSamplesPerPixel)
            // {
            //     mSubCursor = 0;
            //     mOscCursor++;
            // }
            mSampleCount++;
        }

        release(inputBuf);
    }
};

const String gWaveformNames[] = {
    "SINE",
    "SAW",
    "SQUARE", // not needed; just use pulse
    "TRIANGLE",
    "ARB", // not yet needed
    "PULSE",
    "SAW_REV",
    "SH",
    "vartri",
    "BL_SAW",
    "BL_SAW_REV",
    "BL_SQUARE", // not needed; use pulse
    "BL_PULSE",
};

// add 2 bools: bandlimited, and reverse
const int AvailableWaveforms[]{
    WAVEFORM_SINE,

    WAVEFORM_SAWTOOTH,
    // WAVEFORM_SAWTOOTH_REVERSE,
    // WAVEFORM_BANDLIMIT_SAWTOOTH,
    // WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE,

    WAVEFORM_TRIANGLE,
    // WAVEFORM_TRIANGLE_VARIABLE,

    WAVEFORM_PULSE,
    // WAVEFORM_BANDLIMIT_PULSE,

    WAVEFORM_SAMPLE_HOLD,
};

int ToggleWaveform(int n)
{
    return (n + 1) % 13;
}
