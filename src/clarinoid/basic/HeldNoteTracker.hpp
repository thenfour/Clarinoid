
#pragma once

namespace clarinoid
{

static uint16_t gNextLiveNoteSequenceID = 1;
static inline uint16_t GetNextLiveNoteSequenceID()
{
    return ++gNextLiveNoteSequenceID;
}

struct HeldNoteInfo
{
    uint32_t mAttackTimestampMS;
    uint32_t mLiveNoteSequenceID;
    float mVelocity01;
    bool mIsPhysicallyHeld;
    MidiNote mMidiNote;
    float mRandomTrigger01;
};

// the held note tracker emits note events (think releasing a pedal etc); this is where to send em.
// this will be a musical device typically (USBKeyboardMusicalDevice).
struct IHeldNoteTrackerEvents
{
    virtual void IHeldNoteTrackerEvents_OnNoteOn(const HeldNoteInfo &noteInfo) = 0;
    virtual void IHeldNoteTrackerEvents_OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote) = 0;
    virtual void IHeldNoteTrackerEvents_OnAllNotesOff() = 0;
};

// takes incoming note ons & offs, and keeps track of what's happening
// see: 7jam, HeldNoteTracker:
// https://github.com/thenfour/digifujam/blob/ca285536909cee8c3282beb4a1f90a8cd3dc86dd/source/DFcommon/DFMusic.js#L310
struct HeldNoteTracker
{
  private:
    bool mPedalDown = false;
    static constexpr size_t MAX_HELD_NOTES = 128;
    std::vector<HeldNoteInfo> mHeldNotes; // NEW NOTES get placed at the BACK.
    IHeldNoteTrackerEvents *mEventHandler;

  public:
    HeldNoteTracker(IHeldNoteTrackerEvents *eventHandler) : mEventHandler(eventHandler)
    {
        mHeldNotes.reserve(MAX_HELD_NOTES);
    }


    // similar function to MusicalState::GetLiveMusicalVoice(const MusicalVoice &existing) const
    const HeldNoteInfo *FindExisting(uint32_t noteID) const {
        for (auto &noteInfo : mHeldNotes)
        {
            if (noteInfo.mLiveNoteSequenceID == noteID)
            {
                return &noteInfo;
            }
        }
        return nullptr;
    }

    // bool isPhysicallyPressed(uint32_t liveNoteSequenceID) const
    // {
    //     for (auto &noteInfo : mHeldNotes)
    //     {
    //         if (noteInfo.mLiveNoteSequenceID == liveNoteSequenceID)
    //         {
    //             return noteInfo.mIsPhysicallyHeld;
    //         }
    //     }
    //     return false;
    // }

    void AllNotesOff()
    {
        mHeldNotes.clear();
        mPedalDown = false;
        mEventHandler->IHeldNoteTrackerEvents_OnAllNotesOff();
    }

    // void DumpHeldNotes(const char *src)
    // {
    //     Serial.println(String(src) + "; held notes: ");
    //     for (size_t i = 0; i < mHeldNotes.size(); ++i)
    //     {
    //         auto &noteInfo = mHeldNotes[i];
    //         Serial.println(String(" #") + i + " id:" + noteInfo.mLiveNoteSequenceID +
    //                        (noteInfo.mIsPhysicallyHeld ? "Fingered " : "pedaled ") +
    //                        noteInfo.mMidiNote.GetNoteDesc().mName);
    //     }
    // }

