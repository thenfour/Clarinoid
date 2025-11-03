#pragma once

#include <algorithm> // std::min, std::max, sort...
#include <array>
#include <cstdint>
#include "FixedVector.hpp"
#include "Music.hpp"

namespace clarinoid
{

// =================== Spelling cost knobs (compile-time) ===================
namespace SpellCost
{
// Per-note accidental simplicity
constexpr int ACC_NATURAL = 0;
constexpr int ACC_FLAT = 100;  // 100;   // tie breaker: prefer flats over sharps
constexpr int ACC_SHARP = 110; // 110;   // sharp/flat

constexpr int RARITY_BASE = 500;           // 500;
constexpr int RARITY_F_FLAT = RARITY_BASE; // penalty for rare spellings
constexpr int RARITY_E_SHARP = RARITY_BASE;
constexpr int RARITY_B_SHARP = RARITY_BASE;
constexpr int RARITY_C_FLAT = RARITY_BASE;

// Pairwise / interval readability
constexpr int BASE_MISMATCH = 300;    // 300;   // base when not a simple diatonic interval. like aug 6.
constexpr int DISTANCE_TO_BASE = 100; // 100;   // per-semitone distance to nearest allowed interval
constexpr int AUG_DIM_UNISON = 300;   // 300;   // penalty same letter but different note (e.g., D -> D#)

// Aesthetic nudge (keep zero or tiny)
constexpr int SIGN_FLIP = 10; // 2;   // penalty when sharp flat between neighbors (Db Eb is better than C# Eb)
} // namespace SpellCost

// =================== Types ===================
// todo: maybe better name.
enum class Letter : int
{
    C = 0,
    D = 1,
    E = 2,
    F = 3,
    G = 4,
    A = 5,
    B = 6
};
enum class Accidental : int
{
    // DoubleFlat = -2,
    Flat = -1,
    Natural = 0,
    Sharp = 1,
    // DoubleSharp = 2
};

struct SpelledNote
{
    Letter letter;
    Accidental acc;
    // int8_t octave; // C4=60 -> 4
    MidiNote midi; // original MIDI 0..127

