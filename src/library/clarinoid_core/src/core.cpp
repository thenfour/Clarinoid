
#include <cstdint>
#include <clarinoid/core/basic/assert.hpp>
#include <clarinoid/core/basic/Util.hpp>
#include <clarinoid/core/basic/Music.hpp>

namespace clarinoid
{

// Uptime.hpp
//uint32_t gUptimeLastMicrosCall = 0;
//int64_t gUptimeCurrentOffset = 0;  // every time the 32-bit micros() call rolls over, this gets += 1<<32;

// assert.hpp
extern IClarinoidCrashReportOutput* gCrashHandlers[10] = {0};
SerialCrashHandler gSerialCrashHandler;

// log.hpp
int gLogIndent = 0;

// util.hpp
int NoInterrupts::gNoInterruptRefs = 0;

// Music.hpp
EnumItemInfo<Note> gNoteItems[12] = {
    {Note::C, "C"},
    {Note::Db, "Db"},
    {Note::D, "D"},
    {Note::Eb, "Eb"},
    {Note::E, "E"},
    {Note::F_, "F_"},
    {Note::Gb, "Gb"},
    {Note::G, "G"},
    {Note::Ab, "Ab"},
    {Note::A, "A"},
    {Note::Bb, "Bb"},
    {Note::B, "B"},
};

NoteDesc const gNotes[12] = {
    {0, "C ", "C "},
    {1, "C#", "C" CHARSTR_SHARP},
    {2, "D ", "D "},
    {3, "D#", "D" CHARSTR_SHARP},
    {4, "E ", "E "},
    {5, "F ", "F "},
    {6, "F#", "F" CHARSTR_SHARP},
    {7, "G ", "G "},
    {8, "G#", "G" CHARSTR_SHARP},
    {9, "A ", "A "},
    {10, "A#", "A" CHARSTR_SHARP},
    {11, "B ", "B "},
};

EnumInfo<Note> gNoteInfo("Note", gNoteItems);

EnumItemInfo<ScaleFlavorIndex> gScaleFlavorIndexItems[ScaleFlavorCount] = {
    {ScaleFlavorIndex::Chromatic, "Chrom"},
    {ScaleFlavorIndex::Major, "Maj"},
    {ScaleFlavorIndex::Minor, "Min"},
    {ScaleFlavorIndex::MelodicMinor, "MelMin"},
    {ScaleFlavorIndex::HarmonicMinor, "HarmMin"},
    {ScaleFlavorIndex::MajorPentatonic, "MajPent"},
    {ScaleFlavorIndex::MinorPentatonic, "MinPent"},
    {ScaleFlavorIndex::WholeTone, "Whole"},
    {ScaleFlavorIndex::HalfWholeDiminished, "HWDim"},
    {ScaleFlavorIndex::WholeHalfDiminished, "WHDim"},
    {ScaleFlavorIndex::Altered, "Alt"},
    {ScaleFlavorIndex::Blues, "Blues"},
    {ScaleFlavorIndex::Unison, "Unison"},
    {ScaleFlavorIndex::Power, "Power"},
};

EnumInfo<ScaleFlavorIndex> gScaleFlavorIndexInfo("ScaleFlavorIndex", gScaleFlavorIndexItems);



}  // namespace clarinoid
