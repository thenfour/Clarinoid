// see also ScaleFollower.hpp for the LUT serialization format.

// - importance doesnt'really work i think; it just shrinks distances it doesn't allow fair comparisons with other scales. redistribution needs to happen. weights should add to 1.
// - i think adding char weight can only go so far; maybe consider a fixed meaningful set of weights.
//   Weight 0: this note does not boost the confidence of this scale at all. <-- implies that we need to really nail the purpose of these notes.
//   Weight 1: this note can suggest this scale.
//   Weight 2: this note is key to this scale. it's important
//   Weight 3: this is a characteristic note of this scale
//
// of course there's other problems with this. in C major all this depends on which chord you're playing. you'll need to add in the other
// modes to really make this work. account for dorian, phrygian, lydian, etc. not sure i have enough space for this; we might need to rearrange scale flavors eventually.

#define CLARINOID_PLATFORM_X86
#define CLARINOID_MODULE_TEST // OK this is not a test, but because we pull in all headers, we use test foundation like test timer instead of real timer.

#include <array>
#include <fstream>
#include <sys/stat.h>
#include <map>

#include <clarinoid/x86/ArduinoEmu.hpp>
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/scale_follower/ScaleFollower.hpp>

using MapKey = ScaleFollowerDetail::MapKey;
using MapValue = ScaleFollowerDetail::MapValue;


constexpr float NON_ZERO_CHARACTER_LIFT = 1.0f;
constexpr float PLAYING_BUT_NOT_IN_CURRENT_SCALE_WEIGHT = 2.5f;
constexpr float PLAYING_AND_CURRENT_NOTE_CHARACTERISTIC_WEIGHT_LIFT = 1.0f;
constexpr float NEIGHBOR_TO_PLAYING_NOTE_REDUCTION = 1;
constexpr float MAX_DISTANCE = 99999999.0f;


struct FantasyScaleNote
{
  Note mNote;
  float mCharacteristicWeight;
  float mImportanceFactor;
};

std::string NoteListToString(const std::vector<Note>& c)
{
  String spn = "";
  for (auto& p : c) {
    spn = spn + " " + gNotes[(int)p].mName;
  }
  return spn.mStr.str();
}

std::string NoteListToString(const std::vector<std::pair<Note, int>>& c)
{
  String spn = "";
  for (auto& n : c) {
    spn = spn + " " + gNotes[(int)n.first].mName + ":" + n.second;
  }
  return spn.mStr.str();
}

std::string NoteListToString(const std::vector<FantasyScaleNote>& c)
{
  String spn = "FantasyScale{note, impo, char} [ ";
  for (auto& n : c) {
    spn = spn + " [" + gNotes[(int)n.mNote].mName + ", " + n.mImportanceFactor + ", " + n.mCharacteristicWeight + "]";
  }
  spn += " ]";
  return spn.mStr.str();
}

uint8_t SemitoneDistance(Note a, Note b)
{
  return (uint8_t)ModularDistance<12>((int)a, (int)b);
}



bool gtpairintscale(const std::pair<float, Scale>& lhs, const std::pair<float, Scale>& rhs)
{
  return lhs.first > rhs.first;
}

bool IsSemitoneFromNonDiatonic(std::vector<std::pair<Note, int>> scaleDiatonics, Note diatonic, Note playingNote)
{
  // is the playing note a nondiatonic?
  for (auto& p : scaleDiatonics) {
    if (playingNote == p.first)
      return false;
  }
  return SemitoneDistance(diatonic, playingNote) < 2;
}

template<typename Container, typename Eq>
bool Contains(const Container& c, Eq&& comp)
{
  for (auto& i : c) {
    if (comp(i)) return true;
  }
  return false;
}



///////////////////////////////////////////////
// encapsulates the comparison of scales+playingnotes to a candidate scale.
// it's an object, so it can hold info about the comparison process to be examined later.
struct ScaleComparison
{
  // inputs
  Scale mSourceScale;
  std::vector<Note> mPlayingNotes;
  std::vector<FantasyScaleNote> mFantasyScale;

  Scale mCandidateScale;

  struct NoteDebugInfo
  {
    Note mNote;
    float mSourceCharacteristicWeight = 0.0f;
    float mSourceNonZeroLift = 0.0f;
    float mAlsoPlayingCharacteristicWeight = 0.0f;
    float mOnlyPlayingCharacteristicWeight = 0.0f;
    float mNeighborToNonDiatonicCharWeight = 0.0f;