    void PedalUp()
    {
        // DumpHeldNotes("pedal up");
        mPedalDown = false;
        // figure out the last physically-held note, in order to do trilling monophonic behavior
        HeldNoteInfo *pTrill = nullptr;
        for (auto it = mHeldNotes.rbegin(); it != mHeldNotes.rend(); ++it)
        {
            if (it->mIsPhysicallyHeld)
            {
                pTrill = &(*it);
                break;
            }
        }

        // remove all notes that are not physically held
        for (auto &noteInfo : mHeldNotes)
        {
            if (noteInfo.mIsPhysicallyHeld)
                continue;
            // Serial.println(String("held note tracker PedalUp; calling events::NoteOff on ") +
            //                noteInfo.mMidiNote.GetNoteDesc().mName);
            mEventHandler->IHeldNoteTrackerEvents_OnNoteOff(noteInfo, pTrill);
        }

        auto newEnd = std::remove_if(mHeldNotes.begin(), mHeldNotes.end(), [](const HeldNoteInfo &n) {
            bool ret = !n.mIsPhysicallyHeld;
            // if (ret)
            //     Serial.println(String("held note tracker PedalUp; removing ") + n.mMidiNote.GetNoteDesc().mName);
            return ret;
        });
        mHeldNotes.erase(newEnd, mHeldNotes.end());

        // Serial.println(String("held note tracker PedalUp; count= ") + mHeldNotes.size());
    }

    void PedalDown()
    {
        // Serial.println(String("held note tracker PedalDown"));
        mPedalDown = true;
    }

    void NoteOff(MidiNote note)
    {
        auto existingItem = mHeldNotes.begin();
        for (; existingItem != mHeldNotes.end(); ++existingItem)
        {
            if (existingItem->mMidiNote == note && existingItem->mIsPhysicallyHeld)
                break;
        }
        if (existingItem == mHeldNotes.end())
        {
            // Serial.println(String("held note tracker NoteOff, but is unknown. note=") + note.GetNoteDesc().mName +
            //                "; count=" + mHeldNotes.size());
            // DumpHeldNotes("NoteOff");
            return;
        }
        if (this->mPedalDown)
        {
            existingItem->mIsPhysicallyHeld = false;
            // Serial.println(String("held note tracker NoteOff, marking not held. note=") + note.GetNoteDesc().mName +
            //                "; count=" + mHeldNotes.size());
            // DumpHeldNotes("NoteOff");
            return;
        }
        // Serial.println(String("held note tracker NoteOff, erasing known note=") + note.GetNoteDesc().mName +
        //                "; count=" + mHeldNotes.size());
        auto ni = *existingItem;
        this->mHeldNotes.erase(existingItem);

        // DumpHeldNotes("NoteOff");

        HeldNoteInfo *pTrill = nullptr;
        if (!mHeldNotes.empty())
        {
            pTrill = &(*mHeldNotes.rbegin());
        }

        this->mEventHandler->IHeldNoteTrackerEvents_OnNoteOff(ni, pTrill);
    }

    HeldNoteInfo NoteOn(MidiNote note, float velocity01)
    {
        HeldNoteInfo n;
        n.mIsPhysicallyHeld = true;
        n.mAttackTimestampMS = millis();
        n.mVelocity01 = velocity01;
        n.mRandomTrigger01 = prng_f01();
        n.mMidiNote = note;
        n.mLiveNoteSequenceID = GetNextLiveNoteSequenceID();

        // We used to check for the same note existing, and update/drop it, but it causes problems
        // https://github.com/thenfour/Clarinoid/issues/163
        // where we lose track of a note and it gets stuck. Better to keep all notes, regardless whether they're
        // the same note value or not.
        // The reason it was done like that before is that we guarantee never to play more than 127 notes
        // simultaneously, good for memory mgmt.
        if (mHeldNotes.size() == MAX_HELD_NOTES)
        {
            // virtually remove the oldest note to prevent vector growing.
            auto existingItem = this->mHeldNotes.begin();
            auto ni = *existingItem;
            this->mHeldNotes.erase(existingItem);
            // this may not actually be technically correct; in mono mode for example maybe this will cause your note to
            // stop playing. but does it really matter?
            this->mEventHandler->IHeldNoteTrackerEvents_OnNoteOff(ni, nullptr);
        }

        mHeldNotes.push_back(n);

        // DumpHeldNotes("note on");

        this->mEventHandler->IHeldNoteTrackerEvents_OnNoteOn(n);
        // Serial.println(String("held note tracker NoteOn, adding note=") + note + "; count=" + mHeldNotes.size());
        return n;
    }
};

} // namespace clarinoid
