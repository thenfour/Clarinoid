// crc32_teensy.hpp  — C++17
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

// see C:\Users\carl\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0\cores\teensy4
//#include <arm_math.h>
//#include <imxrt.h>

namespace clarinoid
{


// Build the 8x256 tables on first use (cheap on M7; avoids huge flash blob).
inline uint32_t* tables()
{
  static uint32_t T[8][256];
  static bool inited = false;
  if (!inited)
  {
    // T[0][n] for reflected poly 0xEDB88320
    for (uint32_t n = 0; n < 256; ++n)
    {
      uint32_t c = n;
      for (int k = 0; k < 8; ++k)
        c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      T[0][n] = c;
    }
    // Derive T[1..7]
    for (int i = 1; i < 8; ++i)
    {
      for (uint32_t n = 0; n < 256; ++n)
      {
        T[i][n] = (T[i - 1][n] >> 8) ^ T[0][T[i - 1][n] & 0xFF];
      }
    }
    inited = true;
  }
  return &T[0][0];
}

// One-shot, returns final CRC (same as zlib/ZIP/Ethernet):
//   crc32_ieee("123456789", 9) == 0xCBF43926
inline uint32_t crc32_sw(const void* data, size_t len, uint32_t seed = 0xFFFFFFFFu)
{
  auto* T = tables();  // flat ptr to T[8][256]
  auto IDX = [&](int t, uint32_t b) -> uint32_t&
  {
    return T[(t << 8) + b];
  };

  const uint8_t* p = static_cast<const uint8_t*>(data);
  uint32_t crc = seed;  // seed is usually 0xFFFFFFFF; invert so we can ~ at the end

  // Align to 8
  while (len && (reinterpret_cast<uintptr_t>(p) & 7))
  {
    crc = IDX(0, (crc ^ *p++) & 0xFF) ^ (crc >> 8);
    --len;
  }

  // Process 8 bytes at a time
  while (len >= 8)
  {
    uint64_t v;
    std::memcpy(&v, p, 8);
    p += 8;
    len -= 8;

    uint32_t lo = static_cast<uint32_t>(v);
    uint32_t hi = static_cast<uint32_t>(v >> 32);

    crc ^= lo;
    uint32_t t = IDX(7, (crc) & 0xFF) ^ IDX(6, (crc >> 8) & 0xFF) ^ IDX(5, (crc >> 16) & 0xFF) ^ IDX(4, (crc >> 24)) ^
                 IDX(3, (hi) & 0xFF) ^ IDX(2, (hi >> 8) & 0xFF) ^ IDX(1, (hi >> 16) & 0xFF) ^ IDX(0, (hi >> 24));
    crc = t;
  }

  // Tail
  while (len--)
  {
    crc = IDX(0, (crc ^ *p++) & 0xFF) ^ (crc >> 8);
  }

  return ~crc;
}

template <size_t N>
inline uint32_t crc32_sw_s(const char (&data)[N], uint32_t crc = 0xFFFFFFFFu)
{
  return crc32_sw(data, N - 1, crc);  // Exclude null terminator
}

inline uint32_t crc32_sw_s(const char* data, uint32_t crc = 0xFFFFFFFFu)
{
  return crc32_sw(data, std::strlen(data), crc);
}

// Convert CRC32 to a hash32 (for use in hash tables, etc.) -- instead of a checksum, it's a random hash.
inline uint32_t crc32_to_hash32(uint32_t crc)
{
  crc ^= crc >> 16;
  crc *= 0x7FEB352Du;
  crc ^= crc >> 15;
  crc *= 0x846CA68Bu;
  crc ^= crc >> 16;
  return crc;
}

}  // namespace clarinoid