    float mFantasyCharacterWeight = 0.0f;
    float mCandidateCharacterWeight = 0.0f;
    float mCandidateNonZeroLift = 0.0f;
    float mDist1 = 0.0f;
    float mImportanceFactor = 0.0f;
    float mDist2 = 0.0f;
    float mRet = 0.0f;

    // Note  SrcChar + NZ + AlsoPlaying + OnlyPlaying - Near = |FantChar - (CandChar + NZ)| = dist1 * ImpStretch = dist2 ^.5 = ret
    //  C    2       + 1  + .5          + 0           - 0    = |3.5      - (2        + 1 )| = 1.5   * 2          = 3     ^.5 = 1.44
  };

  // process debug info
  std::array<NoteDebugInfo, 12> mNoteDebugInfo;

  NoteDebugInfo& NoteInfo(size_t n) { return mNoteDebugInfo[n]; }
  NoteDebugInfo& NoteInfo(Note n) { return NoteInfo((size_t)n); }

  // output
  float mDistance;

  ScaleComparison(const Scale& sourceScale, const std::vector<Note>& playingNotes, const Scale& candidateScale = { Note::C, ScaleFlavorIndex::Major }) :
    mSourceScale(sourceScale),
    mPlayingNotes(playingNotes),
    mCandidateScale(candidateScale)
  {
    for (size_t i = 0; i < 12; ++i) {
      mNoteDebugInfo[i].mNote = (Note)i;
    }
    GenerateFantasyScale();
    CalcDistance();
  }

  ScaleComparison(const Scale& sourceScale, const std::vector<Note>& playingNotes, const std::vector<FantasyScaleNote>& fantasyScale, const Scale& candidateScale) :
    mSourceScale(sourceScale),
    mPlayingNotes(playingNotes),
    mFantasyScale(fantasyScale),
    mCandidateScale(candidateScale)
  {
    for (size_t i = 0; i < 12; ++i) {
      mNoteDebugInfo[i].mNote = (Note)i;
    }
    CalcDistance();
  }

  void GenerateFantasyScale()
  {
    auto sourceNotes = mSourceScale.GetDiatonicNotesAndCharacter();
    auto PlayingIndexToImportanceFactor = [&](size_t i) {
      float ret = (float)mPlayingNotes.size() - i;
      return ret;
    };

    for (auto& sn : sourceNotes) {
      FantasyScaleNote fn;
      fn.mCharacteristicWeight = (float)sn.second + NON_ZERO_CHARACTER_LIFT;
      fn.mNote = sn.first;
      fn.mImportanceFactor = 1;

      NoteInfo(sn.first).mSourceCharacteristicWeight = (float)sn.second;
      NoteInfo(sn.first).mSourceNonZeroLift = (float)NON_ZERO_CHARACTER_LIFT;

      for (size_t ip = 0; ip < mPlayingNotes.size(); ++ip)
      {
        auto& pn = mPlayingNotes[ip];
        // - notes from current scale which are also playing have characteristic weight amplified a bit, .5 or so.
        if (sn.first == pn) {
          NoteInfo(sn.first).mAlsoPlayingCharacteristicWeight = PLAYING_AND_CURRENT_NOTE_CHARACTERISTIC_WEIGHT_LIFT;
          fn.mCharacteristicWeight += PLAYING_AND_CURRENT_NOTE_CHARACTERISTIC_WEIGHT_LIFT;
          fn.mImportanceFactor = PlayingIndexToImportanceFactor(ip);
          //NoteInfo(sn.first).AddFlag(NoteDebugInfo::Flags::WeightLift_BothPlayingAndCurrent);
        }

        // - notes from current scale which are 1 semitone from a non-diatonic playing note get characteristic weight reduced.
        if (IsSemitoneFromNonDiatonic(sourceNotes, sn.first, pn)) {
          NoteInfo(sn.first).mNeighborToNonDiatonicCharWeight = NEIGHBOR_TO_PLAYING_NOTE_REDUCTION;
          fn.mCharacteristicWeight -= NEIGHBOR_TO_PLAYING_NOTE_REDUCTION;
          //NoteInfo(sn.first).AddFlag(NoteDebugInfo::Flags::WeightReduction_SemitoneFromNonDiatonicPlaying);
        }
      }

      mFantasyScale.push_back(fn);
    }

    // - notes that are playing but are not in the current scale get a medium characteristic weight. 1.5 or so.
    for (size_t ip = 0; ip < mPlayingNotes.size(); ++ip)
    {
      auto& pn = mPlayingNotes[ip];
      //NoteInfo(pn).AddFlag(NoteDebugInfo::Flags::ExistsInPlaying);
      if (Contains(mFantasyScale, [&](const FantasyScaleNote& i) { return i.mNote == pn; })) {
        continue; // already processed
      }
      //NoteInfo(pn).AddFlag(NoteDebugInfo::Flags::Weight_NeitherPlayingNorCurrent);
      FantasyScaleNote fn;
      NoteInfo(pn).mOnlyPlayingCharacteristicWeight = PLAYING_BUT_NOT_IN_CURRENT_SCALE_WEIGHT;
      fn.mCharacteristicWeight = PLAYING_BUT_NOT_IN_CURRENT_SCALE_WEIGHT;
      fn.mNote = pn;
      fn.mImportanceFactor = PlayingIndexToImportanceFactor(ip);
      mFantasyScale.push_back(fn);
    }
  }

