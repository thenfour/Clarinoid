#pragma once
#include "ledstrip.hpp"

enum class IdentifyState
{
    None,
    Requested,
    Sent,
};

struct Color
{
    byte R = 0;
    byte G = 0;
    byte B = 0;
    // byte level; <-- always just returns R.

    bool Equals(const Color &rhs)
    {
        if (rhs.R != this->R)
            return false;
        if (rhs.G != this->G)
            return false;
        if (rhs.B != this->B)
            return false;
        return true;
    }

    byte GetLevel()
    {
        return R;
    }

    Color &operator=(const Color &rhs) = default;

    static Color Black;
};

// represents a signal curve over time, for RGB colors
struct IAnimation
{
    virtual Color IAnimation_GetColor() = 0;
};

struct OffAnimation : IAnimation
{
    Color IAnimation_GetColor() override
    {
        return {0, 0, 0};
    }
};

OffAnimation gOffAnimation;

struct SolidColor : IAnimation
{
    Color IAnimation_GetColor() override
    {
        return {0, 0, 0};
    }
};

struct TriggerAnim : IAnimation
{
};

// struct LedPin
// {
//     const IAnimation *mCurrentAnimation = &gOff;

//     // because i avoid dynamic allocation, each led should have its own copy of all the animations it will need.
//     // so OrangeLed will have its own trigger animation object it can trigger with etc.
//     // exception: immutable animation types like Off.
//     // then to trigger the OrangeLed, call .Trigger on its trigger animation object.
//     void SetAnimation(const IAnimation *anim)
//     {
//         mCurrentAnimation = anim;
//     }
// };

// enum class PinLeds
// {
//   Green1 = 3,
//   Green2 = 4,
//   Green3 = 23,
//   Green4 = 22,
//   Orange = 17,
//   Blue = 16,
//   Red = 20,
// };

struct ILed
{
    virtual void SetColor(Color c) = 0;
    //virtual void SetAnimation(IAnimation* anim) = 0;
    virtual void Render() = 0;
};

struct PinLed : ILed
{
    int mPin;
    Color mCurrentColor;
    bool mDirty;

    PinLed(int pin) : mPin(pin), mCurrentColor(Color::Black), mDirty(true)
    {
        pinMode(mPin, OUTPUT);
        //         // setting analog resolution causes other problems and ugliness. don't.
        //         // see https://www.pjrc.com/teensy/td_pulse.html
        //         // on how to balance these values.
        //         // analogWriteResolution(10); // now analogWrite is 0 - 1023
        //         // analogWriteFrequency(pin, 46875);
    }

    virtual void SetColor(Color c) override
    {
        if (mCurrentColor.Equals(c))
            return;
        mCurrentColor = c;
        mDirty = true;
    }

    int GetAnalogLevel(double f)
    {
        // gamma correct; try to make these things a bit more linear.
        // return clamp((int)(f * f * f * f * 255), 0, 255);
    }

    virtual void Render() override
    {
        if (!mDirty)
            return;
        mDirty = false;
        // analogWrite(mPin, GetAnalogLevel(f));
        analogWrite(mPin, mCurrentColor.GetLevel());
    }

    //
};

// enum class Leds
// {
// };

// struct MidiPerformerGui
// {
//     clarinoid::Stopwatch gGuiTimer; // don't update leds EVERY loop; update max 60fps
//     static constexpr int gGuiIntervalMS = 1000 / 60;

//     // LEDPin gLedPin_OrangeTriangle{17, 80, 240, 1.0, 100};
//     // LEDPin gLedPin_BlueTriangle{16, 80, 240, 1.0, 100};
//     // LEDPin gLedPin_RedTriangle{20, 240, 3000, 1.0, 100};
//     // LEDPin gLedPin_MidiA_RX{3, 30, 360, 0.05, 200};
//     // LEDPin gLedPin_MidiB_TX{4, 30, 360, 0.05, 200};
//     // LEDPin gLedPin_MidiB_RX{23, 30, 360, 0.05, 200};
//     // LEDPin gLedPin_MidiA_TX{22, 30, 360, 0.05, 200};

//     Ledstrip gLedstrip;

//     void Render()
//     {
//         // process heartbeat
//         // process identify state

//         // gLedPin_MidiA_RX.Present();
//         // gLedPin_MidiA_TX.Present();
//         // gLedPin_MidiB_RX.Present();
//         // gLedPin_MidiB_TX.Present();
//         // gLedPin_RedTriangle.Present();

//         // if (doOrangeAndBlue)
//         // {
//         //     gLedPin_BlueTriangle.Present();
//         //     gLedPin_OrangeTriangle.Present();
//         // }

//         // gLedstrip.Render();

//         // if (gGuiTimer.ElapsedTime().ElapsedMillisI() >= gGuiIntervalMS)
//         // {
//         //     PresentLeds();
//         // }
//     }

//     void SetIdentifyState(IdentifyState state)
//     {
//         //
//     }

//     // void TriggerMidiATX() {}
//     // void TriggerMidiARX() {}
//     // void TriggerMidiBTX() {}
//     // void TriggerMidiBRX() {}

//     // void TriggerOrange() {}
//     // void TriggerBlue() {}
//     // void TriggerRed() {}
// };
