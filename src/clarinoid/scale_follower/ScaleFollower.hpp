/*

brainstorming ideas for other methods of scale detection:
- just use a sort of "ideal" key continuously morphed from a C major scale, altering degrees as they're introduced.
  in other words, no LUT at all, just make your own scale. seems bad though.

*/
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
    // calc importance.
    // is playing
    // duration
    // time since note off
    // note value

    // - is playing (not playing but recent can still count!)
    mImportance = mRunningVoice.IsPlaying() ? 100 : 0;

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
      mNoteDurationTimer.Restart();
    }
    if (te.mNeedsNoteOff) {
      mNoteDurationTimer.Pause();
      mNoteOffTimer.Restart();
    }

    mRunningVoice = mv;

    UpdateImportance();
  }

  // called when we didn't find this note any longer from musical state. consider it a note off.
  void Stop()
  {
    mNoteDurationTimer.Pause();
    mNoteOffTimer.Restart();
    // make our running voice stop "playing".
    mRunningVoice.mVelocity = 0;
    //mRunningVoice.mMidiNote = 0; // don't set this to 0! we can use it to remember which note was played.
    UpdateImportance();
  }

  bool Eligible() const {
    // is the voice even eligible to be used for scale follower?
    if (mRunningVoice.mVoiceId == MAGIC_VOICE_ID_UNASSIGNED)
      return false;
    return true;
  }
};

// keep maximal values at front. pushes out too-low items.
template<typename Tval, size_t N, typename Tlessthan>
struct SortedArray
{
  size_t mSize = 0;
  Tval mArray[N];
  Tlessthan mLessThan;

  SortedArray(Tlessthan&& lt) :
    mLessThan(lt)
  {
  }

  void Clear()
  {
    mSize = 0;
  }

  bool Insert(Tval&& newVal)
  {
    size_t newIndex = 0;
    //std::optional<Tval> ret;
    bool ret = false;
    if (mSize > 0) {
      // figure out where to place it; use binary search.
      size_t windowLeft = 0;
      // place at END, not last index. It means we never actually compare windowRight.
      // this plays well with finding the edge.
      size_t windowRight = std::min(mSize, N - 1); 
      while (true)
      {
        size_t edge = (windowRight + windowLeft) / 2;
        CCASSERT(windowLeft != windowRight); // because R is never compared ("end"), they should never meet.
        if (mLessThan(newVal, mArray[edge])) {
          //     E          where E == L
          //     L--------R where R = L+1
          //      ====V===|   <-- val is less than L which means it's R.
          // OR
          //     L--------E--------R
          //     |         ====V===|
          // =>    -->    |        |
          if (windowLeft == edge) {
            CCASSERT(windowRight == (windowLeft + 1));
            newIndex = windowRight;
            break;
          }
          windowLeft = edge;
        }
        else
        {
          //     E            where E == L
          //     L--------R   where R = L+1
          //     V therefore V == E, and therefore should be inserted at E.
          //    Here, 
          // OR
          //     L--------E--------R
          //     |====V====        |
          // =>  |        |    <--
          if (windowLeft == edge) {
            CCASSERT(windowRight == (windowLeft + 1));
            newIndex = edge;
            break;
          }
          windowRight = edge;
        }
      }

      // insert it there...
      // remember last item.
      size_t itemsToSlide = mSize - newIndex;
      if (mSize == N) {
        // we'll be pushing an item off.
        --itemsToSlide; 
        ret = true;
        //ret.emplace(std::move(mArray[N - 1]));
      }
    
      for (size_t i = newIndex + itemsToSlide; i > newIndex; -- i) {
        mArray[i] = std::move(mArray[i - 1]);
      }
    }

    mArray[newIndex] = std::move(newVal);
    if (mSize < N)
    {
      ++mSize;
    }

    return ret;
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
  Scale Update(Scale currentScale, const MusicalVoice* voices, size_t voiceCount)
  {
    MaxItemsGrabber<int, ScaleFollowerVoice*, SCALE_DISAMBIGUATION_MAPPING_NOTES> mostImportant;

    // this is not necessary, because we reset it later, which prepares for the next frame.
    //for (auto& v : mVoices) { v.mTouched = false; }
    //int16_t mapKey = 0;

    for (size_t i = 0; i < voiceCount; ++ i) {
      auto& lv = voices[i];
      auto* pv = FindAssignedOrAvailable(lv.mVoiceId);
      pv->mTouched = true;
      pv->Update(lv);
      mostImportant.Update(pv->mImportance, pv);
    }

    // calc note offs
    for (auto& v : mVoices) 
    {
      if (!v.mTouched) {
        v.Stop();
        if (v.Eligible()) {
          mostImportant.Update(v.mImportance, &v);
        }
      }
      v.mTouched = false; // reset for next frame
    }

    uint16_t k = ((uint16_t)currentScale.mFlavorIndex) << 12;

    // take important notes
    for (size_t i = 0; i < mostImportant.mArray.mSize; ++i)
    {
      if (mostImportant.mArray.mArray[i].first < SCALE_FOLLOWER_IMPORTANCE_THRESHOLD) {
        break; // filter by threshold of importance
      }
      // convert to relative to the scale.
      auto* p = mostImportant.mArray.mArray[i].second;
      uint8_t rel = currentScale.MidiToChromaticRelativeToRoot(p->mRunningVoice.mMidiNote);
      CCASSERT(rel >= 0 && rel <= 11);

      k |= 1 << rel;
    }

    // convert map value to scale.
    auto v = ScaleFollowerDetail::MapValue::Deserialize(gScaleToScaleMappings[k]);
    Scale ret;
    ret.mFlavorIndex = v.mScaleFlavor;
    int8_t root = v.mRelativeRoot;
    root += currentScale.mRootNoteIndex;
    ret.mRootNoteIndex = RotateIntoRangeByte(root, 12);

    return ret;
  }
};

static ScaleFollower gScaleFollower;