  // calculate distance between a candidate scale and { current scale + playing notes }
  void CalcDistance()
  {
    // not a perfectly straight-forward way to do this, taking into account many factors
    // - musical perception
    // - multi-variability of different situations, playing notes etc
    // - characteristic weights
    // - note importance determined at runtime (which is a whole separate discussion)
    //
    // STEP 1: construct a "fantasy scale" which combines the current scale and the playing notes.
    // this emulates how our brains adapt the current scale as new information arrives.
    // - start with the current scale and characteristic weights. 
    // - add notes from playing notes
    // - notes from current scale which are 1 semitone from a non-diatonic playing note get characteristic weight reduced.
    // - notes from current scale which are also playing have characteristic weight amplified a bit, .5 or so.
    // - notes that are playing but are not in the current scale get a medium characteristic weight. 1.5 or so.
    //
    // STEP 2: get distance between fantasy scale and candidate scale. consider that different scales have different
    // number of notes, so we're comparing distances across incompatible dimensions.
    //
    // how to determine distances? missing notes would be a 0. we're comparing CHARACTER STRENGTHS so not-present is a 0-character strength.
    //
    // for a completely flat comparison, you then just compare all 12 dimensions.
    // euclidian distance would only compare notes in the union of {fantasy, candidate}, and it
    // would favor scales with fewer dimensions whcih is desired, but this balance should be tunable linearly 0-1
    //
    // distance dimensions are also scaled by note importance. So if you're playing 3 notes C G and D, multiply dimensions (stretching them) based on importance factor.
    // the least important is 1, which doesn't stretch but just uses the basic fantasy scale constructe.
    // next go 2, then 3, etc.

    // always allow the current scale flavor to be chosen.
    if ((mCandidateScale != mSourceScale) && !mCandidateScale.GetScaleFlavor().IsAllowedInScaleFollower()) {
      //mFlags = (Flags)((int)mFlags | (int)Flags::CandidateScaleNotAllowedInDeduction);
      //AddFlag(Flags::CandidateScaleNotAllowedInDeduction);
      mDistance = MAX_DISTANCE;
      return;
    }
    // calculate a fitness based on current scale, actual playing notes, and dest scale.
    // we could construct a hypothetical scale which includes notes of the original, replaced by notes being played.

    auto candidateNotes = mCandidateScale.GetDiatonicNotesAndCharacter();

    //if (pdebug) {
    //  cc::log("{ CalcDistance");
    //  cc::gLogIndent++;
    //  cc::log("%s", NoteListToString(fantasyScale).c_str());
    //  cc::log("Candidate scale: %s", NoteListToString(candidateNotes).c_str());
    //}

    float accum = 0;
    for (size_t id = 0; id < 12; ++id)
    {
      Note n = (Note)id;
      const NoteDesc& desc = gNotes[id];
      float importanceFactor = 1;
      float fantasyCharacter = 0;
      float candidateCharacter = 0;

      // find fantasy note and importance factor.
      for (auto& fn : mFantasyScale)
      {
        if (fn.mNote == n) {
          fantasyCharacter = fn.mCharacteristicWeight;
          importanceFactor = fn.mImportanceFactor;
          break;
        }
      }

      // find the note in the candidate scale.
      for (auto& cn : candidateNotes)
      {
        if (cn.first == n) {
          candidateCharacter = cn.second + NON_ZERO_CHARACTER_LIFT;
          NoteInfo(id).mCandidateCharacterWeight = (float)cn.second;
          NoteInfo(id).mCandidateNonZeroLift = NON_ZERO_CHARACTER_LIFT;
          //NoteInfo(cn.first).AddFlag(NoteDebugInfo::Flags::CandidateCharacterLift);
          //if (dolog) {
          //  cc::log("[%s] exists in candidate; lifting non-zero by %f to %f", desc.mName, NON_ZERO_CHARACTER_LIFT, candidateCharacter);
          //}
          break;
        }
      }

      NoteInfo(id).mFantasyCharacterWeight = fantasyCharacter;
      NoteInfo(id).mImportanceFactor = importanceFactor;

      float d = (candidateCharacter - fantasyCharacter);
      NoteInfo(id).mDist1 = d;
      d *= importanceFactor; // stretch it
      NoteInfo(id).mDist2 = d;
      d = d * d; // square it
      NoteInfo(id).mRet = d;


      //if (dolog) {
      //  cc::log("[%s] delta = (%f - %f) = (%f * (importance)%f) = %f, squared = %f",
      //    desc.mName,
      //    candidateCharacter, fantasyCharacter,
      //    (candidateCharacter - fantasyCharacter),
      //    importanceFactor,
      //    (candidateCharacter - fantasyCharacter) * importanceFactor,
      //    d
      //    );
      //}

      accum += d;
    }

    mDistance = std::sqrtf(accum);
    //if (dolog) {
    //  cc::log("SQRT(%f) = distance %f", accum, ret);
    //}

    //if (dolog) {
    //  cc::gLogIndent--;
    //  cc::log("} CalcDistance");
    //}
  }
};





