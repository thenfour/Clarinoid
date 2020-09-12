#pragma once

#include <Shared_CCUtil.h>
#include "AppSettings.h"
#include "Harmonizer.hpp"

// todo:
// - buffer of recently played notes, ability to remove them and "replay" the scale following without this transient note.
// - LUTs for each scale flavor; every of 12 notes, a new scale connection.
// - thinking about marking notes as transient, or marking scale degrees as "character notes", where the scale would only be selected if the color note is present.
// - need "hidden" scales which specify certain ambiguous contexts like just 1-5, or just 1. Or chords like C7, when b9 is not really known.
// - if we select a scale, and a recently played not is NOT in this scale, then remove it. This will keep our list of context notes short enough.

struct ScaleFollower
{
    // Call to "feed" musical context. This will set gAppSettings.DeducedScale.
    // we can decide later if it's better to feed it STATE or EVENTS
    void Update(const MusicalVoice* voices, size_t voiceCount) {
        //
    }
};

static ScaleFollower gScaleFollower;
