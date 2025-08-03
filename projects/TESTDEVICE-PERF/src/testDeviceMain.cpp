
#define AUDIO_BLOCK_SAMPLES 128

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <Arduino.h>

#pragma GCC diagnostic pop

#include "harness.hpp"


// todo:
// - support various signature types (int16, int32, uint32, 2-op, 3-op, etc... maybe group tests by signature type?)
// - better test discovery (simple registration function...)
// - operate on FxValue<> instead of uint32_t

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 4000)
  {
  }
  Serial.println("\n--- Modular benchmark harness (Teensy 4.x) ---");
#if HAVE_DWT
  Serial.println("Timing: DWT cycle counter");
#else
  Serial.println("Timing: micros() fallback (coarser)");
#endif
  Serial.printf("Random=%u  Sweep=%u  Edge=%s\n", (unsigned)kNumRandom, (unsigned)kNumSweep, kDoEdge ? "yes" : "no");

  // Run all registered UQF ops
  for (uint32_t i = 0; i < kNumOps_UQF; ++i)
  {
    run_bench(kOps_UQF[i], kNumRandom, kNumSweep, kDoEdge);
  }

  // Run all registered float ops (uncomment when you add entries)
  for (uint32_t i = 0; i < kNumOps_Float; ++i)
  {
    run_bench(kOps_Float[i], kNumRandom, kNumSweep);
  }

  Serial.println("\n### Summary (Markdown-ready)\n");
  print_markdown_summaries();

  Serial.println("\nDone.");
}

void loop() {}
