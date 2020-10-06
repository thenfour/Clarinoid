
#pragma once

//#include <optional> // not available!
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/harmonizer/harmonizer.hpp>

// # of notes to actually use in mapping dest scale.
static const size_t SCALE_DISAMBIGUATION_MAPPING_NOTES = 4;

// # of notes in the recent pool which results in the above mapping.
// we are polyphonic so you may need to throw out up to MAX_MUSICAL_VOICES number of voices if they are found to be too short.
static const size_t SCALE_DISAMBIGUATION_NOTES_TO_ANALYZE = SCALE_DISAMBIGUATION_MAPPING_NOTES + MAX_MUSICAL_VOICES;

static const int SCALE_FOLLOWER_TRANSIENT_NOTE_THRESH_MILLIS = 50;

// Map context to a new scale.
// KEY:
//   [4 bits scale flavor]
//   [12 bits of context notes, relative to scale root. each bit = note on or off]
// VALUE:
//   [4 bits scale root, relative to current (unsigned)]
//   [4 bits scale flavor index]
//
// if instead we wanted to like, use sorted list of notes, it gets more complex, and i'm not sure it's worth it.
#ifdef CLARINOID_DONT_INCLUDE_SCALE_FOLLOWER_LUT
uint8_t gScaleToScaleMappings[65536] = { 0 };
#else
#include "ScaleFollowerLUT.hpp"
#endif

// playing notes below this threshold will not even be considered for scale follower
static const int SCALE_FOLLOWER_IMPORTANCE_THRESHOLD = 100;

struct ScaleFollowerVoice
{
  MusicalVoice mRunningVoice;
  Stopwatch mNoteDurationTimer;
  Stopwatch mNoteOffTimer;
  int mImportance = 0;
  bool mTouched = false;
  bool mIsTransient = true;

  // log2 sorta
  template<size_t BitsToCount = 11, bool reverse = false>
  static int CalcDurationScore0_100(uint32_t lenMS)
  {
    // - duration - i'd like log behavior but maybe that's not worth the processing time
    int durationScore = 0;
    if (lenMS >= (1 << BitsToCount)) {
      if (reverse)
        durationScore = 0;
      else
        durationScore = BitsToCount + 1;
    }
    else
    {
      if (reverse) {
        lenMS = (1 << BitsToCount) - lenMS;
      }
      // 1 2 4 8 16 32 64 128 256 512 1024
      // 0 1 2 3 4  5  6  7   8   9   10
      for (size_t i = 0; i < BitsToCount; ++i)
      {
        if (lenMS) {
          durationScore = i + 1;
        }
        lenMS >>= 1;
      }
    }
    // now duration is 1-12; let's scale up to 110.
    durationScore = (durationScore * 100) / (BitsToCount + 1);
    return durationScore;
  }

  void UpdateImportance()
  {
    // calc importance
    // is playing
    // duration
    // time since note off
    // note value

    // - is playing (not playing but recent can still count!)
    bool isPlaying = mRunningVoice.IsPlaying();

    if (!isPlaying && mIsTransient) {
      mImportance = 0;
      return;
    }
      
    mImportance = isPlaying ? 100 : 0;

    // - duration - i'd like log behavior but maybe that's not worth the processing time
    int durationScore = CalcDurationScore0_100(mNoteDurationTimer.ElapsedMillis());
    if (mRunningVoice.IsPlaying()) {
      mImportance += durationScore;
    }
    else
    {
      mImportance += durationScore / 2;// duration means less if the note is off.
      // for notes that are not currently playing, how recently was it playing?
      // somehow i feel like this should be a longer timeline than note duration (maybe like up to 3-5 seconds),
      // and falloff linear.
      auto offTime = mNoteOffTimer.ElapsedMillis();
      offTime = offTime / 30;// scale 3000 to 100
      if (offTime > 100)
        offTime = 100;
      offTime = 100 - offTime;
      mImportance += offTime;
    }

    // - lower notes more important
    mImportance += (128 - mRunningVoice.mMidiNote) / 2;

    // - notes far away from others are more important, though this is hard to calculate efficiently.
    // - louder notes more important (but right now it's not used anyway)
  }

