
#pragma once

#define SIX_OCTAVE_SEQ_BUTTONS

#define CLARINOID_MIDI_INTERFACE Serial1

namespace clarinoid
{

const char gClarinoidVersion[] = "CLARINOID 2";

static const size_t MAX_SYNTH_VOICES = 6;

static const size_t HARM_PRESET_COUNT = 32;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 32;
static const size_t SYNTH_MODULATIONS_MAX = 8;

static const size_t MAPPED_CONTROL_SEQUENCE_LENGTH = 4; // how many items in the "mapped control value sequence"

static const size_t MAX_DISPLAY_WIDTH = 128; // in order to maintain some static buffers.

// assignable slots.
static const size_t MAX_CONTROL_MAPPINGS = 64;


enum class PhysicalControl : uint8_t
{
    Back,
    Ok,
    EncButton,
    LHx1,
    LHx2,
    LHx3,

    Oct1,
    Oct2,
    Oct3,
    Oct4,
    Oct5,
    Oct6,
    
    LHKey1,
    LHKey2,
    LHKey3,
    LHKey4,

    RHx1,
    RHx2,
    RHx3,
    RHx4,
    RHKey1,
    RHKey2,
    RHKey3,
    RHKey4,

    Breath,
    Pitch,

    Enc,

    COUNT,
};

} // namespace clarinoid
