
#pragma once

namespace clarinoid
{

// // manually-controlled analog channel. like how we use VirtualButtons or BitButton controls for MCP23017 / PCA9554,
// // this lets us establish analog channels for each channel of the ADS1115.
// struct VirtualAnalogChannel : IAnalogAxis
// {
//     float mValue = 0.0f;
//     virtual float CurrentValue01() const override
//     {
//         return mValue;
//     }
// };

// This device can be used in different ways, for example various MUX modes, continuous vs single-shot, requesting
// status, gain modes, etc. for our purpose, set the data rate as high as possible (860 SPS), and use single shot mode
// only.
struct ADS1115Device
{
    static constexpr uint8_t gDefaultAddress = 0x48;
    static constexpr uint16_t ADS1015_REG_CONFIG_CQUE_NONE =
        (0x0003); ///< Disable the comparator and put ALERT/RDY in high state (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_CLAT_NONLAT = (0x0000); ///< Non-latching comparator (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_CPOL_ACTVLOW =
        (0x0000);                                                        ///< ALERT/RDY pin is low when active (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_CPOL_ACTVHI = (0x0008); ///< ALERT/RDY pin is high when active
    static constexpr uint16_t ADS1015_REG_CONFIG_CMODE_TRAD =
        (0x0000); ///< Traditional comparator with hysteresis (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_CMODE_WINDOW = (0x0010); ///< Window comparator
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_8SPS = (0x0000);      ///<  00000000 8 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_16SPS = (0x0020);     ///<  00100000 16 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_32SPS = (0x0040);     ///<  01000000 32 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_64SPS = (0x0060);     ///<  01100000 64 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_128SPS = (0x0080);    ///< 10000000 128 samples per second (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_250SPS = (0x00A0);    ///< 10100000 250 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_475SPS = (0x00C0);    ///< 11000000 475 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_DR_860SPS = (0x00E0);    ///< 11100000 860 samples per second
    static constexpr uint16_t ADS1015_REG_CONFIG_MODE_CONTIN = (0x0000);  ///< Continuous conversion mode
    static constexpr uint16_t ADS1015_REG_CONFIG_MODE_SINGLE = (0x0100);  ///< Power-down single-shot mode (default)

    static constexpr uint16_t ADS1015_REG_CONFIG_PGA_6_144V = (0x0000); ///< +/-6.144V range = Gain 2/3
    static constexpr uint16_t ADS1015_REG_CONFIG_PGA_4_096V = (0x0200); ///< +/-4.096V range = Gain 1
    static constexpr uint16_t ADS1015_REG_CONFIG_PGA_2_048V = (0x0400); ///< +/-2.048V range = Gain 2 (default)
    static constexpr uint16_t ADS1015_REG_CONFIG_PGA_1_024V = (0x0600); ///< +/-1.024V range = Gain 4
    static constexpr uint16_t ADS1015_REG_CONFIG_PGA_0_512V = (0x0800); ///< +/-0.512V range = Gain 8
    static constexpr uint16_t ADS1015_REG_CONFIG_PGA_0_256V = (0x0A00); ///< +/-0.256V range = Gain 16

    static constexpr uint16_t ADS1015_REG_CONFIG_OS_SINGLE = (0x8000); ///< Write: Set to start a single-conversion
    static constexpr uint16_t ADS1015_REG_CONFIG_OS_BUSY = (0x0000);   ///< Read: Bit = 0 when conversion is in progress
    static constexpr uint16_t ADS1015_REG_CONFIG_OS_NOTBUSY =
        (0x8000); ///< Read: Bit = 1 when device is not performing a conversion

    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_0 = (0x4000); ///< Single-ended AIN0
    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_1 = (0x5000); ///< Single-ended AIN1
    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_2 = (0x6000); ///< Single-ended AIN2
    static constexpr uint16_t ADS1015_REG_CONFIG_MUX_SINGLE_3 = (0x7000); ///< Single-ended AIN3

    static constexpr uint16_t ADS1015_REG_POINTER_MASK = (0x03);      ///< Point mask
    static constexpr uint16_t ADS1015_REG_POINTER_CONVERT = (0x00);   ///< Conversion
    static constexpr uint16_t ADS1015_REG_POINTER_CONFIG = (0x01);    ///< Configuration
    static constexpr uint16_t ADS1015_REG_POINTER_LOWTHRESH = (0x02); ///< Low threshold
    static constexpr uint16_t ADS1015_REG_POINTER_HITHRESH = (0x03);  ///< High threshold

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
    Stopwatch mDebugTimer;

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
        CCASSERT(channel <= 3);

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
        mDebugTimer.Restart();
    }

    VirtualAxis mAnalogControls[4];

    uint16_t GetValueForChannel(uint8_t channel) const
    {
        CCASSERT(channel <= 3);
        return mLastKnownChannelValues[channel];
    }

    void Begin(uint8_t channel = 0)
    {
        CCASSERT(channel <= 3);
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
            // nothing to do.
            // Serial.println(String("ADC idle"));
            return;
        case ConversionState::Waiting:
            break;
        }

        // has the conversion completed? We deduce this by the timer. We could also request whether the conversion is
        // complete but the timing is fixed therefore faster.
        if (mConversionTimer.ElapsedTime().ElapsedMicros() < mConversionDelayMicros)
        {
            // not ready yet.
            // Serial.println(String("ADC not ready ") + (int)(mConversionTimer.ElapsedTime().ElapsedMicros()) + " < " +
            // mConversionDelayMicros);
            return;
        }

        uint16_t val = readRegister(ADS1015_REG_POINTER_CONVERT);
        mLastKnownChannelValues[mCurrentlyReadingChannel] = val;
        mAnalogControls[mCurrentlyReadingChannel].mValue = ConvertValueTo01Clamped(val);

        if (mDebugTimer.ElapsedTime().ElapsedMillisI() > 30)
        {
            // Serial.println(String("ADC state [") + mLastKnownChannelValues[0] + ", " + mLastKnownChannelValues[1] +
            //                ", " + mLastKnownChannelValues[2] + ", " + mLastKnownChannelValues[3] + "]");
            // Serial.println(String("ADC state [") + mAnalogControls[0].CurrentValue01() + ", " +
            // mAnalogControls[1].CurrentValue01() +
            //                ", " + mAnalogControls[2].CurrentValue01() + ", " + mAnalogControls[3].CurrentValue01() +
            //                "]");
            mDebugTimer.Restart();
        }
        Begin((mCurrentlyReadingChannel + 1) % 4);
    }
};

} // namespace clarinoid