template<size_t Digits>
void AddPermutations(std::vector<std::vector<Note>>& outp, std::vector<Note> l)
{
  outp.push_back(l);
  for (size_t n = 0; n < 12; ++n)
  {
    std::vector<Note> th(l);
    if (Contains(l, [&](const Note& xx) { return xx == (Note)n; })) {
      continue; // don't allow dupes in the list. all playing notes are unique.
    }
    th.push_back((Note)n);
    AddPermutations<Digits - 1>(outp, th);
  }
}

template<>
void AddPermutations<0>(std::vector<std::vector<Note>>& outp, std::vector<Note> l)
{
  return;
}


// ogre way of permuting 4 elements
std::vector<std::vector<Note>>& GetAllNoteImportanceCombinations()
{
  static std::vector<std::vector<Note>> ret;
  if (ret.empty()) {
    for (size_t n = 0; n < 12; ++n) {
      AddPermutations<4>(ret, { (Note)n });
    }
  }
  return ret;
}

std::vector<Scale>& GetAllCandidateScales()
{
  static std::vector<Scale> ret;
  if (ret.empty()) {
    for (int i = 0; i < ScaleFlavorCount; ++i)
    {
      for (int n = 0; n < 12; ++n)
      {
        ret.push_back(Scale(n, (ScaleFlavorIndex)i));
      }
    }
  }
  return ret;
}

Scale DeduceScale(const Scale& currentScale, std::vector<Note> playingNotes, bool dolog)
{
  ScaleComparison protoComp = { currentScale, playingNotes, Scale{ Note::C, ScaleFlavorIndex::Major } };
  auto& fantasyScale = protoComp.mFantasyScale;

  SortedArray<std::pair<float, Scale>, 50, decltype(&gtpairintscale)> candidateScales(&gtpairintscale);

  for (auto& candidateScale : GetAllCandidateScales())
  {
    ScaleComparison comp = { currentScale, playingNotes, fantasyScale, candidateScale };
    candidateScales.Insert(std::make_pair(comp.mDistance, candidateScale));
  }

  CCASSERT(candidateScales.mSize);

  // here is the most suitable scale.
  auto chosenScale = candidateScales.mArray[0].second;

  if (dolog) {
    cc::log("DEDUCED SCALE: Scale [ %s ] with notes [ %s ] = [ %s ]",
      currentScale.ToString().mStr.str().c_str(),
      NoteListToString(playingNotes).c_str(),
      chosenScale.ToString().mStr.str().c_str()
    );
  }
  return chosenScale;
}


void ExamineScaleDeduction(const Scale& currentScale, std::vector<Note> playingNotes, const Scale& expectedScale)
{
  ScopeLog ls(String("ExamineScaleDeduction for: Scale [ ")
    + currentScale.ToString().mStr.str().c_str()
    + " ] with notes [ "
    + NoteListToString(playingNotes).c_str()
    + " ]");

  auto deducedScale = DeduceScale(currentScale, playingNotes, true);
  ScaleComparison comp1 = { currentScale, playingNotes, expectedScale };
  ScaleComparison comp2 = { currentScale, playingNotes, deducedScale };
}


