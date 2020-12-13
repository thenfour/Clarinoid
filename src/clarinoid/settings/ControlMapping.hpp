
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
  // flags. consider things like "fine"
    enum class ModifierKey : uint8_t
    {
        None = 0, // requires no modifiers are pressed.
        Fine = 1,
        Course = 2,
        Shift = 4,
        Ctrl = 8,
        Any = 128, // special; any combination works.
    };

    // defines a mapping from a switch.
    struct ControlMapping
    {
        enum class Function : uint8_t
        {
            Nop,
            ModifierFine,
            ModifierCourse,
            ModifierShift,
            ModifierCtrl,
            MenuBack,
            MenuOK,
            LH1,
            LH2,
            LH3,
            LH4,
            Oct1,
            Oct2,
            Oct3,
            RH1,
            RH2,
            RH3,
            RH4,
            Breath,
            PitchBend,
            MenuScrollA,
            SynthPreset,
            Transpose,
            //Volume,
            COUNT,
        };

        enum class MapStyle : uint8_t
        {
          Passthrough,            // can be used as a "nop", or things like mapping a button input to a bool function
          RemapUnipolar,          // map the source value with {min,max} => float01
          RemapBipolar,           // map the source value with {negmin, negmax, dead max, pos min, pos max} => floatN11
          DeltaWithScale,// for encoders scrolling for example. if you just "set" the value, then it would interfere. it's more accurate like this.
          TriggerUpValue,         // when trigger up condition is met, set dest value to X.
          TriggerDownValue,         // when trigger down condition is met, set dest value to X.
          TriggerUpDownValue,     // when trigger up condition is met, set the dest value to X. when the trigger down, set to Y.
          TriggerUpValueSequence, // when trigger condition is met, set the dest value to the next value in the sequence, cycling. can be used to set up a toggle.
        };

        // specifies how aggregate values are combined, AND how it's applied to the destination parameter.
        enum class Operator : uint8_t
        {
            Set,
            Add,
            Multiply,
            COUNT,
        };

        ModifierKey mModifier = ModifierKey::Any;
        PhysicalControl mSource;
        Function mFunction = Function::Nop;
        MapStyle mStyle = MapStyle::Passthrough;
        Operator mOperator = Operator::Set;

        // trigger condition
        float mTriggerBelowValue = 0.5f;
        float mTriggerAboveValue = 0.5f;

        NPolarMapping mNPolarMapping;

        float mDeltaScale = 1.0f; // for delta operators.

        float mValueArray[MAPPED_CONTROL_SEQUENCE_LENGTH];
        size_t mValueCount = 0;

        // --> not app settings, but state stuff.
        size_t mCursor = 0;    // keeps track of the stack or sequence.
        ControlReader mReader; // some caller needs to set this when a mapping is established.

        bool IsTriggerUp()
        {
            float prev = mReader.GetPreviousFloatValue01();
            float curr = mReader.GetCurrentFloatValue01();
            if (prev < mTriggerAboveValue && curr >= mTriggerAboveValue)
            {
                //Serial.println(String("trigger up!!! prev=") + prev + ", curr=" + curr);
                return true;
            }
                //Serial.println(String("no trigger up. prev=") + prev + ", curr=" + curr);
            return false;
        }

        bool IsTriggerDown()
        {
            float prev = mReader.GetPreviousFloatValue01();
            float curr = mReader.GetCurrentFloatValue01();
            if (prev > mTriggerBelowValue && curr <= mTriggerBelowValue)
            {
                return true;
            }
            return false;
        }

        // call to update this mapping with the source control it's mapped to.
        // return whetehr the out value should be used.
        bool UpdateAndMapValue(const IControl *c, /*const ControlValue &i,*/ ControlValue &out)
        {
          mReader.Update(c);
          switch (mStyle)
            {
            default:
            case MapStyle::Passthrough:
                out = ControlValue::FloatValue(mReader.GetCurrentFloatValue01());
                return true;
            case MapStyle::RemapUnipolar: // map the source value with {min,max} => float01. breath would use this.
            {
              float f = mNPolarMapping.PerformUnipolarMapping(mReader.GetCurrentFloatValue01());
              out = ControlValue::FloatValue(f);
              return true;
            }
            case MapStyle::RemapBipolar: // map the source value with {negmin, negmax, dead max, pos min, pos max} => floatN11. think pitch bend with positive & negative regions.
            {
              float f = mNPolarMapping.PerformBipolarMapping(mReader.GetCurrentFloatValue01());
              out = ControlValue::FloatValue(f);
              return true;
            }
            case MapStyle::DeltaWithScale:
              out = ControlValue::FloatValue(this->mDeltaScale * mReader.GetFloatDelta());
              return true;
            case MapStyle::TriggerUpValue: // when trigger condition is met, set dest value to X.
              if (!IsTriggerUp())
                return false;
              out = ControlValue::FloatValue(mValueArray[0]);
              return true;
            case MapStyle::TriggerDownValue: // when trigger condition is met, set dest value to X.
              if (!IsTriggerDown())
                return false;
              out = ControlValue::FloatValue(mValueArray[0]);
              return true;
            case MapStyle::TriggerUpDownValue: // when trigger condition is met, set the dest value to X. when the trigger condition is not met, set to Y.
                if (IsTriggerUp())
                {
                    out = ControlValue::FloatValue(mValueArray[0]);
                    return true;
                }
                if (IsTriggerDown())
                {
                    out = ControlValue::FloatValue(mValueArray[1]);
                    return true;
                }
                return false;
            case MapStyle::TriggerUpValueSequence: // when trigger condition is met, set the dest value to the next value in the sequence, cycling. can be used to set up a toggle.
                if (!IsTriggerUp())
                {
                    return false;
                }
                if (mValueCount < 1)
                    return false;
                CCASSERT(mValueCount > 0);
                mCursor %= mValueCount;
                out = ControlValue::FloatValue(mValueArray[mCursor]);
                mCursor++;
                mCursor %= mValueCount;
                return true;
            }
        }

        static ControlValue ApplyValue(const ControlValue &lhs, const ControlValue &rhs, Operator op)
        {
            switch (op)
            {
            case ControlMapping::Operator::Add:
                return ControlValue::FloatValue(lhs.AsFloat01() + rhs.AsFloat01());
            case ControlMapping::Operator::Set:
                return rhs;
            case ControlMapping::Operator::Multiply:
                return ControlValue::FloatValue(lhs.AsFloat01() * rhs.AsFloat01());
            default:
                CCDIE("unsupported operator");
            }
            return {};
        }

        static ControlMapping MomentaryMapping(PhysicalControl source, Function d)
        {
            ControlMapping ret;
            ret.mSource = source;
            ret.mOperator = Operator::Add;
            ret.mStyle = MapStyle::TriggerUpDownValue; // doing it this way allows you to map multiple buttons to the same boolean thing.
            ret.mValueArray[0] = 1.0f;
            ret.mValueArray[1] = -1.0f;
            ret.mFunction = d;
            return ret;
        }

        // triggers when down pressed, adds a value to the param.
        static ControlMapping ButtonIncrementMapping(PhysicalControl source, Function fn, float delta)
        {
            ControlMapping ret;
            ret.mSource = source;
            ret.mOperator = Operator::Add;
            ret.mStyle = MapStyle::TriggerUpValue; // doing it this way allows you to map multiple buttons to the same boolean thing.
            ret.mValueArray[0] = delta;
            ret.mFunction = fn;
            return ret;
        }

        static ControlMapping TypicalEncoderMapping(PhysicalControl source, Function d)
        {
            ControlMapping ret;
            ret.mSource = source;
            ret.mOperator = Operator::Add;
            ret.mStyle = MapStyle::DeltaWithScale;
            ret.mDeltaScale = 1.0f;
            ret.mFunction = d;
            return ret;
        }

        static ControlMapping UnipolarMapping(PhysicalControl source, Function d, float srcMin, float srcMax)
        {
            ControlMapping ret;
            ret.mSource = source;
            ret.mFunction = d;
            ret.mStyle = MapStyle::RemapUnipolar;
            ret.mOperator = Operator::Set;
            ret.mNPolarMapping.mNegative = clarinoid::UnipolarMapping { srcMin, srcMax, 0.0f, 1.0f, 0.5f, 0.0f };
            return ret;
        }
    };

    //constexpr size_t aoeuuichpcuihp = sizeof(ControlMapping);

} // namespace clarinoid
