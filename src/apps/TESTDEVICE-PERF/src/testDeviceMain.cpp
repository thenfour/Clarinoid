
#define AUDIO_BLOCK_SAMPLES 128

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <Arduino.h>

#pragma GCC diagnostic pop

#include "./harness.hpp"

#include <clarinoid/core/basic/Basic.hpp>

// todo:
// - support various signature types (int16, int32, uint32, 2-op, 3-op, etc...
// maybe group tests by signature type?)
// - better test discovery (simple registration function...)
// - operate on FxValue<> instead of uint32_t
// - output error graph over the test range


constexpr size_t N = 1024;
char buf[N] = {};

void TestCrc32()
{
  // fill buffer
  //strcpy((char*)buf, "123");  // fill with some data
  for (size_t i = 0; i < N; ++i)
  {
    buf[i] = uint8_t(i * 131u + 7u);
  }

  delay(100);
  uint32_t t0, t1;

  // DCP
  //clarinoid::CycleTimer timer;

  // Software
  //timer.Restart();
  clarinoid::format_to_serial("Software CRC32 (1):\n");
  //auto len = strlen(buf);
  {
    clarinoid::NoInterrupts noInt;
    uint32_t c_sw = 100;
    clarinoid::Teensy::cycles_enable();
    c_sw = clarinoid::crc32_sw_s("123456789");
    auto elapsed = clarinoid::Teensy::cycles_now();
    //auto elapsed = timer.ElapsedCycles();
    clarinoid::format_to_serial("Software CRC32: 0x{:08X} ({} cycles)\n", c_sw, elapsed);
  }

  // Software
  //timer.Restart();
  clarinoid::format_to_serial("Software CRC32 (many):\n");
  //auto len = strlen(buf);
  {
    clarinoid::NoInterrupts noInt;
    uint32_t c_sw = 100;
    clarinoid::Teensy::cycles_enable();
    c_sw = clarinoid::crc32_sw_s("1209824035982405982405982049582049852094850294850294850294850294852094852094852094852"
                                 "09485209485209485209485209485029485209485209485204958");
    auto elapsed = clarinoid::Teensy::cycles_now();
    //auto elapsed = timer.ElapsedCycles();
    clarinoid::format_to_serial("Software CRC32(many): 0x{:08X} ({} cycles)\n", c_sw, elapsed);
  }

  clarinoid::format_to_serial("Software CRC32 (1 again):\n");
  {
    clarinoid::NoInterrupts noInt;
    uint32_t c_sw = 100;
    clarinoid::Teensy::cycles_enable();
    c_sw = clarinoid::crc32_sw_s("123456789");
    auto elapsed = clarinoid::Teensy::cycles_now();
    //auto elapsed = timer.ElapsedCycles();
    clarinoid::format_to_serial("Software CRC32(1): {} {:#08X} ({} cycles)\n", c_sw, c_sw, elapsed);
  }

  auto runTest = [&](const char* str)
  {
    clarinoid::NoInterrupts noInt;
    uint32_t c_sw = 100;
    clarinoid::Teensy::cycles_enable();
    c_sw = clarinoid::crc32_sw_s(str);
    auto elapsed = clarinoid::Teensy::cycles_now();
    clarinoid::format_to_serial("Software CRC32: {} {:#08x} ({} cycles)\n", str, c_sw, elapsed);
  };

  runTest("");
  runTest("1");
  runTest("123456789");
  runTest("123456789saonthaostnhaoru cihaou crihau icrahusi ahuscrihasoeurciho aesurichoesuntih oeusntih oesutniho "
          "seuntiho esuntiho esutniho esuntiho esuntiho esutnioh eustniho esutiho seutiho seutiho seutniho seutiho "
          "seutniho seuntih rch,.f3059fg23 559fh3 59fh35 f5");
  runTest("a");
  runTest("abc");


  // auto runWyhashTest = [&](const char* str)
  // {
  //   clarinoid::NoInterrupts noInt;
  //   uint32_t c_sw = 100;
  //   clarinoid::Teensy::cycles_enable();
  //   c_sw = clarinoid::wyhash32_s(str);
  //   auto elapsed = clarinoid::Teensy::cycles_now();
  //   clarinoid::format_to_serial("Software Wyhash: {} {:#08x} ({} cycles)\n", str, c_sw, elapsed);
  // };

  // runWyhashTest("");
  // runWyhashTest("1");
  // runWyhashTest("123456789");
  // runWyhashTest(
  //     "123456789saonthaostnhaoru cihaou crihau icrahusi ahuscrihasoeurciho aesurichoesuntih oeusntih oesutniho "
  //     "seuntiho esuntiho esutniho esuntiho esuntiho esutnioh eustniho esutiho seutiho seutiho seutniho seutiho "
  //     "seutniho seuntih rch,.f3059fg23 559fh3 59fh35 f5");
  // runWyhashTest("a");
  // runWyhashTest("abc");


  // auto runXxh32Test = [&](const char* str)
  // {
  //   clarinoid::NoInterrupts noInt;
  //   uint32_t c_sw = 100;
  //   clarinoid::Teensy::cycles_enable();
  //   c_sw = clarinoid::xxh32_s(str);
  //   auto elapsed = clarinoid::Teensy::cycles_now();
  //   clarinoid::format_to_serial("Software Xxh32: {} {:#08x} ({} cycles)\n", str, c_sw, elapsed);
  // };

  // runXxh32Test("");
  // runXxh32Test("1");
  // runXxh32Test("123456789");
  // runXxh32Test(
  //     "123456789saonthaostnhaoru cihaou crihau icrahusi ahuscrihasoeurciho aesurichoesuntih oeusntih oesutniho "
  //     "seuntiho esuntiho esutniho esuntiho esuntiho esutnioh eustniho esutiho seutiho seutiho seutniho seutiho "
  //     "seutniho seuntih rch,.f3059fg23 559fh3 59fh35 f5");
  // runXxh32Test("a");
  // runXxh32Test("abc");


  // // CRC peripheral
  // timer.Restart();
  // uint32_t c_reg = clarinoid::crc32(buf, N);
  // clarinoid::format_to_serial("CRC peripheral CRC32: 0x{:08X} ({} cycles)\n", c_reg, timer.ElapsedCycles());


  // timer.Restart();
  // uint32_t c_dcp = clarinoid::crc32_dcp(buf, N);
  // clarinoid::format_to_serial("DCP CRC32: 0x{:08X} ({} cycles)\n", c_dcp, timer.ElapsedCycles());
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }

  TestCrc32();
  return;

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
