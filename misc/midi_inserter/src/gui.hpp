#pragma once

#include "ledstrip.hpp"
#include "anim.hpp"

// LED STRIP
// 0 = heartbeat (blue = healthy, red = error)
// 1 = status (green / red)
// 2 = identify state

enum class IdentifyState
{
    None,
    Requested,
    Sent,
};

// hardware driver which applies an animation to a pin PWM led.
template <int Tpin>
struct PinLed : IAnimationTarget
{
    IAnimation *mpCurrentAnimation = &gOffAnimation;

    PinLed()
    {
        pinMode(Tpin, OUTPUT);
        // setting analog resolution causes other problems and ugliness. don't.
        // see https://www.pjrc.com/teensy/td_pulse.html
        // on how to balance these values.
        analogWriteFrequency(Tpin, gLEDPWMFrequency);
        analogWriteResolution(gLEDPWMResolution);
    }

    virtual void IAnimationTarget_SetAnimation(IAnimation *anim) override
    {
        mpCurrentAnimation = anim;
    }

    static int _01ToAnalogLevel(float f)
    {
        return gLEDPWMMaxValue * std::pow(f, 4.5f); // gamma-correct pin LEDs
    }

    virtual void Render()
    {
        auto c = mpCurrentAnimation->IAnimation_GetColor();
        auto level01 = c.GetLevel01();
        analogWrite(Tpin, _01ToAnalogLevel(level01));
    }
};

static DMAMEM byte gWS2812_DisplayMemory[12 /*count*/ * 12 /*bytes per pixel storage*/];
Ledstrip<14, 12> gWS2812 = {gWS2812_DisplayMemory};

PinLed<2> gGreen1;
PinLed<3> gGreen2;
PinLed<4> gGreen3;
PinLed<5> gGreen4;
PinLed<6> gOrangeTriangle;
PinLed<18> gBlueTriangle;
PinLed<19> gRedTriangle;

// various animations
namespace animations
{
PulseAnimation gRedPulse = {1000};
PulseAnimation gBluePulse = {500};
PulseAnimation gOrangePulse;

PulseAnimation gHeartbeatPulse = {300};
PulseAnimation gErrorPulse = {30000};
SolidColor gNoErrorSolid;
SolidColor gLed4Solid; // not used at the moment.

SolidColor gBlueSolid;
SolidColor gOrangeSolid;

PulseAnimation gTXA = {100};
PulseAnimation gTXB = {100};
PulseAnimation gRXA = {100};
PulseAnimation gRXB = {100};

SolidColor gIdentifyStateSolid;

SolidColor gWS2812Solids[12];

PulseAnimation gBeatTrigger = {150};

} // namespace animations

struct MidiPerformerGui
{
    clarinoid::Stopwatch mGuiTimer; // don't update leds EVERY loop; update max 60fps
    static constexpr int gGuiIntervalMS = 1000 / gFrameRate;

    int mFrame = 0;

    clarinoid::Stopwatch mHeartbeatTimer;
    static constexpr int gHeartbeatIntervalMS = 500;

    MidiPerformerGui()
    {
        animations::gHeartbeatPulse.mForeColor = {0.1f, 0.1f, 0.5f};
        animations::gOrangePulse.mForeColor.SetLevel01(0.5f);
        animations::gBluePulse.mForeColor.SetLevel01(0.5f);

        animations::gNoErrorSolid.mColor = {0.1f, 0.1f, 0.5f};
        gWS2812.mLeds[1].IAnimationTarget_SetAnimation(&animations::gNoErrorSolid);

        animations::gLed4Solid.mColor = {0.1f, 0.1f, 0.5f};
        gWS2812.mLeds[3].IAnimationTarget_SetAnimation(&animations::gLed4Solid);

        animations::gTXA.mForeColor.SetLevel01(0.33f);
        animations::gTXB.mForeColor.SetLevel01(0.33f);
        animations::gRXA.mForeColor.SetLevel01(0.33f);
        animations::gRXB.mForeColor.SetLevel01(0.33f);

        animations::gBeatTrigger.mForeColor.SetLevel01(0.55f);
    }

