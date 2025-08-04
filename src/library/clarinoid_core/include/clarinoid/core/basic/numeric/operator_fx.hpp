
#pragma once

namespace clarinoid {
namespace fxl::detail {

constexpr std::uint64_t pow10_u64(unsigned p) {
  return p > 19 ? std::uint64_t{0} : // sentinel = overflow
             (p == 0 ? 1 : pow10_u64(p - 1) * 10);
}

// ----------------------------------------- split sign / body -----------
template <bool Neg, char... Cs> struct signed_chars;
template <char C, char... Cs>
struct signed_chars<false, C, Cs...> : signed_chars<(C == '-'), Cs...> {
}; // flip flag if '-'
template <char... Cs> struct signed_chars<true, Cs...> { // body w/out sign
  using body = std::integer_sequence<char, Cs...>;
};

// ----------------------------------------- locate 'e' or '.' -----------
template <char... Cs> struct scan;
template <char C, char... Rest> struct scan<C, Rest...> {
  static constexpr int dot =
      C == '.' ? 0 : (scan<Rest...>::dot < 0 ? -1 : scan<Rest...>::dot + 1);
  static constexpr int expo =
      (C == 'e' || C == 'E')
          ? 0
          : (scan<Rest...>::expo < 0 ? -1 : scan<Rest...>::expo + 1);
};
template <> struct scan<> {
  static constexpr int dot = -1, expo = -1;
};

// ----------------------------------- gather digits as uint64 ----------
template <char... Ds> constexpr std::uint64_t digits_to_u64() {
  std::uint64_t v = 0;
  ((v = v * 10 + (Ds - '0')), ...);
  return v;
}

// ----------------------------------- master parse ----------------------
template <char... Cs> struct parsed {
  // strip leading sign
  static constexpr bool negative =
      (sizeof...(Cs) && ((Cs == '-') || ... && false)); // first char check

  using body_chars = typename signed_chars<false, Cs...>::body;
  // explode body_chars again
  template <char... Bs> struct body : scan<Bs...> {
    // indices
    static constexpr int dot_pos = scan<Bs...>::dot;
    static constexpr int expo_pos = scan<Bs...>::expo;

    // ---- pick integer / frac / exponent substrings ----------------
    template <int From, int To, char... All> struct slice;
    template <int From, int To, char H, char... T, char... Acc>
    struct slice<From, To, H, T..., Acc...>
        : slice<From - 1, To - 1, T..., Acc...,
                (From <= 0 && To >= 0 ? H : '\0')> {};
    template <int To, char... Acc>
    struct slice<0, To, Acc...> : std::integer_sequence<char, Acc...> {};

    // segments
    using int_chars =
        typename slice<0,
                       (dot_pos >= 0
                            ? dot_pos - 1
                            : (expo_pos >= 0 ? expo_pos - 1
                                             : int(sizeof...(Bs)) - 1)),
                       Bs...>::type;

    using frac_chars = typename std::conditional_t<
        (dot_pos >= 0),
        slice<dot_pos + 1,
              (expo_pos >= 0 ? expo_pos - 1 : int(sizeof...(Bs)) - 1), Bs...>,
        std::integer_sequence<char>>::type;

    using exp_chars = typename std::conditional_t<
        (expo_pos >= 0), slice<expo_pos + 1, int(sizeof...(Bs)) - 1, Bs...>,
        std::integer_sequence<char>>::type;

    // ---- convert segments to integer values -----------------------
    static constexpr std::uint64_t int_part = digits_to_u64<int_chars{}>();

    static constexpr std::uint64_t frac_part = digits_to_u64<frac_chars{}>();

    static constexpr std::int32_t exp10 = ([] {
      // empty = 0
      if constexpr (exp_chars{}.size() == 0)
        return 0;
      // parse exponent, allow leading '+/-'
      std::int32_t e = 0;
      bool neg = false;
      constexpr auto chars = exp_chars{};
      for (char c : chars) {
        if (c == '-')
          neg = true;
        else if (c != '+')
          e = e * 10 + (c - '0');
      }
      return neg ? -e : e;
    })();

    static constexpr std::size_t frac_len = frac_chars{}.size();
  };

  using B = body_chars{}; // instantiate

  // ------------- scaled decimal → exact rational ---------------------
  static constexpr std::uint64_t num =
      (B::int_part * pow10_u64(B::frac_len) + B::frac_part);

  static constexpr std::int32_t dec_exp =
      B::exp10 - B::frac_len; // net 10-exponent

  // multiply/divide by 10^exp to fold exponent
  static constexpr std::uint64_t num_scaled =
      dec_exp >= 0 ? num * pow10_u64(dec_exp) : num / pow10_u64(-dec_exp);

  static constexpr bool has_frac = (B::frac_len != 0) || (dec_exp < 0);
  static constexpr std::uint64_t abs_val = num_scaled;
};

template <char... Cs> constexpr auto make_fixed_from_chars() {
  namespace dt = fxl::detail;
  using P = dt::parsed<Cs...>;

  static_assert(pow10_u64(19) != 0, "pow10 sentinel check");
  static_assert(P::abs_val != 0, "literal too large for 64-bit");

  // ---- choose integer magnitude bits -----------------------------------
  constexpr int mag_bits = dt::needed_int_bits(P::abs_val);

  // ---- choose storage width --------------------------------------------
  constexpr bool needs_sign = true;
  constexpr int total32 = mag_bits + (needs_sign ? 1 : 0);
  using store_t =
      std::conditional_t<(total32 <= 32), std::int32_t, std::int64_t>;
  constexpr int total_bits = std::numeric_limits<store_t>::digits;

  // ---- fractional bits --------------------------------------------------
  constexpr int fract_bits =
      P::has_frac ? (total_bits - mag_bits - (needs_sign ? 1 : 0)) : 0;

  // ---- scale ------------------------------------------------------------
  constexpr std::uint64_t scale = 1ULL << fract_bits;
  constexpr std::uint64_t raw_u =
      (P::num_scaled * scale + 5) / 10; // +0.5 rounding
  constexpr store_t raw =
      static_cast<store_t>(P::negative ? -static_cast<std::int64_t>(raw_u)
                                       : static_cast<std::int64_t>(raw_u));

  using F = Fixed<mag_bits, fract_bits, store_t>;
  return F::FromRaw(raw);
}

} // namespace fxl::detail

namespace fxl::literals {
template <char... Cs> constexpr auto operator"" _fx() {
  return make_fixed_from_chars<Cs...>();
}
} // namespace fxl::literals

// using namespace fxl::literals;
//
// static_assert(std::is_same_v<decltype(3_fx), clarinoid::Fixed<2, 0,
// int32_t>>);
//
// static_assert(std::is_same_v<
//               decltype(-1.234e-2_fx),
//               clarinoid::Fixed<1, 30, int32_t>>); // chose 1 mag-bit, rest
//               fract
//
// auto a = 42_fx;       // Fixed<6,0>  raw = 42
// auto b = -3.14159_fx; // Fixed<2,29> raw ≈ -0x6487ED
// auto c = 1e6_fx;      // promotes to int64 (needs 20 bits mag)
//

} // namespace clarinoid