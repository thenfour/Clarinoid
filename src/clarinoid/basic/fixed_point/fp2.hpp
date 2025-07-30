#pragma once
#include <type_traits>

namespace clarinoid {

    // format type which exposes
// storage kernels (allowing debug sidecars or inspection)
// operation kernels, defining how operations are performed


// allows only "fast" operations.
struct Kernel32NoProm
{
  // --- storage type picker --------------------------------------------
  template<int TotalBits>
  using storage_t = std::conditional_t<(TotalBits <= 32), int32_t, void>; // void = invalid

  using overflow_tag = struct saturating_t; // or a policy class

  // --- result-format deduction ----------------------------------------
  template<class A, class B>
  using add_fmt = std::conditional_t<(A::int_bits > B::int_bits), A, B>; // usual max(I,F)

  template<class A, class B>
  using mul_fmt = Format<                    // stays 32-bit
    std::min(31, A::int_bits + B::int_bits), // clip so sum≤31
    A::fract_bits + B::fract_bits>;

  // --- primitives ------------------------------------------------------
  template<class FA, class FB>
  static constexpr auto add(typename storage_t<FA::total_bits> a, typename storage_t<FB::total_bits> b)
  {
    using S = typename storage_t<add_fmt<FA, FB>::total_bits>;
    long long sum = static_cast<long long>(a) + b;
    return saturate<S>(sum);
  }

  template<class FA, class FB>
  static constexpr auto mul(typename storage_t<FA::total_bits> a, typename storage_t<FB::total_bits> b)
  {
    using S = typename storage_t<mul_fmt<FA, FB>::total_bits>;
    long long prod = static_cast<long long>(a) * b;
    return saturate<S>(prod >> FB::fract_bits); // one shift only
  }

private:
  template<class S>
  static constexpr S saturate(long long v)
  { /* … */
  }
};
//
//struct KernelElastic
//{
//  template<int W>
//  using storage_t = std::conditional_t<(W <= 32), int32_t, int64_t>;
//
//  // same add_fmt / mul_fmt as before, but without the hard clip
//  // …
//};

template<int I, int F, typename Kernel = KernelElastic>
struct Fixed
{
  using fmt = Format<I, F>;
  using store = typename Kernel<Fixed>::template storage_t<fmt::total_bits>;

  constexpr Fixed() = default;
  explicit constexpr Fixed(store raw)
    : value_(raw)
  {
  }

  // + -------------------------------------------------------------------
  template<class R>
  constexpr auto operator+(const R& rhs) const
  {
    using KF = Kernel<Fixed>;
    using ResFmt = typename KF::template add_fmt<fmt, typename R::fmt>;
    using ResStore = typename KF::template storage_t<ResFmt::total_bits>;
    ResStore raw = KF::template add<fmt, typename R::fmt>(value_, rhs.raw());
    return Fixed<ResFmt::I, ResFmt::F, Kernel>{ raw };
  }

  // * -------------------------------------------------------------------
  template<class R>
  constexpr auto operator*(const R& rhs) const
  { /* similar */
  }

  constexpr store raw() const { return value_; }

private:
  store value_{};
};

}