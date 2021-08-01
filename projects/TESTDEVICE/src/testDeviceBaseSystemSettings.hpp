
#pragma once

#define SIX_OCTAVE_SEQ_BUTTONS

#define CLARINOID_MIDI_INTERFACE Serial1

namespace clarinoid
{

const char gClarinoidVersion[] = "CLARINOID TEST APP";

static const size_t MAX_SYNTH_VOICES = 6;
#define VOICE_INITIALIZER {0}, {1}, {2}, {3}, {4}, {5}


static const size_t LOOPER_MEMORY_TOTAL_BYTES = 8192;
static const size_t LOOPER_TEMP_BUFFER_BYTES = 1024;

// check the memory usage menu to see what the value for this should be. it's NOT just 1 per voice or so; it's based on
// how the graph is processed i believe so just check the value.
static constexpr size_t AUDIO_MEMORY_TO_ALLOCATE = 50;
static constexpr float MAX_DELAY_MS = 500;

static const size_t HARM_PRESET_COUNT = 32;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t PERFORMANCE_PATCH_COUNT = 16;

static const int DEFAULT_TRANSPOSE = 12;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES =
    LOOP_LAYERS *
    (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 32;
static const size_t SYNTH_MODULATIONS_MAX = 6; // an additional 5 will be internally created because of FM modulation

static const size_t MAPPED_CONTROL_SEQUENCE_LENGTH = 4; // how many items in the "mapped control value sequence"

static const size_t MAX_DISPLAY_WIDTH = 128; // in order to maintain some static buffers.

// assignable slots.
static const size_t MAX_CONTROL_MAPPINGS = 48;

enum class PhysicalControl : uint8_t
{
    Back,
    Ok,
    Enc,
    EncButton,
    Button3,

    COUNT,
};

} // namespace clarinoid