    void OnFrame()
    {
        if (mGuiTimer.ElapsedTime().ElapsedMillisI() < gGuiIntervalMS)
        {
            return;
        }

        mFrame++;
        mGuiTimer.Restart();

        if (mHeartbeatTimer.ElapsedTime().ElapsedMillisI() >= gHeartbeatIntervalMS)
        {
            mHeartbeatTimer.Restart();
            gWS2812.mLeds[0].IAnimationTarget_SetAnimation(&animations::gHeartbeatPulse);
            animations::gHeartbeatPulse.Trigger();
        }

        gGreen1.Render();
        gGreen2.Render();
        gGreen3.Render();
        gGreen4.Render();
        gOrangeTriangle.Render();
        gBlueTriangle.Render();
        gRedTriangle.Render();

        gWS2812.Render();
    }

    void SetIdentifyState(IdentifyState state)
    {
        switch (state)
        {
        default:
        case IdentifyState::None:
            animations::gIdentifyStateSolid.mColor = {};
            break;
        case IdentifyState::Requested:
            animations::gIdentifyStateSolid.mColor = {0.4f, 0.4f, 0.0f};
            break;
        case IdentifyState::Sent:
            animations::gIdentifyStateSolid.mColor = {0.0f, 0.0f, 0.7f};
            break;
        }
        gWS2812.mLeds[2].IAnimationTarget_SetAnimation(&animations::gIdentifyStateSolid);
    }

    void OnMidiARXTX()
    {
        animations::gRXA.Trigger();
        gGreen1.IAnimationTarget_SetAnimation(&animations::gRXA);
        OnMidiATX();
    }

    void OnMidiBRXTX()
    {
        animations::gRXB.Trigger();
        gGreen3.IAnimationTarget_SetAnimation(&animations::gRXB);
        OnMidiBTX();
    }

    void OnMidiABTX()
    {
        OnMidiBTX();
        OnMidiATX();
    }

    void OnMidiATX()
    {
        animations::gTXA.Trigger();
        gGreen4.IAnimationTarget_SetAnimation(&animations::gTXA);
    }

    void OnMidiBTX()
    {
        animations::gTXB.Trigger();
        gGreen2.IAnimationTarget_SetAnimation(&animations::gTXB);
    }

    void OnBigButtonSendNoteViaBusB()
    {
        gRedTriangle.IAnimationTarget_SetAnimation(&animations::gRedPulse);
        animations::gRedPulse.Trigger();
    }
    void OnError()
    {
        animations::gErrorPulse.Trigger();
        animations::gErrorPulse.mForeColor = {0.5f, 0.0f, 0.0f};
        gWS2812.mLeds[1].IAnimationTarget_SetAnimation(&animations::gErrorPulse);
    }
    void SetLedStripColor(int nled, const Color &c)
    {
        if (nled < 0)
        {
            OnError();
            return;
        }
        nled += 4; // skip the ones for system use
        if (nled >= 12)
        {
            OnError();
            return;
        }

        animations::gWS2812Solids[nled].mColor = c;
        gWS2812.mLeds[nled].IAnimationTarget_SetAnimation(&animations::gWS2812Solids[nled]);
    }
    void OnSetBlueLevel(float level01)
    {
        animations::gBlueSolid.mColor.SetLevel01(level01);
        gBlueTriangle.IAnimationTarget_SetAnimation(&animations::gBlueSolid);
    }
    void OnSetOrangeLevel(float level01)
    {
        animations::gOrangeSolid.mColor.SetLevel01(level01);
        gOrangeTriangle.IAnimationTarget_SetAnimation(&animations::gOrangeSolid);
    }

    void OnBeatTrigger()
    {
        animations::gBeatTrigger.Trigger();
        gOrangeTriangle.IAnimationTarget_SetAnimation(&animations::gBeatTrigger);
    }
};
