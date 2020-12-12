
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

    // flags. consider things like "fine"
    enum class ModifierKey : uint8_t
    {
        None = 0,
        Fine = 1,
        Course = 2,
        Shift = 4,
        Ctrl = 8,
    };

    // defines a mapping from a switch.
    struct ControlMapping
    {
        enum class MapStyle : uint8_t
        {
            Passthrough,            // can be used as a "nop", or things like mapping a button input to a bool function
            RemapUnipolar,          // map the source value with {min,max} => float01
            RemapBipolar,           // map the source value with {negmin, negmax, dead max, pos min, pos max} => floatN11
            TriggerUpValue,         // when trigger up condition is met, set dest value to X.
            TriggerUpDownValue,     // when trigger up condition is met, set the dest value to X. when the trigger down, set to Y.
            TriggerUpValueSequence, // when trigger condition is met, set the dest value to the next value in the sequence, cycling. can be used to set up a toggle.
        };

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
            MenuScrollA,
            COUNT,
        };

        enum class Operator : uint8_t
        {
            Set,
            Add,
            Multiply,
            COUNT,
        };

        ModifierKey mModifier = ModifierKey::None;
        PhysicalControl mSource;
        Function mFunction = Function::Nop;
        MapStyle mStyle = MapStyle::Passthrough;
        Operator mOperator = Operator::Set;

        // trigger condition
        float mTriggerBelowValue = 0.5f;
        float mTriggerAboveValue = 0.5f;

        // for two-polar. like joystick X or pitch bend strip,
        // these are 0-1 source values, but we want to map them into -1 to 1 float values.
        // instead of the 1 region of unipolar defined by {min,max}
        // we have 3 regions (negative, zero, positive), defined by the boundaries.
        // looking at the whole source range,
        // |---------------------------------------------------------------|
        //    |neg_min------neg_max|-----|pos_min----------pos_max|

        struct MapRegion
        {
            float mSrcMin;
            float mSrcMax;
            float mDestMin;
            float mDestMax;
            float mCurveP;
            float mCurveS;
        };

        MapRegion mNegRegion;
        MapRegion mPosRegion; // use this for unipolar.

        float mValueArray[MAPPED_CONTROL_SEQUENCE_LENGTH];
        size_t mValueCount = 0;

        // --> not app settings, but state stuff.
        size_t mCursor = 0;    // keeps track of the stack or sequence.
        ControlReader mReader; // some caller needs to set this when a mapping is established.

        void UpdateValue(const IControl *c)
        {
            mReader.Update(c);
        }

        bool IsTriggerUp()
        {
            float prev = mReader.GetPreviousFloatValue01();
            float curr = mReader.GetCurrentFloatValue01();
            if (prev < mTriggerAboveValue && curr >= mTriggerAboveValue)
            {
                return true;
            }
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

        // return whetehr the out value should be used.
        bool MapValue(const ControlValue &i, ControlValue &out)
        {
            switch (mStyle)
            {
            default:
            case MapStyle::Passthrough:
                out = i;
                return true;
            case MapStyle::RemapUnipolar: // map the source value with {min,max} => float01. breath would use this.
                // todo
                out = i;
                return true;
            case MapStyle::RemapBipolar: // map the source value with {negmin, negmax, dead max, pos min, pos max} => floatN11. think pitch bend with positive & negative regions.
                // todo
                out = i;
                return true;
            case MapStyle::TriggerUpValue: // when trigger condition is met, set dest value to X.
                if (!IsTriggerUp())
                    return false;
                out = i;
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
            __builtin_unreachable();
        }

        static ControlMapping MomentaryMapping(PhysicalControl source, Function d)
        {
            ControlMapping ret;
            ret.mSource = source;
            ret.mStyle = MapStyle::Passthrough;
            ret.mFunction = d;
            return ret;
        }
        static ControlMapping MenuScrollMapping(PhysicalControl source, Function d)
        {
            ControlMapping ret;
            ret.mSource = source;
            ret.mFunction = d;
            return ret;
        }
        static ControlMapping BreathMapping(PhysicalControl source, Function d)
        {
            ControlMapping ret;
            ret.mSource = source;
            ret.mFunction = d;
            return ret;
        }
    };

} // namespace clarinoid
