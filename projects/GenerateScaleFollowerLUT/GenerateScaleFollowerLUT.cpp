// see also ScaleFollower.hpp for the LUT serialization format.

#define CLARINOID_PLATFORM_X86
#define CLARINOID_MODULE_TEST // OK this is not a test, but because we pull in all headers, we use test foundation like test timer instead of real timer.
#define CLARINOID_DONT_INCLUDE_SCALE_FOLLOWER_LUT

#include <clarinoid/x86/ArduinoEmu.hpp>
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/scale_follower/ScaleFollower.hpp>
#include <fstream>
#include <sys/stat.h>
#include <map>

using MapKey = ScaleFollowerDetail::MapKey;
using MapValue = ScaleFollowerDetail::MapValue;

std::string NoteListToString(const std::vector<Note>& c)
{
  String spn = "";
  for (auto& n : c) {
    spn = spn + " " + gNotes[(int)n].mName;
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

template<int period>
int ModularDistance(int a, int b)
{
  a = RotateIntoRange(a, period);
  b = RotateIntoRange(b, period);
  if (a > b) {
    return std::min(a - b, b + period - a);
  }
  return std::min(b - a, a + period - b);
}


uint8_t SemitoneDistance(Note a, Note b)
{
  return (uint8_t)ModularDistance<12>((int)a, (int)b);
}

int CalcDistance(const Scale& currentScale, const std::vector<Note>& playingNotes, const Scale& candidateScale, bool dolog)
{
  static constexpr int PLAYING_WEIGHT = 4;
  static constexpr int NEIGHBOR_ATTENUATION = 1;
  // max current scale is like 15 weight total.
  // max playing notes = 12 * PLAYING_WEIGHT, but that would make current scale all down to 12 absolute max.
  //static constexpr int DIST_MAX = 13 * PLAYING_WEIGHT;

  if (dolog) {
    cc::log("Analyzing scale [ %s ] while playing [ %s ], candidate [ %s ] ",
      currentScale.ToString().mStr.str().c_str(),
      NoteListToString(playingNotes).c_str(),
      candidateScale.ToString().mStr.str().c_str()
    );
    cc::log("  == calc fantasy scale ==");
  }

  // construct a fantasy scale which is the current scale, plus playing notes, and reduce the weight of current scale notes if they're within a semitone of any playing notes.
  std::vector<std::pair<Note, int>> fantasyScale; // vector of note,weight
  auto currentDiatonic = currentScale.GetDiatonicNotesAndWeights();
  for (auto i : currentDiatonic)
  {
    int weightIfNeighbor = i.second - NEIGHBOR_ATTENUATION;
    if (weightIfNeighbor < 1) weightIfNeighbor = 1;

    // is it within a semitone of a playing note? if so then reduce its weight because it's 
    for (auto p : playingNotes)
    {
      int d = SemitoneDistance(p, i.first);
      if (d == 0) {
        // the playing note corresponds with a note in the current scale. promote to the playing weight.
        if (dolog) {
          cc::log("  Note %s is a playing note; weight = %d", gNotes[(int)i.first].mName, PLAYING_WEIGHT);
        }
        i.second = PLAYING_WEIGHT;
        break;
      }
      else if (d == 1) {
        // neighboring note
        if (dolog) {
          cc::log("  Note %s is a neighbor note; reducing weight.", gNotes[(int)i.first].mName);
        }
        i.second = weightIfNeighbor;
      }
    }
    if (i.second > 0) {
      fantasyScale.push_back(i);
    }
    else {
      if (dolog) {
        cc::log("  Note %s has 0 weight; it doesn't make it into the fantasy scale.", gNotes[(int)i.first].mName);
      }
    }
  }

  for (auto p : playingNotes)
  {
    bool dupe = false;
    for (auto f : fantasyScale)
    {
      if (f.first == p) {
        dupe = true;
        break;
      }
    }
    if (!dupe) {
      if (dolog) {
        cc::log("  Adding nondiatonic playing note %s, weight %d.", gNotes[(int)p].mName, PLAYING_WEIGHT);
      }
      fantasyScale.push_back(std::make_pair(p, PLAYING_WEIGHT));
    }
  }

  if (dolog) {
    cc::log("  fantasy scale = [ %s ]", NoteListToString(fantasyScale).c_str());
    cc::log("  == calc distance ==");
  }

  // then just calc distance between cand & fantasy. this is a sort of manhattan distance.
  auto candDiatonic = candidateScale.GetDiatonicNotesAndWeights();
  int dist = 0;
  for (auto f : fantasyScale)
  {
    bool found = false;
    // find corresponding in candidate.
    for (auto c : candDiatonic)
    {
      if (c.first == f.first) {
        if (dolog) {
          cc::log("  Note distance %s (candidate[%d] - fantasy[%d]) satisfied! 0 dist added.", gNotes[(int)c.first].mName, c.second, f.second);
        }

        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }
    // not found in candidate.
    if (dolog) {
      cc::log("  Note distance %s = fantasy[%d] <- candidate doesn't contain this note.", gNotes[(int)f.first].mName, f.second);
    }
    dist += f.second;
  }

  // now, there may be notes in the candidate scale which aren't in the fanasty scale. those count against us.
  for (auto c : candDiatonic)
  {
    bool alreadyConsidered = false;
    for (auto p : fantasyScale)
    {
      if (p.first == c.first)
      {
        alreadyConsidered = true;
        break;
      }
    }
    if (!alreadyConsidered) {
      if (dolog) {
        cc::log("  Note distance %s = %d (candidate note which isn't in fantasy)", gNotes[(int)c.first].mName, c.second);
      }
      dist += c.second;
    }
  }
  //int ret = DIST_MAX - dist;

  if (dolog) {
    //cc::log("  fantasy scale = [ %s ]", NoteListToString(fantasyScale).c_str());
    cc::log("  dist = %d", dist);
    //cc::log("  fitness = %d", ret);
    //cc::log("  currentMatchScore = %d", currentMatchScore);
    //cc::log("  kissScore         = %d", kissScore);
    //cc::log("  fitness = %d", ret);
  }

  return dist;
}


bool gtpairintscale(const std::pair<int, Scale>& lhs, const std::pair<int, Scale>& rhs)
{
  return lhs.first > rhs.first;
}

std::map<MapKey, MapValue> GenerateMap()
{
  uint32_t keysFilled = 0;
  uint32_t fitnessCalculations = 0;
  // # of notes to actually use in mapping dest scale.
  //static const size_t SCALE_DISAMBIGUATION_MAPPING_NOTES = 4;
  // # of notes in the recent pool which results in the above mapping.
  // we are polyphonic so you may need to throw out up to MAX_MUSICAL_VOICES number of voices if they are found to be too short.
  //static const size_t SCALE_DISAMBIGUATION_NOTES_TO_ANALYZE = SCALE_DISAMBIGUATION_MAPPING_NOTES + MAX_MUSICAL_VOICES;

  //   [4 bits scale flavor]
  //   [12 bits of context notes, relative to scale root. each bit = note on or off]
  size_t mapSize = (1 << 16);
  std::map<MapKey, MapValue> ret;
  //for (size_t i = 0; i < mapSize; ++i)
  for (size_t sourceScaleFlavor = 0; sourceScaleFlavor < ScaleFlavorCount; ++ sourceScaleFlavor)
  {
    for (uint16_t playingNoteBits = 0; playingNoteBits < (1 << 12); ++playingNoteBits)
    {
      MapKey k = MapKey::FromScaleFlavorAndNoteBits((ScaleFlavorIndex)sourceScaleFlavor, playingNoteBits); // map key is the current scale flavor + playing notes
      if ((size_t)k.mScaleFlavor >= ScaleFlavorCount) {
        continue;
      }
      Scale currentScale = Scale(0, k.mScaleFlavor); // make everything relative to (0) for simplicity
      MapValue v;
      SortedArray<std::pair<int, Scale>, 50, decltype(&gtpairintscale)> candidateScales(&gtpairintscale);
      std::vector<Note> playingNotes;
      for (int note = 0; note < 12; ++note) {
        if (k.mPlayingNotes[note]) {
          playingNotes.push_back((Note)note);
        }
      }

      // figure out which scale would be most suitable.
      // skip any scales which are not eligible for scale following
      for (int scaleFlavor = 0; scaleFlavor < ScaleFlavorCount; ++scaleFlavor)
      {
        auto f = gScaleFlavors[scaleFlavor];
        for (int note = 0; note < f.mSymmetry; ++note)
        {
          Scale candidateScale = Scale(note, f.mID);
          // always allow the current scale flavor to be chosen.
          if (candidateScale != currentScale) {
            if (!f.IsAllowedInScaleFollower()) {
              continue;
            }
          }

          // calculate a fitness based on current scale, actual playing notes, and dest scale.
          // we could construct a hypothetical scale which includes notes of the original, replaced by notes being played.
          int distance = CalcDistance(currentScale, playingNotes, candidateScale,
            (fitnessCalculations % 5000 == 0) && (playingNotes.size() <= 3));
          candidateScales.Insert(std::make_pair(distance, candidateScale));

          fitnessCalculations++;
        }
      }

      CCASSERT(candidateScales.mSize);

      // here is the most suitable scale.
      auto chosenScale = candidateScales.mArray[0].second;

      if (playingNotes.size() <= 3) {
        cc::log("#%d CHOICE: Scale [ %s ] with notes [ %s ] = [ %s ]",
          keysFilled,
          currentScale.ToString().mStr.str().c_str(),
          NoteListToString(playingNotes).c_str(),
          chosenScale.ToString().mStr.str().c_str()
        );
      }

      v.mRelativeRoot = chosenScale.mRootNoteIndex; // because we start at C=0 scale, the new scale root is already relative.
      v.mScaleFlavor = chosenScale.mFlavorIndex;
      ret[k] = v;
      keysFilled++;
    }
  } // for each map entry

  return ret;
}



// Map context to a new scale.
// KEY:
//   [4 bits scale flavor]
//   [12 bits of context notes, relative to scale root. each bit = note on or off]
//
void Go(std::ostream& outp)
{
  auto map = GenerateMap();

  outp << std::endl << "// THIS FILE IS GENERATED by GenerateScaleFollowerLUT" << std::endl << std::endl;
  //outp << "#pragma once" << std::endl << std::endl;
  outp << "uint8_t gScaleToScaleMappings[" << map.size() << "] =" << std::endl;
  outp << "{" << std::endl;
  for (size_t i = 0; i < map.size(); ++i) {
    MapKey k = MapKey::FromIndex(i);
    auto& v = map[k];
    // convert scale to int8.
    outp << "  " << v.Serialize() << ",";
    if ((i % 12) == 11) // prettier to put many on 1 line
      outp << std::endl;
  }
  outp << "};" << std::endl << std::endl;
}


inline bool FileExists(const std::string& name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

int main()
{
  {
    // cases to test:
    // breaking out of chromatic scale by playing some very clearly major stuff?
    //[47a4:2638] Analyzing scale[C  Min] while playing[D  D# G], candidate[C  Maj]
    //  [47a4:2638]   playingMatches = 2
    //  [47a4:2638]   currentMatches = 2
    //  [47a4:2638]   fitness = 6

    //int f = CalcFitness(Scale(Note::C, ScaleFlavorIndex::Major), std::vector<Note> { Note::C }, Scale(Note::C, ScaleFlavorIndex::Major), true);
    //int f2 = CalcFitness(Scale(Note::C, ScaleFlavorIndex::Major), std::vector<Note> { Note::C }, Scale(Note::B, ScaleFlavorIndex::Power), true);

    //int f1 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Minor), { Note::D, Note::Eb, Note::G },
    //  Scale(Note::C, ScaleFlavorIndex::Minor), true);
    //int f2 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Minor), { Note::D, Note::Eb, Note::G },
    //  Scale(Note::C, ScaleFlavorIndex::Major), true);
    //int f3 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Minor), { Note::D, Note::Eb, Note::G },
    //  Scale(Note::C, ScaleFlavorIndex::MinorPentatonic), true);
    //int f4 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Minor), { Note::D, Note::Eb, Note::G },
    //  Scale(Note::C, ScaleFlavorIndex::HarmonicMinor), true);
    //int f5 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Minor), { Note::D, Note::Eb, Note::G },
    //  Scale(Note::C, ScaleFlavorIndex::MelodicMinor), true);

    //// [4b9c:4a54] #0 CHOICE: Scale[C  Chromatic] with notes[] = [B  Harm Min]
    //int f6 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Chromatic), {  },
    //  Scale(Note::C, ScaleFlavorIndex::Chromatic), true);
    //int f7 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Chromatic), {  },
    //  Scale(Note::B, ScaleFlavorIndex::HarmonicMinor), true);
    //int f8 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Chromatic), {  },
    //  Scale(Note::C, ScaleFlavorIndex::MajorPentatonic), true);



    //// [4358:5728] #4133 CHOICE: Scale[C  Major] with notes[C  D  F] = [D  Min Pent]
    //int f8 = CalcDistance(
    //  Scale(Note::C, ScaleFlavorIndex::Major), { Note::C, Note::D, Note::F_ },
    //  Scale(Note::C, ScaleFlavorIndex::Major), true);

    // [58f0:4eb0] #5136 CHOICE: Scale [ C  Major ] with notes [  E  A# ] = [ A# WholeTone ]
    int f9 = CalcDistance(
      Scale(Note::C, ScaleFlavorIndex::Major), { Note::E, Note::Bb },
      Scale(Note::C, ScaleFlavorIndex::Major), true);

    int fa = CalcDistance(
      Scale(Note::C, ScaleFlavorIndex::Major), { Note::E, Note::Bb },
      Scale(Note::Bb, ScaleFlavorIndex::WholeTone), true);

    int fb = CalcDistance(
      Scale(Note::C, ScaleFlavorIndex::Major), { Note::E, Note::Bb },
      Scale(Note::F_, ScaleFlavorIndex::Major), true);

    cc::log("ok");
  }

  // determine path.
  std::string path = ".\\src\\clarinoid\\scale_follower\\ScaleFollowerLUT.hpp";
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
