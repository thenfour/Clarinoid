// pin 23 = top led
// pin 22 = bottom led

#include <Arduino.h>
#include <MIDI.h>
#include "Stopwatch.hpp"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup()
{
    pinMode(13, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    MIDI.begin();
    // Serial.begin(400000);
}

struct Blinker
{
    clarinoid::Stopwatch sw;
    int mPeriodMS;
    explicit Blinker(int periodMS) : mPeriodMS(periodMS)
    {
    }

    bool GetState()
    {
        auto p2x = sw.ElapsedTime().ElapsedMillisI() % (mPeriodMS * 2);
        return p2x < mPeriodMS;
    }
};

struct TriggerLed
{
    clarinoid::Stopwatch sw;
    int mHoldMS;
    int mDurationMS;
    explicit TriggerLed(int holdMS, int durationMS) : mHoldMS(holdMS), mDurationMS(durationMS)
    {
    }

    void Trigger() {
      sw.Restart();
    }

    float GetState()
    {
      int elapsed = sw.ElapsedTime().ElapsedMillisI();
      if (elapsed < mHoldMS) return 1.0f;
      elapsed -= mHoldMS;
      if (elapsed >= mDurationMS) return 0.0f;
      float r = 1.0f - ((float)elapsed) / mDurationMS;
      return r * r; // steeper
    }
};

struct BigButtonReader
{
    clarinoid::Stopwatch sw; // reset each state change.
    int mPin;
    int mRelaxTimeMS;
    bool mLastValue;

    BigButtonReader(int pin, int relaxMS) : mPin(pin), mRelaxTimeMS(relaxMS)
    {
        pinMode(mPin, INPUT_PULLUP);
        mLastValue = this->LiveRead();
    }
    bool UpdateAndGetState()
    {
        if (sw.ElapsedTime().ElapsedMillisI() < mRelaxTimeMS)
            return mLastValue;
        bool newState = LiveRead();
        if (newState == mLastValue)
            return mLastValue;
        sw.Restart();
        mLastValue = newState;
        return mLastValue;
    }

    bool UpdateAndDidChangeValue() // does an internal read & update, and returns whether the value changed since last.
    {
        if (sw.ElapsedTime().ElapsedMillisI() < mRelaxTimeMS)
            return false;
        bool newState = LiveRead();
        if (newState == mLastValue)
            return false;
        sw.Restart();
        mLastValue = newState;
        return true;
    }

  private:
    bool LiveRead()
    {
        return (digitalRead(mPin) == HIGH);
    }
};

//Blinker gBlinkerTop{200};
BigButtonReader gBigButton{18, 100};
TriggerLed gIndicatorTop {30, 180};
TriggerLed gIndicatorBottom {30, 180};

clarinoid::Stopwatch gNoteOffTimer; // when a noteon happens, set a timer to send note off.
bool gWaitingForNoteOff = false;
static constexpr int gNoteOffDelayMS = 50;

void loop()
{
    if (MIDI.read())
    {
      bool care = true;
      if (MIDI.getType() == midi::ActiveSensing) care = false;
      if (care) {
        gIndicatorTop.Trigger();
      }
      //Serial.println(String("MIDI input: ") + (int) MIDI.getType());
    }

    analogWrite(23, (int)(gIndicatorTop.GetState() * 64));   // 0-256 https://www.pjrc.com/teensy/td_pulse.html

    if (gBigButton.UpdateAndDidChangeValue()) {
      gIndicatorBottom.Trigger();
      MIDI.sendNoteOn(14, 126, 1);
      gNoteOffTimer.Restart();
      gWaitingForNoteOff = true;
    }

    if (gWaitingForNoteOff && gNoteOffTimer.ElapsedTime().ElapsedMillisI() >= gNoteOffDelayMS) {
      MIDI.sendNoteOff(14, 0, 1);
      gWaitingForNoteOff = false;
    }

    analogWrite(22, (int)(gIndicatorBottom.GetState() * 64));   // 0-256 https://www.pjrc.com/teensy/td_pulse.html

    //analogWrite(22, (btn == HIGH) ? 255 : blinkerVal); // 0-256 https://www.pjrc.com/teensy/td_pulse.html

    delay(2);
}
