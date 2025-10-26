
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

// these should be flags someday but for now no need.
enum class ModifierKey : uint8_t
{
    None = 0, // requires no modifiers are pressed.
    Fine = 1,
    Course = 2,

    Synth = 3,
    Perf = 4,
    Harm = 5,
    Shift = 6,
    Transpose = 7,
    Key = 8,
    Tempo = 9,
    Any = 127, // special; any combination works.
};

// defines a mapping from a switch.
struct ControlMapping
{
    static constexpr int kDefaultDoubleClickWindowMS = 325;

    enum class Function : uint8_t
    {
        Nop,
        ModifierFine,
        ModifierCourse,
        ModifierSynth,
        ModifierPerf,
        ModifierHarm,
        ModifierShift,
        ModifierTranspose,
        ModifierKey,
        ModifierTempo,
        MenuBack,
        DisplayFontToggle,
        MenuOK,
        LH1,
        LH2,
        LH3,
        LH4,
        Oct1,
        Oct2,
        Oct3,
        Oct4,
        Oct5,
        Oct6,
        RH1,
        RH2,
        RH3,
        RH4,
        Breath,
        PitchBend,
        MenuScrollA,
        SynthPresetA,
        SynthPresetB,
        HarmPreset,
        Transpose,
        TransposeA,
        TransposeB,
        TransposeReset,
        GlobalKeyRoot,
        GlobalKeyFlavor,
        GlobalTempo,
        PerfPreset,
        EffectsEnabledToggle,
        GlobalScaleDeducedToggle,
        LoopGo,
        LoopStop,
        BaseNoteHoldToggle,
        MetronomeLEDToggle,
        MetronomeToggle,
        HarmPresetOnOffToggle,
        SoftResetMpr121,
        COUNT,
    };

    enum class MapStyle : uint8_t
    {
        Passthrough,        // can be used as a "nop", or things like mapping a button input to a bool function
        RemapUnipolar,      // map the source value with {min,max} => float01
        DeltaWithScale,     // for encoders scrolling for example. if you just "set" the value, then it would interfere.
                            // it's more accurate like this.
        TriggerUpValue,     // when trigger up condition is met, set dest value to X.
        TriggerDownValue,   // when trigger down condition is met, set dest value to X.
        TriggerUpDownValue, // when trigger up condition is met, set the dest value to X. when the trigger down, set to
                            // Y.
        TriggerUpValueSequence, // when trigger condition is met, set the dest value to the next value in the sequence,
                                // cycling. can be used to set up a toggle.
    };

    // specifies how aggregate values are combined, AND how it's applied to the destination parameter.
    enum class Operator : uint8_t
    {
        Set,
        Add,
        Subtract,
        Multiply,
        COUNT,
    };

    enum class Activation : uint8_t
    {
        SinglePress,
        DoublePress,
    };

    // 1.idle  2.primed....3.active
    // ________        ____    ____
    //         |______|    |__|
    // 1. idle
    // 2. primed (first downpress detected, start timer)
    // 3. active (second downpress detected within time window)
    // the triggerup/down logic remains; basically double press removes the first pair of triggers (so triggerdown -> up
    // -> down becomes just the 2nd down.)
    struct DoubleClickLogic
    {
        enum class State : uint8_t
        {
            Idle,
            WaitingSecondPress,
            Activated,
            Cancelled,
        };

        StopwatchLight mStopwatch;
        State mState = State::Idle;

        void Update(bool &triggerUp, bool &triggerDown, bool pressIsTriggerUp, bool isPressedNow, int timeoutMS)
        {
            bool pressEdge = pressIsTriggerUp ? triggerUp : triggerDown;
            bool releaseEdge = pressIsTriggerUp ? triggerDown : triggerUp;

            switch (mState)
            {
            case State::Idle:
                if (pressEdge)
                {
                    mState = State::WaitingSecondPress;
                    mStopwatch.Restart();
                    triggerUp = false;
                    triggerDown = false;
                }
                else if (releaseEdge)
                {
                    triggerUp = false;
                    triggerDown = false;
                }
                break;

            case State::WaitingSecondPress: {
                auto elapsed = mStopwatch.ElapsedTime().ElapsedMillisI();
                if (pressEdge)
                {
                    if (elapsed <= timeoutMS)
                    {
                        mState = State::Activated;
                        if (pressIsTriggerUp)
                        {
                            triggerDown = false;
                        }
                        else
                        {
                            triggerUp = false;
                        }
                    }
                    else
                    {
                        mStopwatch.Restart();
                        triggerUp = false;
                        triggerDown = false;
                    }
                }
                else
                {
                    if (releaseEdge)
                    {
                        triggerUp = false;
                        triggerDown = false;
                    }
                    if (elapsed > timeoutMS)
                    {
                        mState = isPressedNow ? State::Cancelled : State::Idle;
                    }
                }
                break;
            }

            case State::Activated:
                if (releaseEdge)
                {
                    mState = State::Idle;
                }
                else if (pressEdge)
                {
                    if (pressIsTriggerUp)
                    {
                        triggerUp = false;
                    }
                    else
                    {
                        triggerDown = false;
                    }
                }
                break;

            case State::Cancelled:
                if (!isPressedNow)
                {
                    if (releaseEdge)
                    {
                        triggerUp = false;
                        triggerDown = false;
                    }
                    mState = State::Idle;
                }
                else if (releaseEdge)
                {
                    triggerUp = false;
                    triggerDown = false;
                }
                else if (pressEdge)
                {
                    triggerUp = false;
                    triggerDown = false;
                }
                break;
            }
        }
    };

