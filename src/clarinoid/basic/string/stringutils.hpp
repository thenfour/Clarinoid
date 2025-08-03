
#pragma once

#include "../Array.hpp"
#include "./ieeefloat.hpp"
#include "./stringutils.hpp"

namespace clarinoid {

inline char ParseDigit(unsigned char d) {
  static const char Digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  return d < SizeofStaticArray(Digits) ? Digits[d] : 0;
}

// Naive string upper / lower functions
// --------------------------------------------------------------------------------------
template <typename Char> void NaiveCharToLower(Char &c) {
  if (c >= 'A' && c <= 'Z')
    c += 'a' - 'A';
}

} // namespace clarinoid
