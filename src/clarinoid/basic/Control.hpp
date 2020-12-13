
#pragma once

namespace clarinoid
{
    struct ControlValueDatatype
    {
        enum class Type : uint8_t
        {
            Bool,
            Int,
            Enum,
            Float01,  // unipolar float with 0-1 bounds.
            FloatN11, // bipolar float -1 to 1 bounds.
        };

        Type mDatatype = Type::Float01;
        GenericEnumItemInfo *mEnumInfo = nullptr;
    };
    // a control mapping will output a value. this value.
    // the point is to be able to map any control to any function/destination. so it means we need a variant of sorts
    // that all mappable controls and functions can understand and work with. it's by doing this that we could assign
    // a potentiometer to octave transposition, or a push button to master tuning or something.
    struct ControlValue
    {
    private:
        float mVal = 0.0f;

    public:
        // bool Equals(int n) const // there are different ways to treat int, so don't expose this.
        // {
        //     return FloatRoundedEqualsInt(mVal, n);
        // }
        bool Equals(bool n) const
        {
          if (n) {
            return !FloatEquals(mVal, 0.0f);
          }
          return FloatEquals(mVal, 0.0f);
        }
        bool Equals(float f) const
        {
            return FloatEquals(mVal, f, 0.0001f); // seems reasonable. a 16-bit input would be a precision of .0001525902189669642
        }
        bool Equals(const ControlValue &v)
        {
            return Equals(v.mVal);
        }

        bool AsBool() const
        {
            return Equals(true);
        }
        int AsRoundedInt() const
        {
            return FloatRoundToInt(mVal);
        }
        int AsFlooredInt() const
        {
            return (int)::floorf(mVal);
        }
        float AsFloat01() const
        {
            return mVal;
        }

        ControlValue& operator =(bool b) { mVal = b ? 1.0f : 0.0f; return *this; }

        static ControlValue BoolValue(bool b)
        {
            ControlValue ret;
            ret.mVal = b ? 1.0f : 0.0f;
            return ret;
        }
        static ControlValue IntValue(int n)
        {
            ControlValue ret;
            ret.mVal = (float)n;
            return ret;
        }
        static ControlValue FloatValue(float f)
        {
            ControlValue ret;
            ret.mVal = f;
            return ret;
        }

    };

    struct IControl
    {
        virtual ControlValue GetControlValue() const = 0;
    };

    // generic control reader, for use by the control mapping system to determine triggers for example.
    struct ControlReader
    {
    private:
        const IControl *mC = nullptr;
        bool mDirty = false;
        ControlValue mPreviousValue;
        ControlValue mCurrentValue;

    public:
      void Reset()
      {
        mC = nullptr;
      }
        // Each call to Update() frames Dirty / Previous / Current
        void Update(const IControl *c)
        {
            if (mC != c)
            {
                // first time seeing this control.
                mC = c;
                mCurrentValue = mC->GetControlValue();
                mPreviousValue = mCurrentValue;
                mDirty = false;
                return;
            }

            mPreviousValue = mCurrentValue;
            mCurrentValue = mC->GetControlValue();
            mDirty = mPreviousValue.Equals(mCurrentValue);
        }

        // assuming it's a switch-like control,
        bool IsCurrentlyPressed() const { return mCurrentValue.AsBool(); }
        bool IsCurrentlyUnpressed() const { return !IsCurrentlyPressed(); }
        bool IsNewlyPressed() const { return mCurrentValue.AsBool() && mDirty; }
        bool IsNewlyUnpressed() const { return !mCurrentValue.AsBool() && mDirty; }
        bool IsDirty() const { return mDirty; }

        // assuming it's an axis-like control,
        float GetCurrentFloatValue01() const { return mCurrentValue.AsFloat01(); }
        float GetPreviousFloatValue01() const { return mPreviousValue.AsFloat01(); }

        // assuming encoder-like,
        int GetIntDelta() const { return mCurrentValue.AsFlooredInt() - mPreviousValue.AsFlooredInt(); }
        float GetFloatDelta() const { return mCurrentValue.AsFloat01() - mPreviousValue.AsFloat01(); }
        int GetIntValue() const { return mCurrentValue.AsFlooredInt(); }
        float GetFloatValue() const { return mCurrentValue.AsFloat01(); }
    };

    struct ISwitch : IControl
    {
      virtual bool CurrentValue() const = 0;
        virtual ControlValue GetControlValue() const override
        {
            return ControlValue::BoolValue(CurrentValue());
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

    public:
        // Each call to Update() frames Dirty / Previous / Current
        void Update(ISwitch *c)
        {
          if (mC != c)
          {
            mC = c;
            mCurrentValue = mC->CurrentValue();
            mPreviousValue = mCurrentValue;
            mDirty = false;
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

    struct IEncoder : IControl
    {
        virtual float CurrentValue() const = 0;
        virtual ControlValue GetControlValue() const
        {
            return ControlValue::FloatValue(CurrentValue());
        }
    };

    struct EncoderReader
    {
    private:
        IEncoder *mC = nullptr;
        bool mDirty = true;
        ControlValue mPreviousValue;
        ControlValue mCurrentValue;

    public:
        void ClearState()
        {
            mC = nullptr;
            // resets state so that there's no dirtiness.
            // mFirst = true;
            // Update();
        }

        void Update(IEncoder *enc)
        {
            if (mC != enc)
            {
                mC = enc;
                mCurrentValue = mC->GetControlValue();
                mPreviousValue = mCurrentValue;
                mDirty = false; // since current == prev, plus we don't want everything to act like you just changed stuff at the startup.
                //mFirst = false;
                return;
            }
            mPreviousValue = mCurrentValue;
            mCurrentValue = mC->GetControlValue();
            mDirty = mPreviousValue.Equals(mCurrentValue);
        }

        bool IsDirty() const { return mDirty; }
        int GetIntDelta() const { return mCurrentValue.AsFlooredInt() - mPreviousValue.AsFlooredInt(); }
        float GetFloatDelta() const { return mCurrentValue.AsFloat01() - mPreviousValue.AsFloat01(); }
        int GetIntValue() const { return mCurrentValue.AsFlooredInt(); }
        float GetFloatValue() const { return mCurrentValue.AsFloat01(); }
    };

    struct IAnalogAxis : IControl
    {
        virtual float CurrentValue01() const = 0;
        virtual ControlValue GetControlValue() const
        {
            return ControlValue::FloatValue(CurrentValue01());
        }
    };

} // namespace clarinoid
