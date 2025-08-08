#pragma once

#include "./fp2.hpp"
#include <utility>

namespace clarinoid
{
namespace fxl::detail
{

constexpr int needed_int_bits(std::uint64_t value)
{
  if (value == 0)
    return 1;
  int bits = 0;
  while (value)
  {
    value >>= 1;
    ++bits;
  }
  return bits;
}

constexpr std::uint64_t pow10_u64(unsigned p)
{
  return p > 19 ? std::uint64_t{0} : (p == 0 ? 1 : pow10_u64(p - 1) * 10);
}

// -------------------------------------------------- master parsed -----
// Greatly simplified: single pass constexpr parser instead of heavy template slicing.
// Supports forms: [-]? [digits] [ . [digits] ] [ (e|E) [+-]? digits ]
// Produces rational numerator/denominator and magnitude for bit sizing.

template <char... Cs>
struct parsed
{
  static constexpr std::size_t N = sizeof...(Cs);
  static constexpr char chars[N + 1] = {Cs..., '\0'};

  static constexpr bool negative = (N > 0 && chars[0] == '-');

  static constexpr auto compute() noexcept
  {
    struct R
    {
      bool neg;
      std::uint64_t int_part;
      std::uint64_t frac_part;
      std::size_t frac_len;
      int exp10;
      bool has_frac;
      std::uint64_t numerator;
      std::uint64_t denominator;
      std::uint64_t abs_val;
    };
    R r{};
    r.neg = negative;
    r.int_part = 0;
    r.frac_part = 0;
    r.frac_len = 0;
    r.exp10 = 0;
    std::size_t i = r.neg ? 1u : 0u;
    auto is_digit = [](char c)
    {
      return c >= '0' && c <= '9';
    };
    // integer digits
    while (i < N && is_digit(chars[i]))
    {
      r.int_part = r.int_part * 10 + (chars[i] - '0');
      ++i;
    }
    // fractional
    if (i < N && chars[i] == '.')
    {
      ++i;
      while (i < N && is_digit(chars[i]))
      {
        if (r.frac_len < 19)
        {  // cap to 19 digits (fits in 64b and aligns pow10 sentinel)
          r.frac_part = r.frac_part * 10 + (chars[i] - '0');
          ++r.frac_len;
        }
        else
        {
          ++r.frac_len;
        }  // ignore excess digits (implicit rounding later)
        ++i;
      }
    }
    // exponent
    if (i < N && (chars[i] == 'e' || chars[i] == 'E'))
    {
      ++i;
      bool exp_neg = false;
      if (i < N && (chars[i] == '+' || chars[i] == '-'))
      {
        exp_neg = (chars[i] == '-');
        ++i;
      }
      int e = 0;
      while (i < N && is_digit(chars[i]))
      {
        e = e * 10 + (chars[i] - '0');
        ++i;
        if (e > 1000)
          break;
      }
      r.exp10 = exp_neg ? -e : e;
    }
    // packed integer value before applying net exponent
    // V = (int_part + frac_part / 10^{frac_len}) * 10^{exp10}
    // => packed = int_part*10^{frac_len} + frac_part; net exp = exp10 - frac_len
    auto pow10_frac = pow10_u64((unsigned)(r.frac_len <= 19 ? r.frac_len : 19));
    std::uint64_t effective_frac_part = r.frac_part;  // excess digits ignored above
    std::uint64_t packed = (r.int_part * pow10_frac) + effective_frac_part;
    int net_exp = r.exp10 - (int)r.frac_len;
    if (net_exp >= 0)
    {
      auto p10 = pow10_u64((unsigned)net_exp);
      r.numerator = p10 ? packed * p10 : 0;  // overflow -> 0 sentinel
      r.denominator = 1;
    }
    else
    {
      auto p10 = pow10_u64((unsigned)(-net_exp));
      r.numerator = packed;
      r.denominator = p10 ? p10 : 0;  // sentinel if too big
    }
    r.has_frac = (r.frac_len != 0) || (net_exp < 0);
    r.abs_val = (r.denominator == 1) ? r.numerator : (r.denominator ? r.numerator / r.denominator : 0);
    return r;
  }

  static constexpr auto info = compute();
  static constexpr bool valid = (info.numerator != 0 || (!info.neg && info.abs_val == 0));
  static constexpr bool neg = info.neg;
  static constexpr std::uint64_t numerator = info.numerator;
  static constexpr std::uint64_t denominator = info.denominator;
  static constexpr bool has_frac = info.has_frac;
  static constexpr std::uint64_t abs_val = info.abs_val;
};

template <char... Cs>
constexpr auto make_fixed_from_chars()
{
  using P = parsed<Cs...>;
  static_assert(P::denominator != 0, "literal exponent too large");
  static_assert(P::abs_val != 0 || !P::neg, "zero or overflow");
  constexpr int mag_bits = needed_int_bits(P::abs_val == 0 ? 1 : P::abs_val);
  constexpr bool needs_sign = true;
  using store_t = OptimalRawType_t<needs_sign, mag_bits, P::has_frac ? (32 - mag_bits - (needs_sign ? 1 : 0)) : 0>;
  constexpr int total_bits = std::numeric_limits<store_t>::digits;
  constexpr int fract_bits = P::has_frac ? (total_bits - mag_bits) : 0;
  constexpr std::uint64_t scale = (fract_bits > 0) ? (1ULL << fract_bits) : 1ULL;
#if defined(__SIZEOF_INT128__)
  using u128 = unsigned __int128;
  constexpr std::uint64_t raw_u = P::denominator ? (std::uint64_t)(((u128)P::numerator * scale + (P::denominator / 2)) /
                                                                   P::denominator)
                                                 : 0ULL;
#else
  constexpr bool will_overflow = P::denominator && (P::numerator > (std::numeric_limits<std::uint64_t>::max() / scale));
  constexpr std::uint64_t raw_u = will_overflow
                                      ? std::numeric_limits<std::uint64_t>::max()
                                      : (P::denominator
                                             ? ((P::numerator * scale + (P::denominator / 2)) / P::denominator)
                                             : 0ULL);
#endif
  constexpr store_t raw = static_cast<store_t>(P::neg ? -static_cast<std::int64_t>(raw_u)
                                                      : static_cast<std::int64_t>(raw_u));
  using Layout = FxLayout<mag_bits, fract_bits>;
  using Storage = FxStorageTraits<store_t>;
  using Format = FxFormat<Layout, Storage>;
  using FixedType = Fixed<Format, FxSmartKernel>;
  return FixedType::FromRaw(raw);
}

}  // namespace fxl::detail

namespace fxl::literals
{
template <char... Cs>
constexpr auto operator"" _fx()
{
  return fxl::detail::make_fixed_from_chars<Cs...>();
}
}  // namespace fxl::literals

}  // namespace clarinoid