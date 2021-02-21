
#pragma once

#include <clarinoid/harmonizer/MusicalVoice.hpp>

// MIDI library is touchy about how you instantiate.
// Simplest is to do it the way it's designed for: in the main sketch, global scope.
MIDI_CREATE_INSTANCE(HardwareSerial, CLARINOID_MIDI_INTERFACE, gMidi);


namespace clarinoid
{
    const int CCEWI_MIDICHANNEL = 1;

    class CCEWIMIDIOut
    {
        CCThrottlerT<30> gCCThrottle;
        int currentPitchBendRaw = 0;
        int currentBreathCC14Bit = 0;

        // from MIDI.hpp
        static int CalcRawPitchBend(float inPitchValue)
        {
            const int scale = inPitchValue > 0.0 ? MIDI_PITCHBEND_MAX : -MIDI_PITCHBEND_MIN;
            const int value = int(inPitchValue * float(scale));
            return value;
        }

        static int Calc14BitBreath(float inBreath01)
        {
            int ret = (int)(inBreath01 * 0x3fff);
            return ret & 0x3fff;
        }
        // splits 14-bit x into separate 7-bit MSB / LSB.
        static void Breath14BitToMSBLSB(int inValue, uint8_t &outMSB, uint8_t &outLSB)
        {
            // from MIDI.hpp
            outMSB = 0x7f & (inValue >> 7);
            outLSB = 0x7f & inValue;
        }

    public:
        int noteOns = 0;

        CCEWIMIDIOut()
        {
        }

        virtual void setup()
        {
            gMidi.begin();
        }

        void Update(const MusicalVoice& liveVoice, const MusicalVoiceTransitionEvents& transitionEvents)
        {
            // send pitchbend / breath BEFORE note ons, so the note starts with the correct state.
            if (gCCThrottle.IsReady())
            {
                int pb = CalcRawPitchBend(liveVoice.mPitchBendN11.GetFloatVal());
                if (pb != currentPitchBendRaw)
                {
                    gMidi.sendPitchBend(pb, CCEWI_MIDICHANNEL);
                    currentPitchBendRaw = pb;
                }

                float breathAdj = liveVoice.mBreath01.GetFloatVal();// powf(ms.breath01.GetValue(), MIDI_BREATH_CURVE);
                int breath = Calc14BitBreath(breathAdj);

                if (breath != currentBreathCC14Bit)
                {
                    currentBreathCC14Bit = breath;
                    uint8_t msb, lsb;
                    Breath14BitToMSBLSB(breath, msb, lsb);
                    gMidi.sendControlChange(midi::MidiControlChangeNumber::BreathController, msb, CCEWI_MIDICHANNEL);
                    gMidi.sendControlChange(midi::MidiControlChangeNumber::BreathController + 32, lsb, CCEWI_MIDICHANNEL);
                }
            }

            // important: send note on before note off, to make portamento work.
            if (transitionEvents.mNeedsNoteOn)
            {
                gMidi.sendNoteOn(liveVoice.mMidiNote, liveVoice.mVelocity, CCEWI_MIDICHANNEL);
                //log(String("note on") + liveVoice.mMidiNote);
            }
            if (transitionEvents.mNeedsNoteOff)
            {
                gMidi.sendNoteOff(transitionEvents.mNoteOffNote, 0, CCEWI_MIDICHANNEL);
                //log(String("note off") + liveVoice.mMidiNote);
            }
        }
    };

} // namespace clarinoid