  void Update(const MusicalVoice& mv)
  {
    mTouched = true;
    MusicalVoiceTransitionEvents te;
    if (mv.mVoiceId != mRunningVoice.mVoiceId) {
      // unrelated note; no transition.
      te.mNeedsNoteOn = mv.IsPlaying();
    }
    else {
      te = CalculateTransitionEvents(mRunningVoice, mv);
    }
    if (te.mNeedsNoteOn) {
#ifdef CLARINOID_MODULE_TEST
      cc::log("  v:[%d] Note On [%s]", (int)mv.mVoiceId, MidiNote(mv.mMidiNote).ToString());
#endif // CLARINOID_MODULE_TEST
      mNoteDurationTimer.Restart();
    }
    else if (te.mNeedsNoteOff)
    {
      mIsTransient = mNoteDurationTimer.ElapsedMillis() < SCALE_FOLLOWER_TRANSIENT_NOTE_THRESH_MILLIS;
#ifdef CLARINOID_MODULE_TEST
      cc::log("  v:[%d] Note Off [%s], duration: [%d ms%s]", (int)mv.mVoiceId, MidiNote(mv.mMidiNote).ToString(), mNoteDurationTimer.ElapsedMillis(), mIsTransient ? " (TRANSIENT)" : "");
#endif // CLARINOID_MODULE_TEST
      mNoteDurationTimer.Pause();
      mNoteOffTimer.Restart();
    }
#ifdef CLARINOID_MODULE_TEST
    else {
      if (mv.IsPlaying()) {
        cc::log("  v:[%d] continuing... [%s], duration: [%d ms]", (int)mv.mVoiceId, MidiNote(mv.mMidiNote).ToString(), mNoteDurationTimer.ElapsedMillis());
      }
      else {
        cc::log("  v:[%d] no longer playing ... [%s], duration: [%d ms], age: [%d ms]", (int)mv.mVoiceId, MidiNote(mv.mMidiNote).ToString(), mNoteDurationTimer.ElapsedMillis(), mNoteOffTimer.ElapsedMillis());
      }
    }
#endif // CLARINOID_MODULE_TEST

    mRunningVoice = mv;

    if (mRunningVoice.IsPlaying()) {
      mIsTransient = mNoteDurationTimer.ElapsedMillis() < SCALE_FOLLOWER_TRANSIENT_NOTE_THRESH_MILLIS;
    }

    UpdateImportance();
  }

  // called when we didn't find this note any longer from musical state. consider it a note off.
//  void Stop()
//  {
//#ifdef CLARINOID_MODULE_TEST
//    if (mRunningVoice.IsPlaying()) {
//      cc::log("  v:[%d] STOPPING by force (?) [%s], duration: [%d ms]", (int)mRunningVoice.mVoiceId, MidiNote(mRunningVoice.mMidiNote).ToString(), mNoteDurationTimer.ElapsedMillis());
//    }
//    else {
//      cc::log("  v:[%d] STOPPING by force (?) [%s], duration: [%d ms], age: [%d ms]", (int)mRunningVoice.mVoiceId, MidiNote(mRunningVoice.mMidiNote).ToString(), mNoteDurationTimer.ElapsedMillis(), mNoteOffTimer.ElapsedMillis());
//    }
//#endif // CLARINOID_MODULE_TEST
//    mNoteDurationTimer.Pause();
//    mNoteOffTimer.Restart();
//    // make our running voice stop "playing".
//    mRunningVoice.mVelocity = 0;
//    //mRunningVoice.mMidiNote = 0; // don't set this to 0! we can use it to remember which note was played.
//    UpdateImportance();
//  }

  bool Eligible() const {
    // is the voice even eligible to be used for scale follower?
    if (mRunningVoice.mVoiceId == MAGIC_VOICE_ID_UNASSIGNED)
      return false;
    return true;
  }
};

// keeps a list of the top N items in an iteration over a collection.
template<typename Tkey, typename Tvalue, size_t N>
struct MaxItemsGrabber
{
  static bool lt(const std::pair<Tkey, Tvalue>& lhs, const std::pair<Tkey, Tvalue>& rhs)
  {
    return lhs.first < rhs.first;
  }

  SortedArray<std::pair<Tkey, Tvalue>, N, decltype(&lt)> mArray;

  MaxItemsGrabber() : mArray(&lt)
  {}