std::map<MapKey, MapValue> GenerateMap()
{
  //uint32_t keysFilled = 0;
  //uint32_t fitnessCalculations = 0;

  size_t mapSize = (1 << 16);
  std::map<MapKey, MapValue> ret;
  //auto allImportanceCombos = GetAllNoteImportanceCombinations();
  //auto allCandidateScales = GetAllCandidateScales();
  for (size_t sourceScaleFlavor = 0; sourceScaleFlavor < ScaleFlavorCount; ++ sourceScaleFlavor)
  {
    Scale sourceScale = Scale(Note::C, (ScaleFlavorIndex)sourceScaleFlavor); // make everything relative to (0) for simplicity
    for (auto& noteImportanceList : GetAllNoteImportanceCombinations())
    {
      MapKey k = MapKey(sourceScale, noteImportanceList);

      //auto fantasyScale = GenerateFantasyScale(sourceScale, noteImportanceList, nullptr);

      //SortedArray<std::pair<float, Scale>, 50, decltype(&gtpairintscale)> candidateScales(&gtpairintscale);

      //for (auto& candidateScale : GetAllCandidateScales())
      //{
      //  // always allow the current scale flavor to be chosen.
      //  if ((candidateScale != sourceScale) && !candidateScale.GetScaleFlavor().IsAllowedInScaleFollower()) {
      //    continue;
      //  }
      //  // calculate a fitness based on current scale, actual playing notes, and dest scale.
      //  // we could construct a hypothetical scale which includes notes of the original, replaced by notes being played.
      //  float distance = CalcDistance(sourceScale, noteImportanceList, fantasyScale, candidateScale, false);
      //  candidateScales.Insert(std::make_pair(distance, candidateScale));

      //  fitnessCalculations++;
      //}

      //CCASSERT(candidateScales.mSize);

      // here is the most suitable scale.
      //auto chosenScale = candidateScales.mArray[0].second;

      //cc::log("#%d CHOICE: Scale [ %s ] with notes [ %s ] = [ %s ]",
      //  keysFilled,
      //  sourceScale.ToString().mStr.str().c_str(),
      //  NoteListToString(noteImportanceList).c_str(),
      //  chosenScale.ToString().mStr.str().c_str()
      //);

      auto chosenScale = DeduceScale(sourceScale, noteImportanceList, false);

      MapValue v;
      v.mRelativeRoot = (uint8_t)chosenScale.mRootNoteIndex; // because we start at C=0 scale, the new scale root is already relative.
      v.mScaleFlavor = chosenScale.mFlavorIndex;
      ret[k] = v;
      //keysFilled++;
    }
  } // for each map entry

  return ret;
}


void Go(std::ostream& outp)
{
  auto map = GenerateMap();

  outp << std::endl << "// THIS FILE IS GENERATED by GenerateScaleFollowerLUT" << std::endl << std::endl;
  outp << "uint8_t gScaleToScaleMappings[" << map.size() << "] =" << std::endl;
  outp << "{" << std::endl;
  size_t i = 0;
  for (auto& kv : map) {
    ++i;
    // convert scale to int8.
    outp << " " << kv.second.Serialize() << ",";
    if (!(i % 32)) // prettier to put many on 1 line
      outp << std::endl;
  }
  outp << "};" << std::endl << std::endl << "#define CLARINOID_SCALE_FOLLOWER_LUT" << std::endl;
}

int main()
{
  {
    //auto allImportanceCombos = GetAllNoteImportanceCombinations();
    //auto allCandidateScales = GetAllCandidateScales();
        
    // [58f0:4eb0] #5136 CHOICE: Scale [ C  Major ] with notes [  E  A# ] = [ A# WholeTone ]
    ExamineScaleDeduction(
      { Note::C, ScaleFlavorIndex::Major },
      { Note::E, Note::Bb },
      { Note::F_, ScaleFlavorIndex::Major }
      );
    //int f9 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Major), { Note::E, Note::Bb },
    //  Scale(Note::C, ScaleFlavorIndex::Major), true);

    //int fa = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Major), { Note::E, Note::Bb },
    //  Scale(Note::Bb, ScaleFlavorIndex::WholeTone), true);

    //int fb = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Major), { Note::E, Note::Bb },
    //  Scale(Note::F_, ScaleFlavorIndex::Major), true);

    cc::log("ok");
  }

  // determine path.
  std::string path = ".\\src\\clarinoid\\scale_follower\\ScaleFollowerLUT.hpp";
  auto FileExists = [](const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
  };

  bool found = false;

  for (int i = 0; i < 10; ++i) {
    if (FileExists(path)) {
      found = true;
      break;
    }
    path = std::string("..\\") + path;
  }
  if (!found) {
    cc::log("couldn't find the output file; outta here.");
    return 0;
  }

  cc::log("Output file: %s", path.c_str());

  {
    std::ofstream f;
    f.open(path);
    Go(f);
  }
  return 0;
}
