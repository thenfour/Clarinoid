
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

static const size_t LOOPER_MEMORY_TOTAL_BYTES = 256000; // 384 kb should be enough right?
static const size_t LOOPER_TEMP_BUFFER_BYTES = 8192;// a smaller buffer that's just used for intermediate copy ops

 // check the memory usage menu to see what the value for this should be. it's NOT just 1 per voice or so; it's based on how the graph is processed i believe so just check the value.
 static constexpr size_t AUDIO_MEMORY_TO_ALLOCATE = 15 + 250;

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