  void Update(Tkey k, Tvalue v)
  {
    //auto r = 
    mArray.Insert(std::make_pair(k, v));
  }
};


namespace ScaleFollowerDetail
{
  struct MapKey
  {
    bool operator <(const MapKey& rhs)const {
      return ToIndex() < rhs.ToIndex();
    }

    ScaleFlavorIndex mScaleFlavor;
    bool mPlayingNotes[12];

    uint16_t ToIndex() const {
      //   [4 bits scale flavor]
      //   [12 bits of context notes, relative to scale root. each bit = note on or off]
      uint16_t ret = ((uint16_t)mScaleFlavor) << 12;
      for (size_t i = 0; i < 12; ++i)
      {
        if (mPlayingNotes[i]) {
          ret |= 1 << i;
        }
      }
      return ret;
    }

    static MapKey FromScaleFlavorAndNoteBits(ScaleFlavorIndex sf, uint16_t noteBits)
    {
      MapKey ret;
      for (size_t i = 0; i < 12; ++i)
      {
        ret.mPlayingNotes[i] = noteBits & 1;
        noteBits >>= 1;
      }
      ret.mScaleFlavor = sf;
      return ret;
    }

    static MapKey FromIndex(size_t index)
    {
      //   [4 bits scale flavor]
      //   [12 bits of context notes, relative to scale root. each bit = note on or off]
      MapKey ret;
      for (size_t i = 0; i < 12; ++i)
      {
        ret.mPlayingNotes[i] = index & 1;
        index >>= 1;
      }
      ret.mScaleFlavor = (ScaleFlavorIndex)index;
      return ret;
    }
  };

  struct MapValue
  {
    ScaleFlavorIndex mScaleFlavor;
    int8_t mRelativeRoot;
    // some other stuff for diagnostics like confidence etc?
    uint16_t Serialize() const {
      //   [4 bits scale root, relative to current (unsigned)]
      //   [4 bits scale flavor index]
      return ((uint16_t)mRelativeRoot << 4) | ((int)mScaleFlavor);
    }

    static MapValue Deserialize(uint16_t t) {
      MapValue ret;
      ret.mScaleFlavor = (ScaleFlavorIndex)(t & 0xf);
      ret.mRelativeRoot = t >> 4;
      CCASSERT(ret.mRelativeRoot >= 0 && ret.mRelativeRoot <= 11);
      return ret;
    }
  };

} // namespace scalefollowerdetails

struct ScaleFollower
{
  ScaleFollowerVoice mVoices[MAX_MUSICAL_VOICES];

  // the current scale which is calculated ONLY of notes which are not transient.
  // this way, as transient notes pass and are ignored, their effect is reversed.
  Scale mCurrentScale = { Note::C, ScaleFlavorIndex::Major};

  // returns a voice that's either already assigned to this voice, or the best one to free up for it.
  ScaleFollowerVoice* FindAssignedOrAvailable(int16_t musicalVoiceId) {
    ScaleFollowerVoice* firstFree = nullptr;
    for (auto& v : mVoices) {
      if (v.mRunningVoice.mVoiceId == musicalVoiceId) {
        return &v; // already assigned to this voice.
      }
      if (!firstFree && (v.mRunningVoice.mVoiceId == MAGIC_VOICE_ID_UNASSIGNED)) {
        firstFree = &v;
      }
    }
    if (firstFree) {
      return firstFree;
    }
    CCASSERT(!F("not enough voices in scale follower??"));
    return &mVoices[0];
  }

