#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/synth/Analysis.hpp>
#include <clarinoid/application/DisplayDefs.hpp>

namespace clarinoid
{

struct VUMeterConfig
{
    RectI rcDisplay = {0, 0, 24, 24};
};

void RenderVUMeterHoriz(const AudioAnalysisState &state, const VUMeterConfig &config)
{
    //
}
} // namespace clarinoid
