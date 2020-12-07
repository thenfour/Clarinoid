
#pragma once

//#include <optional> // not available!
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/harmonizer/harmonizer.hpp>

namespace clarinoid
{


// # of notes to actually use in mapping dest scale.
static const size_t SCALE_DISAMBIGUATION_MAPPING_NOTES = 4;

// # of notes in the recent pool which results in the above mapping.
// we are polyphonic so you may need to throw out up to MAX_MUSICAL_VOICES number of voices if they are found to be too short.
//static const size_t SCALE_DISAMBIGUATION_NOTES_TO_ANALYZE = SCALE_DISAMBIGUATION_MAPPING_NOTES + MAX_MUSICAL_VOICES;

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
#include "ScaleFollowerLUT.hpp"
#ifndef CLARINOID_SCALE_FOLLOWER_LUT
#define CLARINOID_SCALE_FOLLOWER_LUT
uint8_t gScaleToScaleMappings[65536] = { 0 };
#endif

// playing notes below this threshold will not even be considered for scale follower
static const int SCALE_FOLLOWER_IMPORTANCE_THRESHOLD = 100;


struct ScaleFollowerNote
{
private:

  // log2 sorta
  template<size_t BitsToCount = 11>
  static int CalcDurationScore0_100(int lenMS)
  {
    // - duration - i'd like log behavior but maybe that's not worth the processing time
    int durationScore = 0;
    if (lenMS >= (1 << BitsToCount)) {
      durationScore = BitsToCount + 1;
    }
    else
    {
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

    // - playing refs (not playing but recent can still count!)
    if ((mPlayingRefs == 0) && mIsTransient) {
      mImportance = 0;
      return;
    }

    mImportance = mPlayingRefs * 100;

    // - duration
    int durationScore = CalcDurationScore0_100((int)mNoteDurationTimer.ElapsedTime().ElapsedMillisI());
    if (mPlayingRefs > 0) {
      mImportance += durationScore;
    }
    else
    {
      mImportance += durationScore / 2;// duration means less if the note is off.
      // for notes that are not currently playing, how recently was it playing?
      // somehow i feel like this should be a longer timeline than note duration (maybe like up to 3-5 seconds),
      // and falloff linear.
      int offTime = (int)mNoteOffTimer.ElapsedTime().ElapsedMillisI();
      offTime = offTime / 30;// scale 3000 to 100
      if (offTime > 100)
        offTime = 100;
      offTime = 100 - offTime;
      mImportance += offTime;
    }

    // - lower notes more important
    mImportance += (128 - mLowestNote.GetMidiValue()) / 2;

    // - notes far away from others are more important, though this is hard to calculate efficiently.
    // - louder notes more important (but right now it's not used anyway)
  }


  Stopwatch mNoteDurationTimer;
  Stopwatch mNoteOffTimer;
  int mPlayingRefs = 0; // when this rises from 0, mNoteDurationTimer engages. When it drops to 0, mNoteOffTimer engages.
  MidiNote mLowestNote;

  bool mIsTransient = true;
  int mImportance = 0; // calculated at the end of the frame.

  // frame data. as you iterate through playing voices, accumulate data here.
  // then when the frame ends, merge it into our data and calculate importance.
  bool mTouched = false;
  int mFramePlayingRefs = 0; // how many voices are PLAYING this note (playing)
  MidiNote mFrameLowestPlayingNote;

public:
  int GetImportance() const { return mImportance; }
  bool IsTransient() const { return mIsTransient; }
  MidiNote GetNote() const { return mLowestNote; }

  ScaleFollowerNote(Note n) : mLowestNote(3, n)
  {
    mNoteDurationTimer.Pause();
    mNoteOffTimer.Restart();
  }

  void BeginFrame()
  {
    mTouched = false;
    mFramePlayingRefs = 0;
    mFrameLowestPlayingNote = { 255 };
  }

  // can be called multiple times per frame, for each voice playing this note index.
  void UpdateWithVoice(const MusicalVoice& mv)
  {
    mTouched = true;
    if (mv.IsPlaying())
    {
      ++mFramePlayingRefs;
      if (mv.mMidiNote < mFrameLowestPlayingNote.GetMidiValue())
        mFrameLowestPlayingNote = { mv.mMidiNote };
    }
  }

  void EndFrame()
  {
    if (mTouched)
    {
      // merge data into our data, update transient & importance
      if (mPlayingRefs == 0 && mFramePlayingRefs > 0) {
        // transition to playing.
        mNoteOffTimer.Pause();
        mNoteDurationTimer.Restart();
      }
      else if (mPlayingRefs > 0 && mFramePlayingRefs == 0) {
        // transition to stopped.
        mNoteDurationTimer.Pause();
        mNoteOffTimer.Restart();
      }

      mPlayingRefs = mFramePlayingRefs;

      if (mFramePlayingRefs > 0) {
        // only care about tracking note value if it's actually playing.
        mLowestNote = mFrameLowestPlayingNote;
      }
    }

    if (mPlayingRefs > 0) {
      // while the note is playing, track transient status.
      mIsTransient = mNoteDurationTimer.ElapsedTime().ElapsedMillisI() < SCALE_FOLLOWER_TRANSIENT_NOTE_THRESH_MILLIS;
    }

    if (!mTouched) {
      // if we didn't see this note this frame, then consider it a note off.
      if (mPlayingRefs) {
        mPlayingRefs = 0;
        // transition to stopped.
        mNoteDurationTimer.Pause();
        mNoteOffTimer.Restart();
      }
    }

    UpdateImportance();
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

using ImportantNoteList_t = MaxItemsGrabber<int, ScaleFollowerNote*, SCALE_DISAMBIGUATION_MAPPING_NOTES>;


namespace ScaleFollowerDetail
{
  // [4 bits scale flavor]
  // [15 bits notes & importance]
  //
  // 4 base-13 digits, in order of importance, where 0 is "no note".
  // so max decimal 13*13*13*13 = 28561 = 0x6F91 = 15 bits (max 32768)
  // notes are relative to scale root. each bit = note on or off
  struct MapKey
  {
    static constexpr size_t NoteBase = 13; // 12 for notes, 1 for whether the note is on or off.
    static constexpr size_t NoteBits = 15;
    using index_t = uint32_t; // needs to be able to hold the max key

    bool operator <(const MapKey& rhs)const {
      return SerializeToIndex() < rhs.SerializeToIndex();
    }

    ScaleFlavorIndex mScaleFlavor;
    int8_t mNotes[SCALE_DISAMBIGUATION_MAPPING_NOTES]; // 0 = note off; these are 0-12

    MapKey() = default;

#ifdef CLARINOID_MODULE_TEST
    MapKey(const Scale& currentScale, const std::vector<Note>& importantNotes)
    {
      mScaleFlavor = currentScale.mFlavorIndex;
      for (size_t i = 0; i < importantNotes.size(); ++i) {
        // convert to relative to the scale.
        //auto* p = importantNotes.mArray.mArray[i].second;
        uint8_t rel = currentScale.MidiToChromaticRelativeToRoot(MidiNote(3, importantNotes[i]).GetMidiValue());
        CCASSERT(rel >= 0 && rel <= 11);
        mNotes[i] = rel + 1; // +1 means it's a note on.
      }
    }
#endif

    index_t SerializeToIndex() const {
      uint32_t ret = mNotes[0];
      for (auto n : mNotes)
      {
        ret = (ret * NoteBase) + n;
      }
      ret |= ((uint32_t)mScaleFlavor) << NoteBits;
      return ret;
    }

    static index_t ToIndex(const Scale& currentScale, const ImportantNoteList_t& importantNotes)
    {
      index_t ret = 0;
      for (size_t i = 0; i < importantNotes.mArray.mSize; ++i) {
        ret *= NoteBase;
        // convert to relative to the scale.
        auto* p = importantNotes.mArray.mArray[i].second;
        uint8_t rel = currentScale.MidiToChromaticRelativeToRoot(p->GetNote().GetMidiValue());
        CCASSERT(rel >= 0 && rel <= 11);
        ret += rel + 1; // +1 means it's a note on.
      }
      ret |= ((index_t)currentScale.mFlavorIndex) << NoteBits;
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
  ScaleFollowerNote mNotes[12] = {
    { Note::C },
    { Note::Db },
    { Note::D },
    { Note::Eb },
    { Note::E },
    { Note::F_ },
    { Note::Gb },
    { Note::G },
    { Note::Ab },
    { Note::A },
    { Note::Bb },
    { Note::B },
  };

  // the current scale which is calculated ONLY of notes which are not transient.
  // this way, as transient notes pass and are ignored, their effect is reversed.
  Scale mCurrentScale = { Note::C, ScaleFlavorIndex::Major};

  // Call to "feed" musical context.
  Scale Update(const MusicalVoice* voices, size_t voiceCount)
  {
#ifdef CLARINOID_MODULE_TEST
    clarinoid::log("Scale Follower Update-----------------------");

    auto ImportantListToString = [](ImportantNoteList_t& list) {
      String ret = "(";
      for (size_t i = 0; i < list.mArray.mSize; ++i) {
        // (Eb : 100)
        ret += list.mArray.mArray[i].second->GetNote().ToString();
        ret += " : ";
        ret += list.mArray.mArray[i].second->GetImportance();
        ret += "  ";
      }
      ret += ")";
      return ret.mStr.str();
    };

#endif // CLARINOID_MODULE_TEST

    ImportantNoteList_t mostImportantNoTransients;
    ImportantNoteList_t mostImportantAll;

    for (auto& myv : mNotes)
    {
      myv.BeginFrame();
    }

    for (size_t i = 0; i < voiceCount; ++i) {
      auto& lv = voices[i];
      auto& myv = mNotes[MidiNote(lv.mMidiNote).GetNoteIndex()];
      myv.UpdateWithVoice(lv); // can be called more than once per note.
    }

    for (auto& myv : mNotes)
    {
      myv.EndFrame();
      mostImportantAll.Update(myv.GetImportance(), &myv);
      if (!myv.IsTransient()) {
        mostImportantNoTransients.Update(myv.GetImportance(), &myv);
      }
    }

    uint16_t kNoTransients = ((uint16_t)mCurrentScale.mFlavorIndex) << 12;
    uint16_t kAll = kNoTransients;

    auto fnTakeImportantNotes = [](const decltype(mostImportantNoTransients)& importantNotes, uint16_t& k, const Scale& currentScale)
    {
      for (size_t i = 0; i < importantNotes.mArray.mSize; ++i)
      {
        if (importantNotes.mArray.mArray[i].first < SCALE_FOLLOWER_IMPORTANCE_THRESHOLD) {
#ifdef CLARINOID_MODULE_TEST
          clarinoid::log("  Note [ %s ] importance [ %d ] below threshold [ %d ]",
            importantNotes.mArray.mArray[i].second->GetNote().ToString(),
            importantNotes.mArray.mArray[i].first,
            SCALE_FOLLOWER_IMPORTANCE_THRESHOLD
            );
#endif // CLARINOID_MODULE_TEST
          break; // filter by threshold of importance
        }
        // convert to relative to the scale.
        auto* p = importantNotes.mArray.mArray[i].second;
        uint8_t rel = currentScale.MidiToChromaticRelativeToRoot(p->GetNote().GetMidiValue());// p->mRunningVoice.mMidiNote);
        CCASSERT(rel >= 0 && rel <= 11);

        k |= 1 << rel;
      }
    };

    fnTakeImportantNotes(mostImportantNoTransients, kNoTransients, mCurrentScale);
    fnTakeImportantNotes(mostImportantAll, kAll, mCurrentScale);

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

    auto newCurrentScale = fnGetScale(kNoTransients, mCurrentScale);
    Scale ret = fnGetScale(kAll, mCurrentScale);

#ifdef CLARINOID_MODULE_TEST
    if (newCurrentScale != mCurrentScale) {
      clarinoid::log("  CHANGING current scale [%s] + long notes [%s] ==> [%s]",
        mCurrentScale.ToString().mStr.str().c_str(),
        ImportantListToString(mostImportantNoTransients).c_str(),
        newCurrentScale.ToString().mStr.str().c_str());
    }
    clarinoid::log("  Current scale [%s] + short notes [%s] ==> [%s]",
      mCurrentScale.ToString().mStr.str().c_str(),
      ImportantListToString(mostImportantAll).c_str(),
      ret.ToString().mStr.str().c_str());
#endif // CLARINOID_MODULE_TEST

    mCurrentScale = newCurrentScale;

    return ret;
  }
};

//static ScaleFollower gScaleFollower;

} // namespace clarinoid