    ModifierKey mModifier = ModifierKey::Any;
    PhysicalControl mSource;
    Function mFunction = Function::Nop;
    MapStyle mStyle = MapStyle::Passthrough;
    Operator mOperator = Operator::Set;
    Activation mActivation = Activation::SinglePress;
    DoubleClickLogic mDoubleClickLogic;

    // trigger condition
    static constexpr float mTriggerBelowValue = 0.5f;
    static constexpr float mTriggerAboveValue = 0.5f;
    static constexpr float mDeltaScale = 1.0f; // for delta operators.

    UnipolarMapping mUnipolarMapping;

    float mValueArray[MAPPED_CONTROL_SEQUENCE_LENGTH];
    size_t mValueCount = 0;

    // --> not app settings, but state stuff.
    size_t mCursor = 0;    // keeps track of the stack or sequence.
    ControlReader mReader; // some caller needs to set this when a mapping is established.
    // DoubleClickState mDoubleClickState = DoubleClickState::Idle;
    //  int mDoubleClickWindowMS = kDefaultDoubleClickWindowMS;
    // TimeSpan mFirstClickTime = TimeSpan::Zero();

    bool IsTriggerUp()
    {
        float prev = mReader.GetPreviousFloatValue01();
        float curr = mReader.GetCurrentFloatValue01();
        if (prev < mTriggerAboveValue && curr >= mTriggerAboveValue)
        {
            // Serial.println(String("trigger up!!! prev=") + prev + ", curr=" + curr);
            return true;
        }
        // Serial.println(String("no trigger up. prev=") + prev + ", curr=" + curr);
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
        // rising / falling edge detection
        bool triggerUp = IsTriggerUp();
        bool triggerDown = IsTriggerDown();

        if (mActivation == Activation::DoublePress)
        {
            mDoubleClickLogic.Update(triggerUp,
                                     triggerDown,
                                     PressUsesTriggerUp(),
                                     mReader.IsCurrentlyPressed(),
                                     kDefaultDoubleClickWindowMS);
        }

        switch (mStyle)
        {
        default:
        case MapStyle::Passthrough:
            out = ControlValue::FloatValue(mReader.GetCurrentFloatValue01());
            return true;
        case MapStyle::RemapUnipolar: // map the source value with {min,max} => float01. breath would use this.
        {
            float f = mUnipolarMapping.PerformMapping(mReader.GetCurrentFloatValue01());
            out = ControlValue::FloatValue(f);
            return true;
        }
        // case MapStyle::RemapBipolar: // map the source value with {negmin, negmax, dead max, pos min, pos max} =>
        // floatN11. think pitch bend with positive & negative regions.
        // {
        //   float f = mNPolarMapping.PerformBipolarMapping(mReader.GetCurrentFloatValue01());
        //   out = ControlValue::FloatValue(f);
        //   return true;
        // }
        case MapStyle::DeltaWithScale:
            out = ControlValue::FloatValue(this->mDeltaScale * mReader.GetFloatDelta());
            return true;
        case MapStyle::TriggerUpValue: // when trigger condition is met, set dest value to X.
            if (!triggerUp)
                return false;
            out = ControlValue::FloatValue(mValueArray[0]);
            return true;
        case MapStyle::TriggerDownValue: // when trigger condition is met, set dest value to X.
            if (!triggerDown)
                return false;
            out = ControlValue::FloatValue(mValueArray[0]);
            return true;
        case MapStyle::TriggerUpDownValue: // when trigger condition is met, set the dest value to X. when the trigger
                                           // condition is not met, set to Y.
            if (triggerUp)
            {
                out = ControlValue::FloatValue(mValueArray[0]);
                return true;
            }
            if (triggerDown)
            {
                out = ControlValue::FloatValue(mValueArray[1]);
                return true;
            }
            return false;
        case MapStyle::TriggerUpValueSequence: // when trigger condition is met, set the dest value to the next value in
                                               // the sequence, cycling. can be used to set up a toggle.
            if (!triggerUp)
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

    // if this is the first operand, then lhs will be null.
    static ControlValue ApplyValue(const ControlValue *lhs, const ControlValue &rhs, Operator op)
    {
        if (!lhs)
        {
            // this makes sense for all operators so far except maybe Multiply or Subtract
            return rhs;
        }
        switch (op)
        {
        case ControlMapping::Operator::Add:
            return ControlValue::FloatValue(lhs->AsFloat01() + rhs.AsFloat01());
        case ControlMapping::Operator::Subtract:
            return ControlValue::FloatValue(lhs->AsFloat01() - rhs.AsFloat01());
        case ControlMapping::Operator::Set:
            return rhs;
        case ControlMapping::Operator::Multiply:
            return ControlValue::FloatValue(lhs->AsFloat01() * rhs.AsFloat01());
        default:
            CCDIE("unsupported operator");
        }
        return {};
    }

    static ControlMapping MomentaryMapping(PhysicalControl source,
                                           Function d,
                                           ModifierKey mod = ModifierKey::Any,
                                           ModifierKey mod2 = ModifierKey::Any,
                                           Activation activation = Activation::SinglePress)
    {
        ControlMapping ret;
        ret.mSource = source;
        ret.mOperator = Operator::Add;
        ret.mStyle = MapStyle::TriggerUpDownValue; // doing it this way allows you to map multiple buttons to the same
                                                   // boolean thing.
        ret.mValueArray[0] = 1.0f;
        ret.mValueArray[1] = -1.0f;
        ret.mFunction = d;
        ret.mModifier = mod;
        ret.mActivation = activation;
        return ret;
    }

    // for touch keys, when state gets messed up, the TriggerUpDownValue will never reset.
    // this simply passes through the value, so it's always in sync with the touch key state,
    // but you lose the ability to stack mappings (which would be unreasonable anyway)
    static ControlMapping UniqueMomentaryMapping(PhysicalControl source, Function d)
    {
        ControlMapping ret;
        ret.mSource = source;
        ret.mFunction = d;
        ret.mOperator = Operator::Set;
        ret.mStyle = MapStyle::Passthrough;
        return ret;
    }

    // triggers when down pressed, adds a value to the param.
    static ControlMapping ButtonIncrementMapping(PhysicalControl source,
                                                 Function fn,
                                                 float delta,
                                                 ModifierKey mod = ModifierKey::Any,
                                                 Activation activation = Activation::SinglePress)
    {
        ControlMapping ret;
        ret.mSource = source;
        ret.mOperator = Operator::Add;
        ret.mStyle =
            MapStyle::TriggerUpValue; // doing it this way allows you to map multiple buttons to the same boolean thing.
        ret.mValueArray[0] = delta;
        ret.mFunction = fn;
        ret.mModifier = mod;
        ret.mActivation = activation;
        return ret;
    }

    static ControlMapping TypicalEncoderMapping(PhysicalControl source, Function d, ModifierKey mod = ModifierKey::Any)
    {
        ControlMapping ret;
        ret.mSource = source;
        ret.mOperator = Operator::Add;
        ret.mStyle = MapStyle::DeltaWithScale;
        // ret.mDeltaScale = 1.0f;
        ret.mFunction = d;
        return ret;
    }

    static ControlMapping MakeUnipolarMapping(PhysicalControl source,
                                              Function d,
                                              float srcMin,
                                              float srcMax,
                                              float destMin = 0.0f,
                                              float destMax = 1.0f)
    {
        ControlMapping ret;
        ret.mSource = source;
        ret.mFunction = d;
        ret.mStyle = MapStyle::RemapUnipolar;
        ret.mOperator = Operator::Set;
        ret.mUnipolarMapping = clarinoid::UnipolarMapping{srcMin, srcMax, destMin, destMax, 0.5f, 0.0f};
        return ret;
    }

  private:
    bool PressUsesTriggerUp() const
    {
        switch (mStyle)
        {
        case MapStyle::TriggerDownValue:
            return false;
        default:
            return true;
        }
    }
};

constexpr size_t aoeuuichpcuihp = sizeof(ControlMapping);

} // namespace clarinoid
