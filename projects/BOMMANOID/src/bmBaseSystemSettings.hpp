
#pragma once

#define THREE_BUTTON_OCTAVES

#define CLARINOID_MIDI_INTERFACE Serial1

namespace clarinoid
{
    // for breath controllers, this is true to enable a specialized filter controlled by breath.
    // that filter is a temporary thing in the design and should be replaced by a proper modulation in the graph,
    // but for the moment it's a hard-coded graph element with logic in synthvoice. So for non-breath controllers,
    // this is a way to disable it.
static const bool USE_BREATH_FILTER = false;

const char gClarinoidVersion[] = "Bommanoid v0.00";

static const int8_t DEFAULT_TRANSPOSE = 0;
static const int8_t PERFORMANCE_PATCH_COUNT = 24;

static const size_t MAX_SYNTH_VOICES = 3;
#define VOICE_INITIALIZER {0}, {1}, {2}//, {3}, {4}, {5}, {6}, {7}

static const size_t LOOPER_MEMORY_TOTAL_BYTES = 16300;
static const size_t LOOPER_TEMP_BUFFER_BYTES = 512;   // a smaller buffer that's just used for intermediate copy ops

// check the memory usage menu to see what the value for this should be. it's NOT just 1 per voice or so; it's based on
// how the graph is processed i believe so just check the value.
static constexpr size_t AUDIO_MEMORY_TO_ALLOCATE = 15 + 1000;
static constexpr float MAX_DELAY_MS = 300;

static constexpr size_t MUSICALSTATE_TIMESLICE_PERIOD_MICROS = 1400;
static constexpr size_t BREATH_SIGNAL_SMOOTHING_PERIOD_MICROS = 18000;
static constexpr size_t BREATH_SIGNAL_SMOOTHING_FRAMES =
    (BREATH_SIGNAL_SMOOTHING_PERIOD_MICROS / MUSICALSTATE_TIMESLICE_PERIOD_MICROS);

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 4;
static constexpr size_t MAX_MUSICAL_VOICES =
    LOOP_LAYERS *
    (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 32;
static const size_t SYNTH_MODULATIONS_MAX = 8;

static const size_t MAPPED_CONTROL_SEQUENCE_LENGTH = 4; // how many items in the "mapped control value sequence"

static const size_t MAX_DISPLAY_WIDTH = 128; // in order to maintain some static buffers.

// assignable slots.
static const size_t MAX_CONTROL_MAPPINGS = 48;

enum class PhysicalControl : uint8_t
{
    Enc,
    Back,
    Ok,
    x1,
    x2,
    // CPToggleUp,
    // CPEncButton,
    // LHx1,
    // LHx2,
    // LHx3,
    // LHx4,
    // LHEncButton,
    // LHBack,
    // LHOk,
    // LHThx1,
    // LHThx2,
    // LHOct1,
    // LHOct2,
    // LHOct3,
    // LHKey1,
    // LHKey2,
    // LHKey3,
    // LHKey4,
    // RHTh1,
    // RHTh2,
    // RHTh3,
    // // RHJoyButton,
    // // RHEncButton,
    // RHx1,
    // RHx2,
    // RHx3,
    // RHx4,
    // RHx5,
    // RHKey1,
    // RHKey2,
    // RHKey3,
    // RHKey4,

    // Breath,
    // Pitch,
    // JoyX,
    // JoyY,
    // Volume,

    // LHEnc,
    // RHEnc,

    COUNT,
};

} // namespace clarinoid