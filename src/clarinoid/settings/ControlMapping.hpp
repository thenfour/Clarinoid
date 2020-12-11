
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{


// flags. consider things like "fine"
enum class ModifierKey : uint8_t
{
    None = 0,
    Ctrl = 0b001,
    Shift = 0b010,
    ShiftCtrl = 0b011,
    Alt = 0b100,
    AltCtrl = 0b101,
    AltShift = 0b110,
    AltCtrlShift = 0b111,
    COUNT = 8,
};

struct ButtonMapping
{
    enum class Behavior : uint8_t
    {
        Nop,
        Momentary, // 
        //Latching,
        //PushValue,
        //PopValue,
        //CycleValueList,
        //AddValue,
    };

    enum class Destination : uint8_t
    {
        Nop,
        //Ctrl,
        //Shift,
        //Alt,
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
        COUNT,
    };

    ModifierKey mModifier = ModifierKey::None;
    PhysicalSwitch mSource;
    Behavior mSourceBehavior = Behavior::Nop;

    Destination mDestination = Destination::Nop;
    // stack<10>
    // value list
    //Param mParam;

    static ButtonMapping MomentaryMapping(PhysicalSwitch source, Destination d)
    {
        ButtonMapping ret;
        ret.mSource = source;
        ret.mDestination = d;
        ret.mSourceBehavior = Behavior::Momentary;
        return ret;
    }
};

struct AxisMapping
{
    enum class Behavior : uint8_t
    {
        Nop,
        Unipolar,
        //Bipolar,
    };

    enum class Destination : uint8_t
    {
        Nop,
        Breath,
        //PitchBend,
        //Modulation,
        COUNT,
    };

    ModifierKey mModifier = ModifierKey::None;
    PhysicalAxis mSource;
    Behavior mSourceBehavior = Behavior::Nop;
    Destination mDestination = Destination::Nop; // destination behavior is defined by the destination
    
    // unipolar source calib
    // bipolar source calib
    // dest mapping details?

    static AxisMapping SimpleMapping(PhysicalAxis src, Destination dest)
    {
        AxisMapping ret;
        ret.mSource = src;
        ret.mDestination = dest;
        ret.mSourceBehavior = Behavior::Unipolar;
        return ret;
    }
};

struct EncoderMapping
{
    enum class Destination : uint8_t
    {
        Nop,
        MenuScroll,
        //HarmonizerPreset,
        //SynthPreset,
        //BPM,
        COUNT,
    };

    ModifierKey mModifier = ModifierKey::None;
    PhysicalEncoder mSource;
    Destination mDestination = Destination::Nop; // destination behavior is defined by the destination
    float mScale = 1.0f;
    
    // unipolar source calib
    // bipolar source calib
    // dest mapping details?

    static EncoderMapping SimpleMapping(PhysicalEncoder src, Destination dest)
    {
        EncoderMapping ret;
        ret.mSource = src;
        ret.mDestination = dest;
        return ret;
    }
};



} // namespace clarinoid

