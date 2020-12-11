
#pragma once

namespace clarinoid
{
    struct ISwitch
    {
        virtual bool CurrentValue() const
        {
            CCASSERT(false);
            return false;
        }
    };

    // if you need to track previous values ("is newly pressed"), use this instead of reading the control directly.
    struct SwitchControlReader
    {
    private:
        ISwitch *mC = nullptr;
        bool mDirty = false;
        bool mPreviousValue = false;
        bool mCurrentValue = false;
        bool mFirst = true;

    public:
        explicit SwitchControlReader(ISwitch *c) : mC(c)
        {
            // NB: don't update in ctor because it's a pure virtual fn call :/
        }

        explicit SwitchControlReader()
        {
        }

        void SetSource(ISwitch *c)
        {
            mC = c;
            mFirst = true;
        }

        // Each call to Update() frames Dirty / Previous / Current
        void Update()
        {
            if (mFirst)
            {
                mCurrentValue = mC->CurrentValue();
                mPreviousValue = mCurrentValue;
                mDirty = false;
                mFirst = false;
                return;
            }
            mPreviousValue = mCurrentValue;
            mCurrentValue = mC->CurrentValue();
            mDirty = mPreviousValue != mCurrentValue;
        }

        // overkill alert
        // but seriously it's good when dealing with bools to have very explicit & precise naming.
        // For example "IsPressed" is a bit ambiguous whether it means "newly pressed" or just "current state is pressed"
        bool IsPressedState() const { return mCurrentValue; }
        bool IsUnpressedState() const { return !mCurrentValue; }
        bool IsNewlyPressed() const { return mCurrentValue && mDirty; }
        bool IsNewlyUnpressed() const { return !mCurrentValue && mDirty; }
        bool IsDirty() const { return mDirty; }
        bool WasPressedState() const { return mPreviousValue; }
        bool WasUnpressedState() const { return !mPreviousValue; }
    };

    struct EncoderState
    {
        int RawValue;
        float FloatValue; // divided by TStep
        int CommonValue;  // rounded version of FloatValue
    };

    struct IEncoder
    {
        virtual EncoderState CurrentValue() const
        {
            CCASSERT(false);
            return {};
        }
    };

    struct EncoderReader
    {
    private:
        IEncoder *mC = nullptr;
        bool mDirty = true;
        bool mFirst = true;
        EncoderState mPreviousValue;
        EncoderState mCurrentValue;

    public:
        explicit EncoderReader(IEncoder *enc) : mC(enc)
        {
        }
        EncoderReader()
        {
        }
        void SetSource(IEncoder *enc)
        {
            mC = enc;
            mFirst = true; // mark to reset everything.
        }

        void ClearState()
        {
            // resets state so that there's no dirtiness.
            mFirst = true;
            Update();
        }

        void Update()
        {
            if (mFirst)
            {
                mCurrentValue = mC->CurrentValue();
                mPreviousValue = mCurrentValue;
                mDirty = false; // since current == prev, plus we don't want everything to act like you just changed stuff at the startup.
                mFirst = false;
                //Serial.println(String("enc reader ") + ((uintptr_t)this) + " updated for the first time with raw val " + mCurrentValue.RawValue);
                return;
            }
            mPreviousValue = mCurrentValue;
            mCurrentValue = mC->CurrentValue();
            mDirty = (mPreviousValue.RawValue != mCurrentValue.RawValue);
            //Serial.println(String("enc reader ") + ((uintptr_t)this) + " updated with raw val " + mCurrentValue.RawValue + ", prev=" + mPreviousValue.RawValue);
        }

        bool IsDirty() const { return mDirty; }
        int GetIntDelta() const { return mCurrentValue.CommonValue - mPreviousValue.CommonValue; }
        float GetFloatDelta() const { return mCurrentValue.FloatValue - mPreviousValue.FloatValue; }
        int GetIntValue() const { return mCurrentValue.CommonValue; }
        float GetFloatValue() const { return mCurrentValue.FloatValue; }
    };

    struct IAnalogAxis
    {
        virtual float CurrentValue01() const = 0;
    };

    // struct NullBreathSensor : IAnalogControl
    // {
    //   virtual float CurrentValue01() const override { return 0; }
    // };

} // namespace clarinoid
