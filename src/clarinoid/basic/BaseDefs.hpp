

#pragma once

namespace clarinoid {

// clarinoid custom fonts will define these characters
#define CHARSTR_DB "\x7f"
#define CHARSTR_QEQ "\x80"
#define CHARSTR_INFINITY "\x81"
#define CHARSTR_SHARP "\x82"
#define CHARSTR_FLAT "\x83"
#define CHARSTR_NARROWPLUS "\x84"
#define CHARSTR_NARROWMINUS "\x85"
#define CHARSTR_NARROWPLUSMINUS "\x86"
#define CHARSTR_DIGITWIDTHSPACE "\x87"

template<typename T, size_t N>
struct StaticArray
{
  T (&mArray)[N];
  static constexpr size_t Size = N;
  StaticArray(T (&x)[N])
    : mArray(x)
  {
  }
};

template<typename T, size_t N>
constexpr size_t
SizeofStaticArray(const T (&x)[N])
{
  return N;
}

template<typename T, size_t N>
void
CopyPODArray(const T (&from)[N], T (&to)[N])
{
  memcpy(to, from, sizeof(T) * N);
}

template<typename T>
void
CopyPODArray(const T* from, T* to, size_t N)
{
  memcpy(to, from, sizeof(T) * N);
}

struct IMetronome
{
  virtual uint32_t GetBeatInt() const = 0;
  virtual float GetBeatFrac() const = 0;
  virtual float GetBeatFloat() const = 0;
  virtual void OnBPMChanged() = 0;
};

} // namespace clarinoid
