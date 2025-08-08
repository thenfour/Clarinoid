#pragma once

#include "./fp2.hpp"
#include <utility>

namespace clarinoid
{
namespace fxl::detail
{

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
    return []
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
      //auto pow10_u64 = [](unsigned p)
      //{
      //  return p > 19 ? std::uint64_t{0} : (p == 0 ? 1 : ((std::uint64_t)10 * pow10_u64(p - 1)));
      //};
      auto pow10_frac = pow10_u64((unsigned)(r.frac_len <= 19 ? r.frac_len : 19));
      std::uint64_t packed = (r.int_part * pow10_frac) + r.frac_part;
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
    }();
  }

  static constexpr auto info = compute();

  static constexpr std::uint64_t int_part = info.int_part;
  static constexpr std::uint64_t frac_part = info.frac_part;
  static constexpr int exp10 = info.exp10;

  static constexpr bool valid = (info.numerator != 0 || (!info.neg && info.abs_val == 0));
  static constexpr bool neg = info.neg;
  static constexpr std::uint64_t numerator = info.numerator;
  static constexpr std::uint64_t denominator = info.denominator;
  static constexpr bool has_frac = info.has_frac;
  static constexpr std::uint64_t abs_val = info.abs_val;
};

// Bridge to kernel decision

template <char... Cs>
constexpr auto make_fixed_from_chars()
{
  using Parsed = parsed<Cs...>;
  static_assert(Parsed::denominator != 0, "literal exponent too large");
  static_assert(Parsed::abs_val != 0 || !Parsed::neg, "zero or overflow");
  using Decision = FxSmartKernel::LiteralDecision<Parsed>;
  using Kernel = FxSmartKernel;
  using FixedType = Fixed<typename Decision::Format, Kernel>;
  return FixedType::FromFxValue(Decision::value);
}

// Expose descriptor for tests

template <char... Cs>
using literal_descriptor = FxSmartKernel::LiteralDecision<parsed<Cs...>>;

}  // namespace fxl::detail

namespace fxl::literals
{
template <char... Cs>
constexpr auto operator"" _fx()
{
  return fxl::detail::make_fixed_from_chars<Cs...>();
}

// Testing: exposes descriptor so tests can static_assert on chosen bits
template <char... Cs>
constexpr auto operator"" _fxd()
{
  using D = fxl::detail::literal_descriptor<Cs...>;
  return D{};
}


}  // namespace fxl::literals

}  // namespace clarinoid