    String ToString(bool withOctave = false) const
    {
        String s;
        switch (letter)
        {
        case Letter::C:
            s += "C";
            break;
        case Letter::D:
            s += "D";
            break;
        case Letter::E:
            s += "E";
            break;
        case Letter::F:
            s += "F";
            break;
        case Letter::G:
            s += "G";
            break;
        case Letter::A:
            s += "A";
            break;
        case Letter::B:
            s += "B";
            break;
        }
        switch (acc)
        {
        case Accidental::Flat:
            s += CHARSTR_FLAT;
            break;
        case Accidental::Natural:
            s += " "; // keep fixed width
            break;
        case Accidental::Sharp:
            s += CHARSTR_SHARP;
            break;
        }

        if (withOctave)
        {
            s += String(midi.GetOctave());
        }
        return s;
    }
};

struct SpellResult
{
    fixed_vector<SpelledNote, MAX_SYNTH_VOICES> notes; // same order as input
};

// =================== LUTs & helpers ===================

static inline int letterIndex(Letter L)
{
    return static_cast<int>(L);
}

// Natural letter semitone (relative to C)
static inline int naturalSemitone(Letter L)
{
    switch (L)
    {
    case Letter::C:
        return 0;
    case Letter::D:
        return 2;
    case Letter::E:
        return 4;
    case Letter::F:
        return 5;
    case Letter::G:
        return 7;
    case Letter::A:
        return 9;
    case Letter::B:
        return 11;
    }
    return 0;
}

static inline int accOffset(Accidental a)
{
    return static_cast<int>(a);
}

// Absolute semitone (0..11) of a spelled letter (ignore octave)
static inline int semitoneOf(Letter L, Accidental a)
{
    int s = naturalSemitone(L) + accOffset(a);
    s %= 12;
    if (s < 0)
        s += 12;
    return s;
}

// // MIDI → octave with C4=60  (oct = midi/12 - 1)
// static inline int midiToOctave(int midi)
// {
//     return midi / 12 - 1;
// }

static constexpr size_t MAX_CANDIDATES_PER_PITCH = 2;
// Candidates per pitch class (prefer naturals / single accidentals; doubles kept only where natural)
using CandidatePair = std::pair<Letter, Accidental>;
using CandidatePairList = fixed_vector<CandidatePair, MAX_CANDIDATES_PER_PITCH>;
static inline const std::array<CandidatePairList, 12> &candidateTable()
{
    static const std::array<CandidatePairList, 12> table = {
        /*0 C  */ CandidatePairList{CandidatePair{Letter::C, Accidental::Natural}, {Letter::B, Accidental::Sharp}},
        /*1 C# */ CandidatePairList{CandidatePair{Letter::C, Accidental::Sharp}, {Letter::D, Accidental::Flat}},
        /*2 D  */
        CandidatePairList{
            CandidatePair{Letter::D, Accidental::Natural},
        },
        /*3 D# */ CandidatePairList{CandidatePair{Letter::D, Accidental::Sharp}, {Letter::E, Accidental::Flat}},
        /*4 E  */ CandidatePairList{CandidatePair{Letter::E, Accidental::Natural}, {Letter::F, Accidental::Flat}},
        /*5 F  */ CandidatePairList{CandidatePair{Letter::F, Accidental::Natural}, {Letter::E, Accidental::Sharp}},
        /*6 F# */ CandidatePairList{CandidatePair{Letter::F, Accidental::Sharp}, {Letter::G, Accidental::Flat}},
        /*7 G  */ CandidatePairList{CandidatePair{Letter::G, Accidental::Natural}},
        /*8 G# */ CandidatePairList{CandidatePair{Letter::G, Accidental::Sharp}, {Letter::A, Accidental::Flat}},
        /*9 A  */ CandidatePairList{CandidatePair{Letter::A, Accidental::Natural}},
        /*10 A#*/ CandidatePairList{CandidatePair{Letter::A, Accidental::Sharp}, {Letter::B, Accidental::Flat}},
        /*11 B */ CandidatePairList{CandidatePair{Letter::B, Accidental::Natural}, {Letter::C, Accidental::Flat}}};
    return table;
}

// Generate candidate spelled notes for a MIDI number.
// Because semitoneOf(letter,acc) == pc, octave is simply midi/12 - 1 (C4=60).
static inline fixed_vector<SpelledNote, MAX_CANDIDATES_PER_PITCH> generateCandidatesForMidi(const MidiNote &midi)
{
    fixed_vector<SpelledNote, MAX_CANDIDATES_PER_PITCH> out;
    // int pitchClass = midi % 12;
    // if (pitchClass < 0)
    //     pitchClass += 12;
    // int oct = midiToOctave(midi);
    const int pitchClass = midi.GetNoteIndex();
    // const int oct = midi.GetOctave();
    for (auto &p : candidateTable()[pitchClass])
    {
        Letter L = p.first;
        Accidental A = p.second;
        if (semitoneOf(L, A) == pitchClass)
        {
            out.push_back(SpelledNote{L, A, midi});
        }
    }
    if (out.empty())
    { // Safety fallback; shouldn't happen
        out.push_back(SpelledNote{Letter::C, Accidental::Natural, midi});
    }
    return out;
}

// =================== Integer costs ===================

static inline int accidentalCostInt(const SpelledNote &sn)
{
    switch (sn.acc)
    {
    case Accidental::Natural:
        return SpellCost::ACC_NATURAL;
    case Accidental::Sharp:
        return SpellCost::ACC_SHARP;
    case Accidental::Flat:
        return SpellCost::ACC_FLAT;
        // case Acc::DoubleSharp:
        // case Acc::DoubleFlat:  return SpellCost::ACC_DOUBLE;
    }
    return 10;
}

static inline int rarityCostInt(const SpelledNote &sn)
{
    if (sn.letter == Letter::F && sn.acc == Accidental::Flat)
        return SpellCost::RARITY_F_FLAT;
    if (sn.letter == Letter::E && sn.acc == Accidental::Sharp)
        return SpellCost::RARITY_E_SHARP;
    if (sn.letter == Letter::B && sn.acc == Accidental::Sharp)
        return SpellCost::RARITY_B_SHARP;
    if (sn.letter == Letter::C && sn.acc == Accidental::Flat)
        return SpellCost::RARITY_C_FLAT;
    return 0;
}

static inline int signFlipCostInt(Accidental a, Accidental b)
{
    int sa = static_cast<int>(a), sb = static_cast<int>(b);
    if (sa == 0 || sb == 0)
        return 0;
    return ((sa > 0) != (sb > 0)) ? SpellCost::SIGN_FLIP : 0;
}

// keep in sync with nearestDistToAllowed
// nominal semitones mask per diatonic step (bit s set if allowed)
// basically if specified here, no penalty is incurred.
// for example [C# E]... we want to prefer 2 steps (C# to E) rather than 1 step (Db to E).
static inline uint16_t allowedMaskForStep(int step)
{
    // Using a 16-bit mask so we can include bit 12 for octave matching on step 0.
    switch (step)
    {
    case 0:
        return (1u << 0) | (1u << 12); // unison/octave
    case 1:
        return (1u << 1) | (1u << 2); // | (1u << 3);     // m2/M2
    case 2:
        return (1u << 3) | (1u << 4); // m3/M3
    case 3:
        return (1u << 5) | (1u << 6); // P4/tritone
    case 4:
        return (1u << 7); // P5
    case 5:
        return (1u << 8) | (1u << 9); // m6/M6
    case 6:
        return (1u << 10) | (1u << 11); // m7/M7
    }
    return 0;
}

// keep in sync with allowedMaskForStep
// Distance to nearest allowed semitone for a given diatonic step
// "if your actual semitone distance ΔS isn’t one of those allowed values, how far (in semitones) is it from the nearest
// allowed one?" we add that to the mismatch penalty, so the farther you are from a normal interval, the more it hurts.
static inline int nearestDistToAllowed(int step, int semis)
{
    switch (step)
    {
    case 0:
        return std::min(std::abs(semis - 0), std::abs(semis - 12));
    case 1:
        return std::min(std::abs(semis - 1), std::abs(semis - 2));
    case 2:
        return std::min(std::abs(semis - 3), std::abs(semis - 4));
    case 3:
        return std::min(std::abs(semis - 5), std::abs(semis - 6));
    case 4:
        return std::abs(semis - 7);
    case 5:
        return std::min(std::abs(semis - 8), std::abs(semis - 9));
    case 6:
        return std::min(std::abs(semis - 10), std::abs(semis - 11));
    }
    return 3;
}

// Pairwise interval readability cost (non-negative; can include micro-bonuses via negative constants)
static inline int pairIntervalCostInt(const SpelledNote &a, const SpelledNote &b)
{
    int dMidi = (int)b.midi.GetMidiValue() - (int)a.midi.GetMidiValue();
    if (dMidi < 0)
        dMidi = -dMidi;
    int dS = dMidi % 12; // delta semitones

    int dL = letterIndex(b.letter) - letterIndex(a.letter);
    if (dL < 0)
        dL += 7; // delta letters

    // Strong penalty for augmented/diminished unisons (same letter, semitone != 0)
    if (dL == 0 && dS != 0)
    {
        return SpellCost::AUG_DIM_UNISON; // + SpellCost::AUG_DIM_BONUS;
    }

    // Normal match?
    if ((allowedMaskForStep(dL) >> dS) & 1u)
    {
        // if (dL == 1 && dS == 1) return SpellCost::BONUS_TRUE_M2;  // prefer diatonic m2 (e.g., prefer C -> Db)
        return 0; // SpellCost::BONUS_NORMAL_INT;
    }

    // Mismatch: base + distance to nearest allowed + tiny sting
    return SpellCost::BASE_MISMATCH +
           SpellCost::DISTANCE_TO_BASE * nearestDistToAllowed(dL, dS); // + SpellCost::AUG_DIM_BONUS;
}

// =================== Viterbi (min-sum DP) ===================

struct Candidate
{
    SpelledNote note;
    int selfCost; // per-note (accidental) cost
};

// Spells an arbitrary set of MIDI notes (keyless, interval-aware).
// Input order is preserved in the output.
SpellResult spellChord(const fixed_vector<MidiNote, MAX_SYNTH_VOICES> &midiIn)
{
    struct Item
    {
        MidiNote midi;
        int idx;
    };
    fixed_vector<Item, MAX_SYNTH_VOICES> items;
    for (int i = 0; i < (int)midiIn.size(); ++i)
    {
        items.push_back({midiIn[i], i});
    }

    // Sort by MIDI ascending for interval chain
    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
        return a.midi.GetMidiValue() < b.midi.GetMidiValue();
    });

    // Candidates per sorted note
    fixed_vector<fixed_vector<Candidate, MAX_CANDIDATES_PER_PITCH>, MAX_SYNTH_VOICES> C;
    for (auto it : items)
    {
        auto candNotes = generateCandidatesForMidi(it.midi);
        fixed_vector<Candidate, MAX_CANDIDATES_PER_PITCH> cs;
        for (auto &sn : candNotes)
        {
            cs.push_back(Candidate{sn, accidentalCostInt(sn) + rarityCostInt(sn)});
        }
        if (cs.empty())
        {
            SpelledNote sn{Letter::C,
                           Accidental::Natural,
                           // static_cast<int8_t>(midiToOctave(it.midi)),
                           it.midi};
            cs.push_back(Candidate{sn, accidentalCostInt(sn) + rarityCostInt(sn)});
        }
        C.push_back(std::move(cs));
    }

    const int INF = 0x7ffffff;
    fixed_vector<fixed_vector<int, MAX_CANDIDATES_PER_PITCH>, MAX_SYNTH_VOICES> dp;      //(C.size());
    fixed_vector<fixed_vector<int8_t, MAX_CANDIDATES_PER_PITCH>, MAX_SYNTH_VOICES> prev; //(C.size());
    for (size_t i = 0; i < C.size(); ++i)
    {
        dp.push_back(fixed_vector<int, MAX_CANDIDATES_PER_PITCH>());
        dp[i].assign(C[i].size(), INF);
        prev.push_back(fixed_vector<int8_t, MAX_CANDIDATES_PER_PITCH>());
        prev[i].assign(C[i].size(), -1);
    }

    // Init
    if (!C.empty())
    {
        for (size_t k = 0; k < C[0].size(); ++k)
        {
            dp[0][k] = C[0][k].selfCost;
        }
    }

    // Transitions
    for (size_t i = 1; i < C.size(); ++i)
    {
        for (size_t k = 0; k < C[i].size(); ++k)
        {
            const auto &cur = C[i][k].note;
            int sc = C[i][k].selfCost;

            int best = INF;
            int8_t bestj = -1;
            for (size_t j = 0; j < C[i - 1].size(); ++j)
            {
                const auto &prv = C[i - 1][j].note;
                int cost = dp[i - 1][j] + pairIntervalCostInt(prv, cur) +
                           signFlipCostInt(prv.acc, cur.acc)
                           //+ sameLetterAdjPenalty(prv, cur)
                           + sc;
                if (cost < best)
                {
                    best = cost;
                    bestj = static_cast<int8_t>(j);
                }
            }
            dp[i][k] = best;
            prev[i][k] = bestj;
        }
    }

    // Backtrack
    fixed_vector<SpelledNote, decltype(C)::capacity()> bestSorted;
    if (!C.empty())
    {
        int best = INF, bestk = -1;
        for (size_t k = 0; k < C.back().size(); ++k)
        {
            if (dp.back()[k] < best)
            {
                best = dp.back()[k];
                bestk = static_cast<int>(k);
            }
        }
        int k = bestk;
        for (int i = (int)C.size() - 1; i >= 0; --i)
        {
            bestSorted.push_back(C[i][k].note);
            k = prev[i][k];
            if (i > 0 && k < 0)
                break; // safety
        }
        std::reverse(bestSorted.begin(), bestSorted.end());
    }

    // Map back to original order + make display strings
    SpellResult out;
    out.notes.resize(items.size());
    for (size_t si = 0; si < items.size(); ++si)
    {
        int originalIdx = items[si].idx;
        const auto &sn = bestSorted[si];
        out.notes[originalIdx] = sn;
    }
    return out;
}
} // namespace clarinoid