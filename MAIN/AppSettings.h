
#ifndef CCAPPSETTINGS_H
#define CCAPPSETTINGS_H

static const size_t MAX_VOICES = 8;

static const size_t HARM_VOICES = 4;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 4;

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 10;

bool gTouchKeyGraphsIsRunning = false;

class PresetName
{
  char buf[PRESET_NAME_LEN];
};

struct Note
{
  int mSequence;
  const char *mName;
};

Note gNotes[12] = {
  {0, "C"},
  {1, "C#"},
  {2, "D"},
  {3, "D#"},
  {4, "E"},
  {5, "F"},
  {6, "F#"},
  {7, "G"},
  {8, "G#"},
  {9, "A"},
  {10, "A#"},
  {11, "B"},
};

struct ScaleFlavor
{
  ScaleFlavor(int seq, const char *name, int8_t i1, int8_t i2 = -1, int8_t i3 = -1, int8_t i4 = -1, int8_t i5 = -1, int8_t i6 = -1, int8_t i7 = -1) :
    mSequence(seq),
    mName(name)
  {
    mIntervals[0] = i1;
    mIntervals[1] = i2;
    mIntervals[2] = i3;
    mIntervals[3] = i4;
    mIntervals[4] = i5;
    mIntervals[5] = i6;
    mIntervals[6] = i7;
    for (size_t i = 0; i < SizeofStaticArray(mIntervals); ++ i) {
      if (mIntervals[i] == -1) {
        mIntervalCount = i;
        break;
      }
    }
  }
  int mSequence;
  const char *mName;
  // list of intervals in the scale?
  uint8_t mIntervalCount;
  uint8_t mIntervals[7];
};

ScaleFlavor gScaleFlavors[5] = {
  {0, "Maj", 2,2,1,2,2,2,1},
  {0, "Who", 2},
  {0, "Chr", 1},
  {0, "hwd", 1,2},
  {1, "Alt", 1,2,1,2,2,2,2}
//  {0, "Maj", B10, B10, B1, B10, B10, B10, B1},
//  {1, "Alt", B11, B01, B1, B01, B01, B01, B0}
};
struct Scale
{
  int mFlavor = 0;
  int mRoot = 0;
};


enum class GlobalLocal : uint8_t
{
  Global,
  Local
};

EnumItemInfo<GlobalLocal> gGlobalLocalItems[2] = {
  { GlobalLocal::Global, "Global" },
  { GlobalLocal::Local, "Local" },
};

EnumInfo<GlobalLocal> gGlobalLocalInfo (gGlobalLocalItems);


struct HarmVoiceSequenceEntry
{
  bool mEnd = true;
  int8_t mInterval = 0;
};

enum class NoteOOBBehavior : uint8_t
{
  Drop,
  TransposeOctave
};

enum class NonDiatonicBehavior : uint8_t
{
  NextDiatonicNote,
  PrevDiatonicNote,
  FollowMelodyFromBelow, // so this voice plays a nondiatonic note too, based on distance from lower note
  FollowMelodyFromAbove, // so this voice plays a nondiatonic note too, based on distance from lower note
  Drop,
  DontMove,
  TryAlternateScale, // could be interesting to have a list of alternative scales to try.
};

enum class HarmVoiceType : uint8_t
{
  Off,
  Interval,
  Sequence
};

EnumItemInfo<HarmVoiceType> gHarmVoiceTypeItems[3] = {
  { HarmVoiceType::Off, "Off" },
  { HarmVoiceType::Interval, "Interval" },
  { HarmVoiceType::Sequence, "Sequence" },
};

EnumInfo<HarmVoiceType> gHarmVoiceTypeInfo (gHarmVoiceTypeItems);



struct HarmVoiceSettings
{
  HarmVoiceType mVoiceType = HarmVoiceType::Off;
  HarmVoiceSequenceEntry mSequence[HARM_SEQUENCE_LEN];
  GlobalLocal mSynthPresetRef = GlobalLocal::Global;
  int mLocalSynthPreset = 0;
  GlobalLocal mScaleRef = GlobalLocal::Global;
  Scale mLocalScale;
  int mMinOutpNote = 0;
  int mMaxOutpNote = 0;
  NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::Drop;
  NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::Drop;
  int mMinOutpVel = 0;
  int mMaxOutpVel = 127;
};

struct HarmSettings
{
  bool mIsEnabled = false;
  PresetName mName;
  HarmVoiceSettings mVoiceSettings[HARM_VOICES];

  int mGlobalSynthPreset;
  Scale mGlobalScale;
  int mMinRotationTimeMS;
};

struct AppSettings
{
  float mPortamentoTime = 0.005f;
  float mReverbGain = .2f;

  bool mDisplayDim = true;
  bool mOrangeLEDs = false;

  float mBreathLowerBound = 0.05f;
  float mBreathUpperBound = 0.7f;
  float mBreathNoteOnThreshold = 0.01;
  
  bool mMetronomeOn = false;
  float mMetronomeGain = 0.8f;
  int mMetronomeNote = 80;
  int mMetronomeDecayMS= 15;

  int mTranspose = 0;
  Scale mGlobalScale;
  float mBPM = 90.0f;
  
  HarmSettings mHarmSettings;

  float mTouchMaxFactor = 1.5f;
  float mPitchDownMin = 0.15f;
  float mPitchDownMax = 0.6f;
};

AppSettings gAppSettings;

#endif
