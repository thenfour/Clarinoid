#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <type_traits>


#include "../CLArduino.hpp"

namespace clarinoid
{

//// ---- config/help ------------------------------------------------------------
//
//// C++17: many <cmath> funcs aren't constexpr-evaluable; we still mark constexpr
//// so the compiler can constant-fold what it can.
//
//template <class T>
//using xyz = std::array<T, 3>;
//
//template <class T>
//using rgb = std::array<T, 3>;
//
//// Reference white tags (for future CATs --color adaptation transforms)
//struct D65
//{
//};
//
//template <class T>
//CL_NODISCARD constexpr T min3(T a, T b, T c) noexcept
//{
//  return std::min(a, std::min(b, c));
//}
//
//template <class T>
//CL_NODISCARD constexpr T max3(T a, T b, T c) noexcept
//{
//  return std::max(a, std::max(b, c));
//}
//
//// ---- basic math -------------------------------------------------------------
//
//template <class T>
//CL_NODISCARD constexpr T clamp(T v, T lo, T hi) noexcept
//{
//  return v < lo ? lo : (v > hi ? hi : v);
//}
//
//template <class T>
//CL_NODISCARD constexpr T saturate01(T v) noexcept
//{
//  return clamp(v, T(0), T(1));
//}
//
//template <class T>
//CL_NODISCARD constexpr T lerp(T a, T b, T t) noexcept
//{
//  return a + (b - a) * t;
//}
//
//// For scalar gamma; std::pow might not be constexpr-evaluable in C++17.
//// Still OK for runtime; const-folding may happen in practice.
//template <class T>
//CL_NODISCARD inline T powf(T x, T g) noexcept
//{
//  using std::pow;
//  return pow(x, g);
//}
//
//// ---- per-channel helpers ----------------------------------------------------
//
//template <class T, class TF>
//CL_NODISCARD constexpr rgb<T> map_channels(rgb<T> c, TF&& fn) noexcept(noexcept(fn(T{})))
//{
//  return {fn(c[0]), fn(c[1]), fn(c[2])};
//}
//
//template <class T>
//CL_NODISCARD constexpr rgb<T> add(rgb<T> a, T d) noexcept
//{
//  return {a[0] + d, a[1] + d, a[2] + d};
//}
//
//template <class T>
//CL_NODISCARD constexpr rgb<T> mul(rgb<T> a, T s) noexcept
//{
//  return {a[0] * s, a[1] * s, a[2] * s};
//}
//
//template <class T>
//CL_NODISCARD constexpr rgb<T> clamp01(rgb<T> a) noexcept
//{
//  return {saturate01(a[0]), saturate01(a[1]), saturate01(a[2])};
//}
//
//template <class T>
//CL_NODISCARD constexpr rgb<T> invert01(rgb<T> a) noexcept
//{
//  return {T(1) - a[0], T(1) - a[1], T(1) - a[2]};
//}
//
//template <class T>
//CL_NODISCARD inline rgb<T> gamma(rgb<T> a, T g) noexcept
//{
//  return {powf(a[0], g), powf(a[1], g), powf(a[2], g)};
//}
//
//template <class T>
//CL_NODISCARD constexpr rgb<T> mix(rgb<T> a, rgb<T> b, T t) noexcept
//{
//  return {lerp(a[0], b[0], t), lerp(a[1], b[1], t), lerp(a[2], b[2], t)};
//}
//
//// ---- luminance / Y estimators ----------------------------------------------
//// Rec.709 / sRGB coefficients, linear-light expected.
//template <class T>
//CL_NODISCARD constexpr T luminance709(rgb<T> lin_srgb) noexcept
//{
//  return lin_srgb[0] * T(0.2126) + lin_srgb[1] * T(0.7152) + lin_srgb[2] * T(0.0722);
//}
//
//// ---- HSV helpers (generic, 0..1) -------------------------------------------
//
//template <class T>
//CL_NODISCARD constexpr std::array<T, 3> rgb_to_hsv(rgb<T> c) noexcept
//{
//  const T r = c[0], g = c[1], b = c[2];
//  const T mx = max3(r, g, b), mn = min3(r, g, b);
//  const T d = mx - mn;
//
//  T h = T(0), s = T(0), v = mx;
//  if (mx > T(0))
//    s = d / mx;
//  if (d > T(0))
//  {
//    if (mx == r)
//    {
//      h = (g - b) / d;
//      if (h < T(0))
//        h += T(6);
//    }
//    else if (mx == g)
//    {
//      h = (b - r) / d + T(2);
//    }
//    else
//    {
//      h = (r - g) / d + T(4);
//    }
//    h /= T(6);
//  }
//  return {h, s, v};
//}
//
//template <class T>
//CL_NODISCARD constexpr rgb<T> hsv_to_rgb(std::array<T, 3> hsv) noexcept
//{
//  T h = hsv[0], s = hsv[1], v = hsv[2];
//  if (s == T(0))
//    return {v, v, v};
//
//  h = (h - std::floor(h)) * T(6);
//  const int i = static_cast<int>(h);
//  const T f = h - T(i);
//  const T p = v * (T(1) - s);
//  const T q = v * (T(1) - s * f);
//  const T t = v * (T(1) - s * (T(1) - f));
//
//  switch (i % 6)
//  {
//    case 0:
//      return {v, t, p};
//    case 1:
//      return {q, v, p};
//    case 2:
//      return {p, v, t};
//    case 3:
//      return {p, q, v};
//    case 4:
//      return {t, p, v};
//    default:
//      return {v, p, q};
//  }
//}
//
//// ---- XYZ <-> Linear sRGB (D65) ---------------------------------------------
//// Matrices from IEC 61966-2-1 / sRGB with D65.
//template <class T>
//CL_NODISCARD constexpr xyz<T> lin_srgb_to_xyz(rgb<T> c) noexcept
//{
//  const T r = c[0], g = c[1], b = c[2];
//  return {r * T(0.4123907992659595) + g * T(0.3575843393838780) + b * T(0.1804807884018343),
//          r * T(0.2126390058715104) + g * T(0.7151686787677560) + b * T(0.0721923153607337),
//          r * T(0.0193308187155918) + g * T(0.1191947797946260) + b * T(0.9505321522496606)};
//}
//
//template <class T>
//CL_NODISCARD constexpr rgb<T> xyz_to_lin_srgb(xyz<T> X) noexcept
//{
//  const T x = X[0], y = X[1], z = X[2];
//  return {x * T(3.240969941904521) + y * T(-1.537383177570093) + z * T(-0.498610760293003),
//          x * T(-0.969243636280880) + y * T(1.875967501507720) + z * T(0.041555057407175),
//          x * T(0.055630079696993) + y * T(-0.203976958888977) + z * T(1.056971514242879)};
//}
//
//// sRGB EOTF / OETF (display-referred). These convert between non-linear sRGB
//// and **linear** sRGB. 0..1 domain; unclamped OK.
//template <class T>
//CL_NODISCARD inline T srgb_to_linear_scalar(T v) noexcept
//{
//  return (v <= T(0.04045)) ? (v / T(12.92)) : std::pow((v + T(0.055)) / T(1.055), T(2.4));
//}
//template <class T>
//CL_NODISCARD inline T linear_to_srgb_scalar(T v) noexcept
//{
//  return (v <= T(0.0031308)) ? (v * T(12.92)) : (T(1.055) * std::pow(v, T(1.0 / 2.4)) - T(0.055));
//}
//template <class T>
//CL_NODISCARD inline rgb<T> srgb_to_linear(rgb<T> c) noexcept
//{
//  return {srgb_to_linear_scalar(c[0]), srgb_to_linear_scalar(c[1]), srgb_to_linear_scalar(c[2])};
//}
//template <class T>
//CL_NODISCARD inline rgb<T> linear_to_srgb(rgb<T> c) noexcept
//{
//  return {linear_to_srgb_scalar(c[0]), linear_to_srgb_scalar(c[1]), linear_to_srgb_scalar(c[2])};
//}
//
//
//// ---------- space concept (by convention) ----------
//// Each Space must provide:
////   static constexpr int channels;
////   template<class T> using storage = std::array<T, channels>;   // or custom
////   using white = D65;  // or something else
////   template<class T> static xyz<T> to_xyz(storage<T>) noexcept;
////   template<class T> static storage<T> from_xyz(xyz<T>) noexcept;
//// Optional feature flags:
////   static constexpr bool is_linear = true/false;         // channel math assumes linear [0,1]
////   static constexpr bool supports_hsv = true/false;      // only sensible for 3ch RGB-like
//
//// ---------- common conversions for sRGB family ----------
////template <class T>
////inline T srgb_to_linear_scalar(T v) noexcept
////{
////  return (v <= T(0.04045)) ? (v / T(12.92)) : std::pow((v + T(0.055)) / T(1.055), T(2.4));
////}
////template <class T>
////inline T linear_to_srgb_scalar(T v) noexcept
////{
////  return (v <= T(0.0031308)) ? (v * T(12.92)) : (T(1.055) * std::pow(v, T(1.0 / 2.4)) - T(0.055));
////}
//
//// ---------- example spaces ----------
//struct LinearSRGB
//{
//  static constexpr int channels = 3;
//  template <class T>
//  using storage = std::array<T, channels>;
//  static constexpr bool is_linear = true;
//  static constexpr bool supports_hsv = true;
//  using white = D65;
//
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> to_xyz(storage<T> c) noexcept
//  {
//    return lin_srgb_to_xyz(c);
//  }
//  template <class T>
//  CL_NODISCARD static constexpr storage<T> from_xyz(xyz<T> X) noexcept
//  {
//    return xyz_to_lin_srgb(X);
//  }
//};
//
//struct SRGB
//{
//  static constexpr int channels = 3;
//  template <class T>
//  using storage = std::array<T, channels>;
//  static constexpr bool is_linear = false;
//  static constexpr bool supports_hsv = true;
//  using white = D65;
//
//  template <class T>
//  CL_NODISCARD static inline xyz<T> to_xyz(storage<T> c) noexcept
//  {
//    return lin_srgb_to_xyz<T>({srgb_to_linear_scalar(c[0]), srgb_to_linear_scalar(c[1]), srgb_to_linear_scalar(c[2])});
//  }
//  template <class T>
//  CL_NODISCARD static inline storage<T> from_xyz(xyz<T> X) noexcept
//  {
//    auto lin = xyz_to_lin_srgb<T>(X);
//    return {linear_to_srgb_scalar(lin[0]), linear_to_srgb_scalar(lin[1]), linear_to_srgb_scalar(lin[2])};
//  }
//};
//
//// Perceptual grayscale (Y channel of XYZ/D65). Storage is 1 channel.
//struct GrayY
//{
//  static constexpr int channels = 1;
//  template <class T>
//  using storage = std::array<T, channels>;
//  static constexpr bool is_linear = true;  // Y is linear light
//  static constexpr bool supports_hsv = false;
//  using white = D65;
//
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> to_xyz(storage<T> c) noexcept
//  {
//    const T Y = c[0];
//    return xyz<T>{Y, Y, Y};  // NOTE: this assumes equal-energy; fine for “gray from Y”
//  }
//  template <class T>
//  CL_NODISCARD static constexpr storage<T> from_xyz(xyz<T> X) noexcept
//  {
//    return storage<T>{X[1]};  // Y channel
//  }
//};
//
//// CMYK example (device dependent!): here’s a naive “working CMYK” just to
//// prove the variable-channel design. Real CMYK needs a profile & black gen.
//struct CMYK_Working
//{
//  static constexpr int channels = 4;
//  template <class T>
//  using storage = std::array<T, channels>;
//  static constexpr bool is_linear = false;  // don’t trust per-channel math
//  static constexpr bool supports_hsv = false;
//  using white = D65;
//
//  template <class T>
//  CL_NODISCARD static inline xyz<T> to_xyz(storage<T> c) noexcept
//  {
//    // crude conversion: convert to approximate RGB then to XYZ
//    const T C = c[0], M = c[1], Y = c[2], K = c[3];
//    const T r = (T(1) - C) * (T(1) - K);
//    const T g = (T(1) - M) * (T(1) - K);
//    const T b = (T(1) - Y) * (T(1) - K);
//    return lin_srgb_to_xyz<T>({r, g, b});
//  }
//  template <class T>
//  CL_NODISCARD static inline storage<T> from_xyz(xyz<T> X) noexcept
//  {
//    auto rgb = xyz_to_lin_srgb<T>(X);
//    const T K = T(1) - std::max({rgb[0], rgb[1], rgb[2]});
//    const T denom = T(1) - K + T(1e-8);
//    const T C = (T(1) - rgb[0] - K) / denom;
//    const T M = (T(1) - rgb[1] - K) / denom;
//    const T Y = (T(1) - rgb[2] - K) / denom;
//    return {clamp(C, T(0), T(1)), clamp(M, T(0), T(1)), clamp(Y, T(0), T(1)), clamp(K, T(0), T(1))};
//  }
//};
//
//// ---------- generic per-channel ops (N = Space::channels) ----------
//template <class Space, class T, class F>
//CL_NODISCARD constexpr typename Space::template storage<T> map_channels(typename Space::template storage<T> v,
//                                                                        F&& f) noexcept(noexcept(f(T{})))
//{
//  for (int i = 0; i < Space::channels; ++i)
//    v[i] = f(v[i]);
//  return v;
//}
//
//template <class Space, class T>
//CL_NODISCARD constexpr typename Space::template storage<T> add(typename Space::template storage<T> v, T d) noexcept
//{
//  for (int i = 0; i < Space::channels; ++i)
//    v[i] += d;
//  return v;
//}
//template <class Space, class T>
//CL_NODISCARD constexpr typename Space::template storage<T> mul(typename Space::template storage<T> v, T s) noexcept
//{
//  for (int i = 0; i < Space::channels; ++i)
//    v[i] *= s;
//  return v;
//}
//template <class Space, class T>
//CL_NODISCARD constexpr typename Space::template storage<T> clamp01(typename Space::template storage<T> v) noexcept
//{
//  for (int i = 0; i < Space::channels; ++i)
//    v[i] = clamp(v[i], T(0), T(1));
//  return v;
//}
//template <class Space, class T>
//CL_NODISCARD constexpr typename Space::template storage<T> invert01(typename Space::template storage<T> v) noexcept
//{
//  for (int i = 0; i < Space::channels; ++i)
//    v[i] = T(1) - v[i];
//  return v;
//}
//
//// ---------- cross-space convert ----------
//template <class ToSpace, class FromSpace, class T>
//CL_NODISCARD inline typename ToSpace::template storage<T> convert(typename FromSpace::template storage<T> v) noexcept
//{
//  const xyz<T> X = FromSpace::template to_xyz<T>(v);
//  return ToSpace::template from_xyz<T>(X);
//}
//
//
//
//// ---------------- traits scaffold -------------------------------------------
//
//template <class Space>
//struct space_traits;  // primary template, intentionally incomplete
//
//// Helper to detect features at compile-time (SFINAE-friendly in C++17)
//template <class Space, class = void>
//struct has_hsv_helpers : std::false_type
//{
//};
//template <class Space>
//struct has_hsv_helpers<Space, std::void_t<decltype(Space::supports_hsv)>>
//    : std::integral_constant<bool, Space::supports_hsv>
//{
//};
//
//template <class Space, class = void>
//struct is_linear_light : std::false_type
//{
//};
//template <class Space>
//struct is_linear_light<Space, std::void_t<decltype(Space::is_linear)>> : std::integral_constant<bool, Space::is_linear>
//{
//};
//
//// ---------------- concrete spaces -------------------------------------------
//
//// 1) Linear sRGB (scene- or display-referred linear; gamut sRGB, white D65)
//struct LinearSRGB
//{
//  static constexpr bool is_linear = true;
//  static constexpr bool supports_hsv = true;  // hue/sat/value math on linear is okay for utilities
//  using white = D65;
//
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> to_xyz(rgb<T> c) noexcept
//  {
//    return lin_srgb_to_xyz(c);
//  }
//  template <class T>
//  CL_NODISCARD static constexpr rgb<T> from_xyz(xyz<T> X) noexcept
//  {
//    return xyz_to_lin_srgb(X);
//  }
//};
//
//// 2) sRGB (non-linear display space, D65). Uses the same primaries, different transfer.
//struct SRGB
//{
//  static constexpr bool is_linear = false;
//  static constexpr bool supports_hsv = true;
//  using white = D65;
//
//  template <class T>
//  CL_NODISCARD static inline xyz<T> to_xyz(rgb<T> c) noexcept
//  {
//    return lin_srgb_to_xyz(srgb_to_linear(c));
//  }
//  template <class T>
//  CL_NODISCARD static inline rgb<T> from_xyz(xyz<T> X) noexcept
//  {
//    return linear_to_srgb(xyz_to_lin_srgb(X));
//  }
//};
//
//// 3) XYZ/D65 itself as a “space”
//struct XYZ_D65
//{
//  static constexpr bool is_linear = true;
//  static constexpr bool supports_hsv = false;  // HSV on XYZ makes no sense
//  using white = D65;
//
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> to_xyz(xyz<T> X) noexcept
//  {
//    return X;
//  }
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> from_xyz(xyz<T> X) noexcept
//  {
//    return X;
//  }
//};
//
//// ---------------- traits specializations ------------------------------------
//
//template <>
//struct space_traits<LinearSRGB>
//{
//  static constexpr int channels = 3;
//  static constexpr bool has_alpha = false;
//  static constexpr bool has_hsv = true;
//  using white = LinearSRGB::white;
//
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> to_xyz(rgb<T> c) noexcept
//  {
//    return LinearSRGB::to_xyz<T>(c);
//  }
//  template <class T>
//  CL_NODISCARD static constexpr rgb<T> from_xyz(xyz<T> X) noexcept
//  {
//    return LinearSRGB::from_xyz<T>(X);
//  }
//};
//
//template <>
//struct space_traits<SRGB>
//{
//  static constexpr int channels = 3;
//  static constexpr bool has_alpha = false;
//  static constexpr bool has_hsv = true;
//  using white = SRGB::white;
//
//  template <class T>
//  CL_NODISCARD static inline xyz<T> to_xyz(rgb<T> c) noexcept
//  {
//    return SRGB::to_xyz<T>(c);
//  }
//  template <class T>
//  CL_NODISCARD static inline rgb<T> from_xyz(xyz<T> X) noexcept
//  {
//    return SRGB::from_xyz<T>(X);
//  }
//};
//
//template <>
//struct space_traits<XYZ_D65>
//{
//  static constexpr int channels = 3;
//  static constexpr bool has_alpha = false;
//  static constexpr bool has_hsv = false;
//  using white = XYZ_D65::white;
//
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> to_xyz(xyz<T> X) noexcept
//  {
//    return X;
//  }
//  template <class T>
//  CL_NODISCARD static constexpr xyz<T> from_xyz(xyz<T> X) noexcept
//  {
//    return X;
//  }
//};
//
//// ---------------- convenience conversions -----------------------------------
//
//template <class ToSpace, class FromSpace, class T>
//CL_NODISCARD inline std::array<T, 3> convert3(std::array<T, 3> v) noexcept
//{
//  const xyz<T> X = space_traits<FromSpace>::template to_xyz<T>(v);
//  return space_traits<ToSpace>::template from_xyz<T>(X);
//}
//
//
//template <class Space, class T = float>
//struct Color
//{
//  using space = Space;
//  using value_t = T;
//  using storage = typename Space::template storage<T>;
//
//  storage v{};  // trivially copyable, POD-ish
//
//  // --- ctors ---
//  constexpr Color() = default;
//  constexpr explicit Color(const storage& vv)
//      : v(vv)
//  {
//  }
//
//  // helpers to build with N args (1..4) without overloading explosion
//  template <class... Args, std::enable_if_t<(sizeof...(Args) == Space::channels), int> = 0>
//  constexpr explicit Color(Args... args)
//      : v{static_cast<T>(args)...}
//  {
//  }
//
//  // --- size/info ---
//  CL_NODISCARD static constexpr int Channels() noexcept
//  {
//    return Space::channels;
//  }
//  CL_NODISCARD static constexpr bool IsLinear() noexcept
//  {
//    return Space::is_linear;
//  }
//
//  // --- generic channel access ---
//  CL_NODISCARD constexpr T Channel(int i) const noexcept
//  {
//    return v[static_cast<std::size_t>(i)];
//  }
//  CL_NODISCARD constexpr Color WithChannel(int i, T val) const noexcept
//  {
//    storage out = v;
//    out[static_cast<std::size_t>(i)] = val;
//    return Color(out);
//  }
//
//  // --- scalar transforms (fluent, immutable) ---
//  CL_NODISCARD constexpr Color Scaled(T s) const noexcept
//  {
//    return Color(mul<Space, T>(v, s));
//  }
//  CL_NODISCARD constexpr Color Offset(T d) const noexcept
//  {
//    return Color(add<Space, T>(v, d));
//  }
//  CL_NODISCARD constexpr Color Clamped01() const noexcept
//  {
//    return Color(clamp01<Space, T>(v));
//  }
//  CL_NODISCARD constexpr Color Inverted() const noexcept
//  {
//    return Color(invert01<Space, T>(v));
//  }
//
//  // --- tonal adjustments (require linear [0,1]) ---
//  template <bool OK = Space::is_linear, std::enable_if_t<OK, int> = 0>
//  CL_NODISCARD constexpr Color Lightened(T f) const noexcept
//  {
//    return Color(map_channels<Space, T>(v,
//                                        [f](T c)
//                                        {
//                                          return lerp(c, T(1), f);
//                                        }));
//  }
//  template <bool OK = Space::is_linear, std::enable_if_t<OK, int> = 0>
//  CL_NODISCARD constexpr Color Darkened(T f) const noexcept
//  {
//    return Color(map_channels<Space, T>(v,
//                                        [f](T c)
//                                        {
//                                          return lerp(c, T(0), f);
//                                        }));
//  }
//  // Simple gamma on each channel; caller responsible for using linear values.
//  CL_NODISCARD inline Color GammaCorrect(T g) const noexcept
//  {
//    storage out = v;
//    for (int i = 0; i < Space::channels; ++i)
//      out[i] = std::pow(out[i], g);
//    return Color(out);
//  }
//
//  // --- mix ---
//  template <class OtherSpace>
//  CL_NODISCARD inline Color MixedWith(Color<OtherSpace, T> other, T t) const noexcept
//  {
//    // Convert both to a common space (XYZ → current Space) to mix in *this* space.
//    // For many workflows you may prefer mixing in a specific linear space.
//    auto a_this = v;
//    auto b_this = convert<Space, OtherSpace, T>(other.v);
//    storage out = a_this;
//    for (int i = 0; i < Space::channels; ++i)
//      out[i] = lerp(a_this[i], b_this[i], t);
//    return Color(out);
//  }
//  template <class OtherSpace>
//  CL_NODISCARD inline Color Lerp(Color<OtherSpace, T> other, T t) const noexcept
//  {
//    return MixedWith(other, t);
//  }
//
//  // --- analysis ---
//  // Luminance defined only when space is linear and 3ch RGB-like (LinearSRGB).
//  template <class S = Space, std::enable_if_t<std::is_same<S, LinearSRGB>::value, int> = 0>
//  CL_NODISCARD constexpr T Luminance() const noexcept
//  {
//    // Rec.709 coefficients
//    return v[0] * T(0.2126) + v[1] * T(0.7152) + v[2] * T(0.0722);
//  }
//
//  // --- conversions ---
//  template <class ToSpace>
//  CL_NODISCARD inline Color<ToSpace, T> To() const noexcept
//  {
//    return Color<ToSpace, T>(convert<ToSpace, Space, T>(v));
//  }
//};


struct ColorF
{
  float r;
  float g;
  float b;
};

// effectively Q8 format. todo: used fixed<>
struct ColorByte
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  // luminance
  // hsl -> vec3 / etc.
  // rgb -> vec3 / etc.

  // hsla -> vec4 / etc.

  // WithR() / WithAlpha()
  // WithSaturation()

  // etc...
};

}  // namespace clarinoid
