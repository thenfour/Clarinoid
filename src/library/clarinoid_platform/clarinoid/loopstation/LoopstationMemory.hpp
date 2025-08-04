
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

// .../AudioStream.h:107:30: error: data causes a section type conflict with gLoopStationBuffer
// https://stackoverflow.com/questions/30076949/gcc-error-variable-causes-a-section-type-conflict
// one cheap solution is to just put all our DMAMEM stuff in 1 struct like this.
struct DMAClarinoidMemory
{
    uint8_t gLoopStationBuffer[LOOPER_MEMORY_TOTAL_BYTES];
    uint8_t gLoopStationTempBuffer[LOOPER_TEMP_BUFFER_BYTES];
#ifndef CLARINOID_MODULE_TEST
    audio_block_t gAudioMemory[AUDIO_MEMORY_TO_ALLOCATE];
#endif // CLARINOID_MODULE_TEST
};
static DMAMEM DMAClarinoidMemory gClarinoidDmaMem;

// #define LOOPSTATION_BUFFER (gLoopStationBuffer)
// #define LOOPSTATION_TEMP_BUFFER (gLoopStationTempBuffer)
#define LOOPSTATION_BUFFER (gClarinoidDmaMem.gLoopStationBuffer)
#define LOOPSTATION_TEMP_BUFFER (gClarinoidDmaMem.gLoopStationTempBuffer)
#define CLARINOID_AUDIO_MEMORY (gClarinoidDmaMem.gAudioMemory)

} // namespace clarinoid
