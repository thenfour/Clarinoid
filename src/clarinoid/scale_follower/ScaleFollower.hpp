#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/harmonizer/harmonizer.hpp>

// # of notes to actually use in mapping dest scale.
static const size_t SCALE_DISAMBIGUATION_MAPPING_NOTES = 3;
// # of notes in the pool which results in the above mapping.
// should be at least (MAPPING_NOTES + 1) because it's often that the
// currently-playing-note will be thrown out for being too short.
static const size_t SCALE_DISAMBIGUATION_NOTES_TO_ANALYZE = 5;

struct ToneToScaleDisambiguationMapping
{
  uint8_t mImportantTones[SCALE_DISAMBIGUATION_MAPPING_NOTES];
};

struct ToneToScaleConnection
{
  //
};

// huge generated map of all connections from a scale to another scale.
// the index is a tonal context of 4 notes, 3-bits each.
// 1st note is the nondiatonic note
// 2,3,4 notes are diatonic.
// that's 12 bits meaning each scale flavor has 1<<12 = 4096 mapping entries.
ToneToScaleConnection gScaleToScaleMappings[4096*20] =
{
  // dest scaleflavor + dest reltonic can even be just 1 byte, if we support <=16 scale flavors.
  // that would also mean this map can be a single huge map, where index is a bitfield of:
  // [4bits scale flavor][3 bites nondiatonic tone][3 bits context note 1][3 bits context note 2][3 bits context note 2]
  // for 16 bits worth of IDs: 64kbytes of data.
  // maj
  // harmmin
  // melmin
  // dim
  // wholetone
  // chrom
  // blues
  // 8 pentatonic
  // altered
  // 10 chord: 9
  // 11 chord: maj13 (maj minus 4th)
  // 12 unisono
  // 13 5ths
  // 14
  // 15
  // 16
};

// maybe some LUTs to help understand WHERE in that map to look for things.
// gmapping[scale][scaledegree][mapping]
// it's that last one that might be tricky. can we convert the mapping to a single 16-bit number to avoid a scan? that would be amazing.

// should be a ring buffer.
struct TonalContext
{
  struct Entry
  {
    // note
    // when started
    // velocity (todo)
  };
  Entry mEntries[SCALE_DISAMBIGUATION_NOTES_TO_ANALYZE];
  
  ToneToScaleDisambiguationMapping GetMapping()
  {
    // convert to 
    return ToneToScaleDisambiguationMapping();
  }
};

struct ScaleFollower
{
  Scale mCurrentScale;
  TonalContext mTonalContext;
  //uint8_t mImportantTones[3]; // chromatic and relative to current scale.
  // list of recently-played notes, sorted by most recent
  // when a note-off happens figure out if it should be removed.

  // Call to "feed" musical context. This will set gAppSettings.DeducedScale.
  // we can decide later if it's better to feed it STATE or EVENTS
  void Update(const MusicalVoice* voices, size_t voiceCount) {
    // prune tonal context,
    // feed tonal context with new notes, get our importantTones.

    // update scale using map.
  }
};

static ScaleFollower gScaleFollower;