  // Call to "feed" musical context.
  Scale Update(const MusicalVoice* voices, size_t voiceCount)
  {
#ifdef CLARINOID_MODULE_TEST
    cc::log("Scale Follower Update-----------------------");

    auto ImportantListToString = [](MaxItemsGrabber<int, ScaleFollowerVoice*, SCALE_DISAMBIGUATION_MAPPING_NOTES>& list) {
      String ret = "(";
      for (size_t i = 0; i < list.mArray.mSize; ++i) {
        // (Eb : 100)
        ret += MidiNote(list.mArray.mArray[i].second->mRunningVoice.mMidiNote).ToString();
        ret += " : ";
        ret += list.mArray.mArray[i].second->mImportance;
      }
      ret += ")";
      return ret.mStr.str();
    };

#endif // CLARINOID_MODULE_TEST

    MaxItemsGrabber<int, ScaleFollowerVoice*, SCALE_DISAMBIGUATION_MAPPING_NOTES> mostImportantNonTransient;
    MaxItemsGrabber<int, ScaleFollowerVoice*, SCALE_DISAMBIGUATION_MAPPING_NOTES> mostImportantWithTransient;

    // this is not necessary, because we reset it later, which prepares for the next frame.
    //for (auto& v : mVoices) { v.mTouched = false; }
    //int16_t mapKey = 0;

    for (size_t i = 0; i < voiceCount; ++i) {
      auto& lv = voices[i];
      auto* pv = FindAssignedOrAvailable(lv.mVoiceId);
      CCASSERT(!pv->mTouched); // this would be a duplicate voice ID in 1 update.
      pv->mTouched = true;
      pv->Update(lv);
      mostImportantWithTransient.Update(pv->mImportance, pv);
      if (!pv->mIsTransient) {
        mostImportantNonTransient.Update(pv->mImportance, pv);
      }
    }

    // calc note offs
    for (auto& v : mVoices)
    {
      if (v.mTouched) {
        v.mTouched = false; // reset for next frame
        continue;
      }
      if (v.Eligible()) {
        //v.Stop();
        v = {}; // reset it fully.
        mostImportantWithTransient.Update(v.mImportance, &v);
        if (!v.mIsTransient) {
          mostImportantNonTransient.Update(v.mImportance, &v);
        }
      }
    }

    uint16_t kNonTransient = ((uint16_t)mCurrentScale.mFlavorIndex) << 12;
    uint16_t kWithTransient = kNonTransient;

    auto fnTakeImportantNotes = [](const decltype(mostImportantWithTransient)& importantNotes, uint16_t& k, const Scale& currentScale)
    {
      for (size_t i = 0; i < importantNotes.mArray.mSize; ++i)
      {
        if (importantNotes.mArray.mArray[i].first < SCALE_FOLLOWER_IMPORTANCE_THRESHOLD) {
          break; // filter by threshold of importance
        }
        // convert to relative to the scale.
        auto* p = importantNotes.mArray.mArray[i].second;
        uint8_t rel = currentScale.MidiToChromaticRelativeToRoot(p->mRunningVoice.mMidiNote);
        CCASSERT(rel >= 0 && rel <= 11);

        k |= 1 << rel;
      }
    };

    fnTakeImportantNotes(mostImportantWithTransient, kWithTransient, mCurrentScale);
    fnTakeImportantNotes(mostImportantNonTransient, kNonTransient, mCurrentScale);

#ifdef CLARINOID_MODULE_TEST
    // dump important note list
#endif // CLARINOID_MODULE_TEST


    auto fnGetScale = [](uint16_t k, const Scale& currentScale)
    {
      // convert map value to scale.
      auto v = ScaleFollowerDetail::MapValue::Deserialize(gScaleToScaleMappings[k]);
      Scale ret;
      ret.mFlavorIndex = v.mScaleFlavor;
      int8_t root = v.mRelativeRoot;
      root += (uint8_t)currentScale.mRootNoteIndex;
      ret.mRootNoteIndex = (Note)RotateIntoRangeByte(root, 12);
      return ret;
    };

    auto newCurrentScale = fnGetScale(kNonTransient, mCurrentScale);
    Scale ret = fnGetScale(kWithTransient, newCurrentScale);

#ifdef CLARINOID_MODULE_TEST
    if (newCurrentScale != mCurrentScale) {
      cc::log("  CHANGING current scale [%s] + long notes [%s] ==> [%s]",
        mCurrentScale.ToString().mStr.str().c_str(),
        ImportantListToString(mostImportantNonTransient).c_str(),
        newCurrentScale.ToString().mStr.str().c_str());
    }
    cc::log("  Current scale [%s] + short notes [%s] ==> [%s]",
      newCurrentScale.ToString().mStr.str().c_str(),
      ImportantListToString(mostImportantWithTransient).c_str(),
      ret.ToString().mStr.str().c_str());
#endif // CLARINOID_MODULE_TEST

    mCurrentScale = newCurrentScale;

    return ret;
  }
};

static ScaleFollower gScaleFollower;

