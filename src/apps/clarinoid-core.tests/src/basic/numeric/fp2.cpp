#include <windows.h>

#include <gtest/gtest.h>

#include <clarinoid/core/basic/basic.hpp>
#include <clarinoid/core/basic/numeric/fp2.hpp>
// #include <clarinoid/core/basic/numeric/operator_fx.hpp>

using namespace clarinoid;
// using namespace clarinoid::fxl::literals;

namespace FixedPointTests
{

// Test fixture for common format constants
struct FixedPointTest : public ::testing::Test
{
  static constexpr double EPSILON = 1e-9;  // For floating point comparisons
};

// ============================================================================
// Format and Layout Tests
// ============================================================================


TEST_F(FixedPointTest, PickUsableTypeTests)
{
  {
    using T = clarinoid::PickUsableType_t<true /*signed*/, 1, int8_t, uint8_t>;
    T a = 0;
    static_assert(std::is_same_v<T, int8_t>);
  }
  {
    using T = clarinoid::PickUsableType_t<false /*signed*/, 1, int8_t, uint8_t>;
    T a = 0;
    static_assert(std::is_same_v<T, uint8_t>);
  }
  {
    using T = clarinoid::PickUsableType_t<true /*signed*/, 1, int8_t>;
    T a = 0;
    static_assert(std::is_same_v<T, int8_t>);
  }
  // pick PREFERRED, not widest / narrowest.
  {
    using T = clarinoid::PickUsableType_t<false /*signed*/, 1, int8_t, uint16_t>;
    T a = 0;
    static_assert(std::is_same_v<T, uint8_t>);
  }
  {
    using T = clarinoid::PickUsableType_t<false /*signed*/, 1, int16_t, uint8_t>;
    T a = 0;
    static_assert(std::is_same_v<T, uint16_t>);
  }
}

TEST_F(FixedPointTest, WidestTypeTests)
{
  {
    using T = clarinoid::WidestType_t<int8_t, uint8_t>;
    T a = 0;
    static_assert(std::is_same_v<T, uint8_t>);
  }
  {
    using T = clarinoid::WidestType_t<int8_t, uint16_t>;
    T a = 0;
    static_assert(std::is_same_v<T, uint16_t>);
  }
  {
    using T = clarinoid::WidestType_t<int16_t, uint8_t>;
    T a = 0;
    static_assert(std::is_same_v<T, int16_t>);
  }
}

TEST_F(FixedPointTest, NarrowedFormatTests) {}

TEST_F(FixedPointTest, SqueezedFormatTests) {}

TEST_F(FixedPointTest, LayoutConstants)
{
  // Test FxLayout constants
  using Layout15_16 = FxLayout<15, 16>;
  EXPECT_EQ(15, Layout15_16::MagBits);
  EXPECT_EQ(16, Layout15_16::FracBits);
  EXPECT_EQ(31, Layout15_16::ValueBits);

  using Layout1_31 = FxLayout<1, 31>;
  EXPECT_EQ(1, Layout1_31::MagBits);
  EXPECT_EQ(31, Layout1_31::FracBits);
  EXPECT_EQ(32, Layout1_31::ValueBits);
}

TEST_F(FixedPointTest, StorageTraitsConstants)
{
  // Test FxStorageTraits for int32_t
  using Storage32 = FxStorageTraits<int32_t>;
  EXPECT_TRUE(Storage32::IsSigned);
  EXPECT_EQ(32, Storage32::StorageWidthBits);
  EXPECT_EQ(31, Storage32::StorageValueBits);  // 31 for signed int32_t

  // Test FxStorageTraits for uint16_t
  using Storage16u = FxStorageTraits<uint16_t>;
  EXPECT_FALSE(Storage16u::IsSigned);
  EXPECT_EQ(16, Storage16u::StorageWidthBits);
  EXPECT_EQ(16, Storage16u::StorageValueBits);  // 16 for unsigned uint16_t
}

TEST_F(FixedPointTest, FormatConstants)
{
  // Test Q15.16 format
  using Q15_16_Format = FxFormat<FxLayout<15, 16>, FxStorageTraits<int32_t>>;
  EXPECT_EQ(15, Q15_16_Format::MagBits);
  EXPECT_EQ(16, Q15_16_Format::FracBits);
  EXPECT_EQ(31, Q15_16_Format::ValueBits);
  EXPECT_TRUE(Q15_16_Format::IsSigned);
  EXPECT_EQ(0, Q15_16_Format::HeadroomBits);  // 31 value bits in 31-bit storage

  // Test Q0.31 format (valid - 31 value bits in 31-bit signed storage)
  using Q0_31_Format = FxFormat<FxLayout<0, 31>, FxStorageTraits<int32_t>>;
  EXPECT_EQ(0, Q0_31_Format::MagBits);
  EXPECT_EQ(31, Q0_31_Format::FracBits);
  EXPECT_EQ(31, Q0_31_Format::ValueBits);
  EXPECT_TRUE(Q0_31_Format::IsSigned);
  EXPECT_EQ(0, Q0_31_Format::HeadroomBits);  // 31 value bits in 31-bit storage
}

TEST_F(FixedPointTest, RawMinMaxConstants)
{
  // Test Q15.16 format (signed 32-bit storage)
  using Q15_16_Format = FxFormat<FxLayout<15, 16>, FxStorageTraits<int32_t>>;

  // For Q15.16: 15 int bits + 16 frac bits = 31 value bits in signed int32_t
  // RawMax should be (2^30 - 1) for signed 31-bit value range
  // RawMin should be -RawMax - 1 = -2^30
  int32_t expected_max = (1 << 30) - 1;      // 2^30 - 1
  int32_t expected_min = -expected_max - 1;  // -2^30

  EXPECT_EQ(expected_max, Q15_16_Format::RawMax);
  EXPECT_EQ(expected_min, Q15_16_Format::RawMin);

  // Test Q0.31 format (signed 32-bit storage, all fractional bits)
  using Q0_31_Format = FxFormat<FxLayout<0, 31>, FxStorageTraits<int32_t>>;

  // Same as Q15.16 since both use 31 value bits in signed int32_t
  EXPECT_EQ(expected_max, Q0_31_Format::RawMax);
  EXPECT_EQ(expected_min, Q0_31_Format::RawMin);

  // Test Q7.8 format (signed 16-bit storage)
  using Q7_8_Format = FxFormat<FxLayout<7, 8>, FxStorageTraits<int16_t>>;

  // For Q7.8: 7 int bits + 8 frac bits = 15 value bits in signed int16_t
  // RawMax should be (2^14 - 1) for signed 15-bit value range
  // RawMin should be -RawMax - 1 = -2^14
  int32_t expected_max_16 = 0x7fff;   // 2^14 - 1 = 16383
  int32_t expected_min_16 = -0x8000;  // -2^14 = -16384

  EXPECT_EQ(expected_max_16, Q7_8_Format::RawMax);
  EXPECT_EQ(expected_min_16, Q7_8_Format::RawMin);

  // Test unsigned format
  using Q8_8_Unsigned_Format = FxFormat<FxLayout<8, 8>, FxStorageTraits<uint16_t>>;

  // For unsigned Q8.8: 8 int bits + 8 frac bits = 16 value bits in uint16_t
  // RawMax should be (2^16 - 1) = 65535
  // RawMin should be 0
  int32_t expected_max_u16 = 0xffff;  // 2^16 - 1 = 65535
  int32_t expected_min_u16 = 0;

  EXPECT_EQ(expected_max_u16, Q8_8_Unsigned_Format::RawMax);
  EXPECT_EQ(expected_min_u16, Q8_8_Unsigned_Format::RawMin);

  // Test format with headroom
  using Q3_4_Format = FxFormat<FxLayout<3, 4>, FxStorageTraits<int16_t>>;

  // For Q3.4: 3 int bits + 4 frac bits = 7 value bits in signed int16_t (15 available)
  // So we have 8 bits of headroom
  // RawMax should be (2^6 - 1) for signed 7-bit value range = 63
  // RawMin should be -RawMax - 1 = -64
  int16_t expected_max_small = (1 << 6) - 1;             // 2^6 - 1 = 63
  int16_t expected_min_small = -expected_max_small - 1;  // -2^6 = -64

  EXPECT_EQ(expected_max_small, Q3_4_Format::RawMax);
  EXPECT_EQ(expected_min_small, Q3_4_Format::RawMin);
  EXPECT_EQ(8, Q3_4_Format::HeadroomBits);  // Verify we have headroom
}

TEST_F(FixedPointTest, RawMinMaxEdgeCases)
{
  // Test 8-bit formats
  using Q3_4_8bit = FxFormat<FxLayout<3, 4>, FxStorageTraits<int8_t>>;

  // 3 + 4 = 7 value bits in 7-bit signed storage (perfect fit)
  int8_t expected_max_8 = (1 << 6) - 1;         // 2^6 - 1 = 63
  int8_t expected_min_8 = -expected_max_8 - 1;  // -64

  EXPECT_EQ(expected_max_8, Q3_4_8bit::RawMax);
  EXPECT_EQ(expected_min_8, Q3_4_8bit::RawMin);
  EXPECT_EQ(0, Q3_4_8bit::HeadroomBits);  // No headroom

  // Test unsigned 8-bit format
  using Q4_4_Unsigned_8bit = FxFormat<FxLayout<4, 4>, FxStorageTraits<uint8_t>>;

  // 4 + 4 = 8 value bits in 8-bit unsigned storage (perfect fit)
  uint8_t expected_max_u8 = (1 << 8) - 1;  // 2^8 - 1 = 255
  uint8_t expected_min_u8 = 0;

  EXPECT_EQ(expected_max_u8, Q4_4_Unsigned_8bit::RawMax);
  EXPECT_EQ(expected_min_u8, Q4_4_Unsigned_8bit::RawMin);
  EXPECT_EQ(0, Q4_4_Unsigned_8bit::HeadroomBits);  // No headroom

  // Test single bit formats
  using Q0_1_Format = FxFormat<FxLayout<0, 1>, FxStorageTraits<int8_t>>;

  // 0 + 1 = 1 value bit in 7-bit signed storage
  int8_t expected_max_1bit = (1 << 0) - 1;            // 2^0 - 1 = 0
  int8_t expected_min_1bit = -expected_max_1bit - 1;  // -1

  EXPECT_EQ(expected_max_1bit, Q0_1_Format::RawMax);
  EXPECT_EQ(expected_min_1bit, Q0_1_Format::RawMin);
  EXPECT_EQ(6, Q0_1_Format::HeadroomBits);  // Lots of headroom
}

// ============================================================================
// Construction and Conversion Tests
// ============================================================================
//
//TEST_F(FixedPointTest, ConstructionFromFloat)
//{
//  // Test Q15.16 construction
//  Q15_16 a(1.5);
//  EXPECT_NEAR(1.5, a.ToFloat(), EPSILON);
//
//  Q15_16 b(2.25);
//  EXPECT_NEAR(2.25, b.ToFloat(), EPSILON);
//
//  Q15_16 c(-3.75);
//  EXPECT_NEAR(-3.75, c.ToFloat(), EPSILON);
//
//  // Test Q0.31 construction
//  Q0_31_Smart d(0.5);
//  EXPECT_NEAR(0.5, d.ToFloat(), EPSILON);
//
//  Q0_31_Smart e(0.25);
//  EXPECT_NEAR(0.25, e.ToFloat(), EPSILON);
//
//  auto x1 = FixedAuto<>();
//}
//
//TEST_F(FixedPointTest, ConstructionFromRaw)
//{
//  // Test raw construction for Q15.16
//  auto raw_value = 0x18000;  // 1.5 in Q15.16 format
//  auto a = Q15_16::FromRaw(raw_value);
//  EXPECT_EQ(raw_value, a.RawValue());
//  EXPECT_NEAR(1.5, a.ToFloat(), EPSILON);
//
//  // Test raw construction for Q0.31
//  auto raw_value_31 = 0x40000000;  // 0.5 in Q0.31 format
//  auto b = Q0_31_Smart::FromRaw(raw_value_31);
//  EXPECT_EQ(raw_value_31, b.RawValue());
//  EXPECT_NEAR(0.5, b.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, DefaultConstruction)
//{
//  Q15_16 a;
//  EXPECT_EQ(0, a.RawValue());
//  EXPECT_NEAR(0.0, a.ToFloat(), EPSILON);
//
//  Q0_31_Smart b;
//  EXPECT_EQ(0, b.RawValue());
//  EXPECT_NEAR(0.0, b.ToFloat(), EPSILON);
//}
//
//// ============================================================================
//// Arithmetic Tests - Naive Kernel
//// ============================================================================
//
//TEST_F(FixedPointTest, NaiveKernelAddition)
//{
//  Q15_16 a(1.5);
//  Q15_16 b(2.25);
//
//  auto result = a + b;
//  EXPECT_NEAR(3.75, result.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, NaiveKernelMultiplication)
//{
//  Q15_16 a(1.5);
//  Q15_16 b(2.0);
//
//  auto result = a * b;
//  EXPECT_NEAR(3.0, result.ToFloat(), EPSILON);
//
//  // Test with fractional results
//  Q15_16 c(1.5);
//  Q15_16 d(2.25);
//  auto result2 = c * d;
//  EXPECT_NEAR(3.375, result2.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, NaiveKernelNegation)
//{
//  Q15_16 a(1.5);
//  auto result = -a;
//  EXPECT_NEAR(-1.5, result.ToFloat(), EPSILON);
//
//  Q15_16 b(-2.25);
//  auto result2 = -b;
//  EXPECT_NEAR(2.25, result2.ToFloat(), EPSILON);
//}
//
//// ============================================================================
//// Arithmetic Tests - Smart Kernel
//// ============================================================================
//
//TEST_F(FixedPointTest, SmartKernelAddition)
//{
//  Q15_16_Smart a(1.5);
//  Q15_16_Smart b(2.25);
//
//  auto result = a + b;
//  EXPECT_NEAR(3.75, result.ToFloat(), EPSILON);
//
//  // Test different fractional bit alignment (when we add this feature)
//  Q0_31_Smart c(0.5);
//  Q0_31_Smart d(0.25);
//  auto result2 = c + d;
//  EXPECT_NEAR(0.75, result2.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, SmartKernelMultiplication)
//{
//  Q15_16_Smart a(1.5);
//  Q15_16_Smart b(2.0);
//
//  auto result = a * b;
//  EXPECT_NEAR(3.0, result.ToFloat(), EPSILON);
//
//  // Test case that should trigger shift elision optimization
//  Q0_31_Smart c(0.5);
//  Q0_31_Smart d(0.25);
//  auto result2 = c * d;
//  EXPECT_NEAR(0.125, result2.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, SmartKernelNegation)
//{
//  Q15_16_Smart a(1.5);
//  auto result = -a;
//  EXPECT_NEAR(-1.5, result.ToFloat(), EPSILON);
//
//  Q0_31_Smart b(0.75);
//  auto result2 = -b;
//  EXPECT_NEAR(-0.75, result2.ToFloat(), EPSILON);
//}
//
//// ============================================================================
//// Comparison Tests
//// ============================================================================
//
//TEST_F(FixedPointTest, EqualityComparison)
//{
//  Q15_16 a(1.5);
//  Q15_16 b(1.5);
//  Q15_16 c(2.5);
//
//  EXPECT_TRUE(a == b);
//  EXPECT_FALSE(a == c);
//
//  Q15_16_Smart d(1.5);
//  Q15_16_Smart e(1.5);
//  Q15_16_Smart f(2.5);
//
//  EXPECT_TRUE(d == e);
//  EXPECT_FALSE(d == f);
//}
//
//TEST_F(FixedPointTest, LessThanComparison)
//{
//  Q15_16 a(1.5);
//  Q15_16 b(2.5);
//
//  EXPECT_TRUE(a < b);
//  EXPECT_FALSE(b < a);
//  EXPECT_FALSE(a < a);
//
//  // Test with negative values
//  Q15_16 c(-1.5);
//  Q15_16 d(1.5);
//
//  EXPECT_TRUE(c < d);
//  EXPECT_FALSE(d < c);
//}
//
//// ============================================================================
//// Edge Cases and Boundary Tests
//// ============================================================================
//
//TEST_F(FixedPointTest, ZeroValues)
//{
//  Q15_16 zero;
//  Q15_16 a(1.5);
//
//  auto sum = zero + a;
//  EXPECT_NEAR(1.5, sum.ToFloat(), EPSILON);
//
//  auto product = zero * a;
//  EXPECT_NEAR(0.0, product.ToFloat(), EPSILON);
//
//  auto negated = -zero;
//  EXPECT_NEAR(0.0, negated.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, SmallValues)
//{
//  // Test very small values that are still representable
//  Q0_31_Smart small(1.0 / (1 << 30));  // Very small but representable in Q0.31
//  EXPECT_GT(small.ToFloat(), 0.0);
//
//  auto doubled = small + small;
//  EXPECT_NEAR(2.0 / (1 << 30), doubled.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, NearMaxValues)
//{
//  // Test values near the maximum representable value
//  // For Q15.16, max should be around 32767.99998...
//  Q15_16 near_max(32767.0);
//  EXPECT_NEAR(32767.0, near_max.ToFloat(), EPSILON);
//
//  // For Q0.31, max should be around 0.99999...
//  Q0_31_Smart near_max_q31(0.99999);
//  EXPECT_NEAR(0.99999, near_max_q31.ToFloat(), 1e-5);
//}
//
//// ============================================================================
//// Precision and Accuracy Tests
//// ============================================================================
//
//TEST_F(FixedPointTest, PrecisionLimits)
//{
//  // Test that we can represent values accurately within the format's precision
//
//  // Q15.16 has 16 fractional bits, so smallest increment is 1/65536
//  Q15_16 one(1.0);
//  Q15_16 one_plus_epsilon = Q15_16::FromRaw(one.RawValue() + 1);
//
//  double expected_diff = 1.0 / 65536.0;
//  double actual_diff = one_plus_epsilon.ToFloat() - one.ToFloat();
//  EXPECT_NEAR(expected_diff, actual_diff, EPSILON);
//
//  // Q0.31 has 31 fractional bits
//  Q0_31_Smart half(0.5);
//  Q0_31_Smart half_plus_epsilon = Q0_31_Smart::FromRaw(half.RawValue() + 1);
//
//  double expected_diff_q31 = 1.0 / (1LL << 31);
//  double actual_diff_q31 = half_plus_epsilon.ToFloat() - half.ToFloat();
//  EXPECT_NEAR(expected_diff_q31, actual_diff_q31, 1e-10);
//}
//
//// ============================================================================
//// Kernel Comparison Tests
//// ============================================================================
//
//TEST_F(FixedPointTest, NaiveVsSmartKernelConsistency)
//{
//  // Verify that naive and smart kernels produce identical results for basic operations
//
//  Q15_16 naive_a(1.5);
//  Q15_16 naive_b(2.25);
//  Q15_16_Smart smart_a(1.5);
//  Q15_16_Smart smart_b(2.25);
//
//  // Addition consistency
//  auto naive_sum = naive_a + naive_b;
//  auto smart_sum = smart_a + smart_b;
//  EXPECT_NEAR(naive_sum.ToFloat(), smart_sum.ToFloat(), EPSILON);
//
//  // Multiplication consistency
//  auto naive_product = naive_a * naive_b;
//  auto smart_product = smart_a * smart_b;
//  EXPECT_NEAR(naive_product.ToFloat(), smart_product.ToFloat(), EPSILON);
//
//  // Negation consistency
//  auto naive_neg = -naive_a;
//  auto smart_neg = -smart_a;
//  EXPECT_NEAR(naive_neg.ToFloat(), smart_neg.ToFloat(), EPSILON);
//}
//
//// ============================================================================
//// Type Safety Tests
//// ============================================================================
//
//TEST_F(FixedPointTest, TypeSafety)
//{
//  // Verify that different formats don't accidentally mix without explicit conversion
//  Q15_16 a(1.5);
//  Q0_31_Smart b(0.5);
//
//  // These should have different types and can't be directly compared
//  // (This is more of a compile-time test, but we can verify result types)
//
//  static_assert(!std::is_same_v<decltype(a), decltype(b)>, "Different fixed-point formats should have different types");
//}
//
//// ============================================================================
//// Convenient Template Alias Tests
//// ============================================================================
//
//TEST_F(FixedPointTest, FixedAutoSyntax)
//{
//  // Test the convenient template alias with automatic raw type selection
//
//  // FixedAuto<1, 15> should automatically select int16_t (16 bits total)
//  FixedAuto<1, 15> small_format(1.5);
//  EXPECT_NEAR(1.5, small_format.ToFloat(), EPSILON);
//
//  // FixedAuto<7, 8> should automatically select int16_t (15 bits total)
//  FixedAuto<7, 8> medium_format(123.25);
//  EXPECT_NEAR(123.25, medium_format.ToFloat(), EPSILON);
//
//  // FixedAuto<15, 16> should automatically select int32_t (31 bits total)
//  FixedAuto<15, 16> large_format(12345.125);
//  EXPECT_NEAR(12345.125, large_format.ToFloat(), EPSILON);
//
//  // Test unsigned variant
//  FixedAuto<8, 8, false> unsigned_format(123.25);
//  EXPECT_NEAR(123.25, unsigned_format.ToFloat(), EPSILON);
//
//  // Test explicit raw type override
//  FixedAuto<1, 15, true, int32_t> explicit_raw(1.5);
//  EXPECT_NEAR(1.5, explicit_raw.ToFloat(), EPSILON);
//
//  // Verify that different combinations have different types (compile-time check)
//  static_assert(!std::is_same_v<decltype(small_format), decltype(medium_format)>,
//                "Different bit layouts should have different types");
//  static_assert(!std::is_same_v<decltype(medium_format), decltype(unsigned_format)>,
//                "Signed vs unsigned should have different types");
//}
//
//TEST_F(FixedPointTest, OptimalRawTypeSelection)
//{
//  // Verify that the optimal raw type selection works correctly
//
//  auto x = clarinoid::SelectOptimalRawType<true, 1, 1>();
//
//  // Small formats should use smaller types
//  static_assert(std::is_same_v<typename FxSmartKernel::OptimalRawType<true, 3, 4>, int8_t>,
//                "7-bit format should use int8_t");
//  static_assert(std::is_same_v<typename FxSmartKernel::OptimalRawType<false, 4, 4>, uint8_t>,
//                "8-bit unsigned format should use uint8_t");
//
//  // Medium formats should use 16-bit types
//  static_assert(std::is_same_v<typename FxSmartKernel::OptimalRawType<true, 7, 8>, int16_t>,
//                "15-bit format should use int16_t");
//  static_assert(std::is_same_v<typename FxSmartKernel::OptimalRawType<false, 8, 8>, uint16_t>,
//                "16-bit unsigned format should use uint16_t");
//
//  // Large formats should use 32-bit types
//  static_assert(std::is_same_v<typename FxSmartKernel::OptimalRawType<true, 15, 16>, int32_t>,
//                "31-bit format should use int32_t");
//  static_assert(std::is_same_v<typename FxSmartKernel::OptimalRawType<false, 16, 16>, uint32_t>,
//                "32-bit unsigned format should use uint32_t");
//
//  // Very large formats should use 64-bit types
//  static_assert(std::is_same_v<typename FxSmartKernel::OptimalRawType<true, 31, 32>, int64_t>,
//                "63-bit format should use int64_t");
//}

// ============================================================================
// Performance Insight Tests
// ============================================================================
//
//TEST_F(FixedPointTest, HeadroomUtilization)
//{
//  // Test cases that demonstrate smart kernel's headroom utilization
//
//  // Q0.31 * Q0.31 should fit in 32-bit storage (31 value bits each, 62 total vs 31 available)
//  // This will still require 64-bit intermediate, but demonstrates the concept
//  Q0_31_Smart a(0.5);
//  Q0_31_Smart b(0.75);
//
//  auto result = a * b;
//  EXPECT_NEAR(0.375, result.ToFloat(), EPSILON);
//
//  // Better example: Q7.8 * Q7.8 = 16 value bits total, fits in 16-bit storage
//  Q7_8_Smart c(1.5);
//  Q7_8_Smart d(2.0);
//  auto result2 = c * d;
//  EXPECT_NEAR(3.0, result2.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, ChainedOperations)
//{
//  // Test chained operations to verify type consistency
//  Q15_16_Smart a(1.5);
//  Q15_16_Smart b(2.0);
//  Q15_16_Smart c(0.5);
//
//  auto result = (a + b) * c;
//  EXPECT_NEAR(1.75, result.ToFloat(), EPSILON);  // (1.5 + 2.0) * 0.5 = 1.75
//
//  auto result2 = a * b + c;
//  EXPECT_NEAR(3.5, result2.ToFloat(), EPSILON);  // 1.5 * 2.0 + 0.5 = 3.5
//
//  // Test with FixedAuto for cleaner syntax
//  FixedAuto<7, 8> x(1.5);
//  FixedAuto<7, 8> y(2.0);
//  FixedAuto<7, 8> z(0.5);
//
//  auto clean_result = (x + y) * z;
//  EXPECT_NEAR(1.75, clean_result.ToFloat(), EPSILON);
//
//  // Show that this compiles to efficient code - no explicit type management needed
//  auto very_clean = FixedAuto<15, 16>(3.5) * FixedAuto<15, 16>(2.0) + FixedAuto<15, 16>(1.0);
//  EXPECT_NEAR(8.0, very_clean.ToFloat(), EPSILON);  // 3.5 * 2.0 + 1.0 = 8.0
//}

// ============================================================================
// Literal Operator Tests
// ============================================================================

// NOTE: These tests are commented out until operator_fx.hpp compilation issues are resolved

using namespace clarinoid::fxl::literals;

//TEST_F(FixedPointTest, BasicLiteralOperator)
//{
//  // Test basic integer literals
//  auto a = 42_fx;
//  EXPECT_NEAR(42.0, a.ToFloat(), EPSILON);
//
//  auto b = 0_fx;
//  EXPECT_NEAR(0.0, b.ToFloat(), EPSILON);
//
//  auto c = 1_fx;
//  EXPECT_NEAR(1.0, c.ToFloat(), EPSILON);
//
//  // Test negative literals
//  auto d = -42_fx;
//  EXPECT_NEAR(-42.0, d.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, FractionalLiteralOperator)
//{
//  // Test basic fractional literals
//  auto a = 3.14_fx;
//  EXPECT_NEAR(3.14, a.ToFloat(), EPSILON);
//
//  auto b = 0.5_fx;
//  EXPECT_NEAR(0.5, b.ToFloat(), EPSILON);
//
//  auto c = -2.71828_fx;
//  EXPECT_NEAR(-2.71828, c.ToFloat(), EPSILON);
//
//  // Test very small fractional values
//  auto d = 0.001_fx;
//  EXPECT_NEAR(0.001, d.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, ScientificNotationLiterals)
//{
//  // Test scientific notation
//  auto a = 1e3_fx;
//  EXPECT_NEAR(1000.0, a.ToFloat(), EPSILON);
//
//  auto b = 1.5e2_fx;
//  EXPECT_NEAR(150.0, b.ToFloat(), EPSILON);
//
//  auto c = 2.5e-3_fx;
//  EXPECT_NEAR(0.0025, c.ToFloat(), EPSILON);
//
//  auto d = -1.23e-2_fx;
//  EXPECT_NEAR(-0.0123, d.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, LiteralOperatorTypeSelection)
//{
//  // Test that literals automatically select appropriate storage types
//
//  // Small values should use smaller storage
//  auto small = 3.14_fx;
//  static_assert(sizeof(decltype(small)::RawType) <= 4, "Small literals should use 32-bit or smaller storage");
//
//  // Large values should promote to larger storage if needed
//  auto large = 1000000.0_fx;
//  // This should still fit in 32-bit but demonstrates the concept
//  EXPECT_NEAR(1000000.0, large.ToFloat(), 1e-6);  // Some precision loss expected
//
//  // Very precise fractional values should maximize fractional bits
//  auto precise = 0.123456789_fx;
//  EXPECT_NEAR(0.123456789, precise.ToFloat(), 1e-6);  // Some precision loss expected
//}
//
//TEST_F(FixedPointTest, LiteralOperatorFormatOptimization)
//{
//  // Test that the literal operator chooses formats optimally
//
//  // Pure integer should have zero fractional bits
//  auto integer = 42_fx;
//  EXPECT_EQ(0, decltype(integer)::FormatType::FracBits);
//  EXPECT_EQ(6, decltype(integer)::FormatType::MagBits);
//  using Txx = decltype(integer)::FormatType::RawType;
//  static_assert(std::is_same_v<uint8_t, decltype(integer)::RawType>);
//
//  // Fractional values should allocate fractional bits
//  auto fractional = 3.14_fx;
//  EXPECT_GT(decltype(fractional)::FormatType::FracBits, 0);
//
//  // Test magnitude bit allocation
//  auto small_mag = 7_fx;    // needs 3 magnitude bits
//  auto large_mag = 255_fx;  // needs 8 magnitude bits
//
//  // Verify that different magnitudes get different bit allocations
//  static_assert(decltype(small_mag)::FormatType::MagBits <= decltype(large_mag)::FormatType::MagBits,
//                "Larger values should get more integer bits");
//}
//
//TEST_F(FixedPointTest, LiteralOperatorArithmetic)
//{
//  // Test arithmetic operations with literal-created values
//  auto a = 1.5_fx;
//  auto b = 2.5_fx;
//
//  auto sum = a + b;
//  EXPECT_NEAR(4.0, sum.ToFloat(), EPSILON);
//
//  auto product = a * b;
//  EXPECT_NEAR(3.75, product.ToFloat(), EPSILON);
//
//  auto negated = -a;
//  EXPECT_NEAR(-1.5, negated.ToFloat(), EPSILON);
//
//  // Test mixed operations with regular Fixed types
//  Q15_16 regular(1.0);
//  // Note: This might require explicit conversion in real usage
//  // auto mixed = a + regular;  // May not compile without conversion
//}
//
//TEST_F(FixedPointTest, LiteralOperatorPrecisionLimits)
//{
//  // Test precision limits of literal-generated types
//
//  // Very small differences should be representable
//  auto a = 1.0_fx;
//  auto b = 1.0001_fx;
//
//  EXPECT_NE(a.RawValue(), b.RawValue());
//  EXPECT_GT(b.ToFloat(), a.ToFloat());
//
//  // Test that precision is maximized for the chosen storage
//  auto precise = 0.999999_fx;
//  EXPECT_NEAR(0.999999, precise.ToFloat(), 1e-5);
//}
//
//TEST_F(FixedPointTest, LiteralOperatorBoundaryValues)
//{
//  // Test boundary cases
//
//  // Very small positive value
//  auto tiny = 0.0001_fx;
//  EXPECT_GT(tiny.ToFloat(), 0.0);
//
//  // Values near powers of 2
//  auto pow2_minus = 1023.99_fx;  // Just under 2^10
//  auto pow2_plus = 1024.01_fx;   // Just over 2^10
//
//  EXPECT_NEAR(1023.99, pow2_minus.ToFloat(), EPSILON);
//  EXPECT_NEAR(1024.01, pow2_plus.ToFloat(), EPSILON);
//
//  // Test that the format can represent these accurately
//  EXPECT_LT(std::abs(pow2_minus.ToFloat() - 1023.99), 0.01);
//  EXPECT_LT(std::abs(pow2_plus.ToFloat() - 1024.01), 0.01);
//}
//
//TEST_F(FixedPointTest, LiteralOperatorTypeUniqueness)
//{
//  // Test that different literals create different types when appropriate
//
//  auto a = 1.5_fx;
//  auto b = 2.5_fx;
//  auto c = 100_fx;
//
//  // Same format should have same type
//  static_assert(std::is_same_v<decltype(a), decltype(b)> || !std::is_same_v<decltype(a), decltype(b)>,
//                "Types may or may not be same depending on optimization");
//
//  // Different magnitude requirements should potentially have different types
//  // This is implementation-dependent based on our optimization strategy
//}

// utility function to test a Fixed<...> for its double representation, raw type, signedness, and bit layout.
template <typename TExpectedRawType, typename FixedType>
void TestFx(const FixedType& fixed, double expected_value, bool is_signed, int int_bits, int frac_bits)
{
  static_assert(std::is_same_v<typename FixedType::RawType, TExpectedRawType>,
                "Fixed type should have expected raw type");
  using FormatType = typename FixedType::FormatType;
  //EXPECT_EQ(expected_raw_value, fixed.RawValue());
  EXPECT_EQ(is_signed, FormatType::IsSigned);
  EXPECT_EQ(int_bits, FormatType::MagBits);
  EXPECT_EQ(frac_bits, FormatType::FracBits);
  EXPECT_NEAR(expected_value, fixed.ToFloat(), FixedPointTest::EPSILON);
}
//
//


TEST_F(FixedPointTest, SumPlan1)
{
  using FormatA = clarinoid::FxFormat<clarinoid::FxLayout<1, 31>, clarinoid::FxStorageTraits<uint32_t>>;
  using FormatB = clarinoid::FxFormat<clarinoid::FxLayout<2, 30>, clarinoid::FxStorageTraits<uint32_t>>;
  using plan = clarinoid::FxSmartKernel::SumPlan<FormatA, FormatB, false>;
  auto als = plan::ALeftShift;
  auto ars = plan::ARightShift;
  auto bls = plan::BLeftShift;
  EXPECT_EQ(1, plan::BLeftShift);  // intermediate shift to equalize A and B; B is now 2.31 uint64_t layout.
  auto brs = plan::BRightShift;

  EXPECT_EQ(3, plan::IdealMagBits);

  auto alsf = plan::ALeftShiftFinal;
  auto arsf = plan::ARightShiftFinal;
  auto blsf = plan::BLeftShiftFinal;
  auto brsf = plan::BRightShiftFinal;

  auto magBits = plan::ResultFormat::MagBits;
  auto fractBits = plan::ResultFormat::FracBits;
}


TEST_F(FixedPointTest, LiteralOperatorConstexpr)
{
  // Test that literal operators work in constexpr contexts

  //constexpr auto compile_time = 3.14159_fx;
  //TestFx<int32_t>(compile_time, 3.14159, true, 2, 30);

  {
    auto v = clarinoid::fx(10);
    //auto v = BuildParsedFromValue(3.14);
    //using D = decltype(v);
  }

  {
    using D = decltype(42_fxd);
    static_assert(42 == D::ParsedType::numerator);
    static_assert(1 == D::ParsedType::denominator);
    static_assert(false == D::ParsedType::neg);
    static_assert(42 == D::ParsedType::int_part);
    static_assert(0 == D::ParsedType::frac_part);
    static_assert(0 == D::ParsedType::exp10);
    static_assert(false == D::ParsedType::has_frac);
    static_assert(true == D::ParsedType::valid);
    static_assert(std::is_same_v<uint8_t, D::RawType>);
  }

  //constexpr auto c1 = 1.5_fx;

  //constexpr auto c2 = 2.5_fx;
  {
    using D = decltype(2.5_fxd);
    static_assert(25 == D::ParsedType::numerator);
    static_assert(10 == D::ParsedType::denominator);
    static_assert(false == D::ParsedType::neg);
    static_assert(2 == D::ParsedType::int_part);
    static_assert(5 == D::ParsedType::frac_part);
    static_assert(0 == D::ParsedType::exp10);
    static_assert(true == D::ParsedType::has_frac);
    static_assert(true == D::ParsedType::valid);
    static_assert(std::is_same_v<uint32_t, D::RawType>);
  }

  {
    using D = decltype(1.5_fxd);
    static_assert(15 == D::ParsedType::numerator);
    static_assert(10 == D::ParsedType::denominator);
    static_assert(false == D::ParsedType::neg);
    static_assert(1 == D::ParsedType::int_part);
    static_assert(5 == D::ParsedType::frac_part);
    static_assert(0 == D::ParsedType::exp10);
    static_assert(true == D::ParsedType::has_frac);
    static_assert(true == D::ParsedType::valid);
    static_assert(std::is_same_v<uint32_t, D::RawType>);

    static_assert(31 == D::FracBits);
    static_assert(1 == D::MagBits);
    static_assert(std::is_same_v<uint32_t, D::RawType>);
  }

  // Test constexpr arithmetic
  {
    constexpr auto a = 1.5_fx;
    TestFx<uint32_t>(a, 1.5, false, 1, 31);
    constexpr auto b = 2.5_fx;
    TestFx<uint32_t>(b, 2.5, false, 2, 30);
    auto runtimeSum = a + b;
    constexpr auto compile_time = a + b;
    TestFx<uint64_t>(runtimeSum, 4, false, 3, 31);  // magbits is 4??
    TestFx<uint64_t>(compile_time, 4, false, 3, 31);
  }

  {
    constexpr auto a = 127_fx;
    TestFx<uint8_t>(a, 127, false, 7, 0);
    constexpr auto b = 1_fx;
    TestFx<uint8_t>(b, 1, false, 1, 0);
    auto runtimeSum = a + b;
    constexpr auto compile_time = a + b;
    TestFx<uint8_t>(runtimeSum, 128, false, 8, 0);
    TestFx<uint8_t>(compile_time, 128, false, 8, 0);
  }

  {
    // even though the value doesn't actualyl overflow, it theoretically could so we end up with a promotion.
    constexpr auto a = 254_fx;
    TestFx<uint8_t>(a, 254, false, 8, 0);
    constexpr auto b = 1_fx;
    TestFx<uint8_t>(b, 1, false, 1, 0);
    auto runtimeSum = a + b;
    constexpr auto compile_time = a + b;
    TestFx<uint16_t>(runtimeSum, 255, false, 9, 0);
    TestFx<uint16_t>(compile_time, 255, false, 9, 0);
  }
}


TEST_F(FixedPointTest, Negate)
{
  {
    constexpr auto a = 1.5_fx;
    TestFx<uint32_t>(a, 1.5, false, 1, 31);
    constexpr auto b = -2.25_fx;
    TestFx<int64_t>(b, -2.25, true, 2, 30);
    auto runtimeSum = a + b;
    constexpr auto compile_time = a + b;
    TestFx<int64_t>(runtimeSum, -0.75, true, 3, 31);
    TestFx<int64_t>(compile_time, -0.75, true, 3, 31);
  }
}

TEST_F(FixedPointTest, Subtract)
{
  {
    constexpr auto a = 1.5_fx;
    TestFx<uint32_t>(a, 1.5, false, 1, 31);
    constexpr auto b = 2.25_fx;
    TestFx<uint32_t>(b, 2.25, false, 2, 30);
    auto runtimeSum = a - b;
    constexpr auto compile_time = a - b;
    TestFx<int64_t>(runtimeSum, -0.75, true, 3, 31);
    TestFx<int64_t>(compile_time, -0.75, true, 3, 31);
  }
}


//
//TEST_F(FixedPointTest, RuntimeConstruction_FracBits)
//{
//  // signed frac-bits only
//  auto a = clarinoid::fx<15>(3.25);  // expect int32 raw, MagBits=31-15=16
//  static_assert(std::is_same_v<decltype(a)::RawType, int32_t>);
//  static_assert(decltype(a)::FormatType::FracBits == 15);
//  static_assert(decltype(a)::FormatType::MagBits == 16);
//  static_assert(decltype(a)::FormatType::IsSigned);
//  EXPECT_NEAR(3.25, a.ToFloat(), EPSILON);
//
//  // unsigned frac-bits only
//  auto b = clarinoid::fx_u<10>(12.75);  // expect uint32 raw, MagBits=32-10=22
//  static_assert(std::is_same_v<decltype(b)::RawType, uint32_t>);
//  static_assert(decltype(b)::FormatType::FracBits == 10);
//  static_assert(decltype(b)::FormatType::MagBits == 22);
//  static_assert(!decltype(b)::FormatType::IsSigned);
//  EXPECT_NEAR(12.75, b.ToFloat(), EPSILON);
//}
//
//TEST_F(FixedPointTest, RuntimeConstruction_MagBits)
//{
//  // signed mag-bits only
//  auto a = clarinoid::fx_m<3>(5.5);  // expect int32 raw, FracBits=31-3=28
//  static_assert(std::is_same_v<decltype(a)::RawType, int32_t>);
//  static_assert(decltype(a)::FormatType::MagBits == 3);
//  static_assert(decltype(a)::FormatType::FracBits == 28);
//  EXPECT_NEAR(5.5, a.ToFloat(), EPSILON);
//
//  // unsigned mag-bits only
//  auto b = clarinoid::fx_m_u<8>(255.0);  // expect uint32 raw, FracBits=32-8=24
//  static_assert(std::is_same_v<decltype(b)::RawType, uint32_t>);
//  static_assert(decltype(b)::FormatType::MagBits == 8);
//  static_assert(decltype(b)::FormatType::FracBits == 24);
//  static_assert(!decltype(b)::FormatType::IsSigned);
//  EXPECT_NEAR(255.0, b.ToFloat(), 1e-3);  // some rounding possible
//}
//
//TEST_F(FixedPointTest, RuntimeConstruction_MagFracBits)
//{
//  auto a = clarinoid::fx_mf<10, 5>(123.625);  // 10 mag, 5 frac, signed
//  static_assert(std::is_same_v<decltype(a)::RawType, int32_t>);
//  static_assert(decltype(a)::FormatType::MagBits == 10);
//  static_assert(decltype(a)::FormatType::FracBits == 5);
//  EXPECT_NEAR(123.625, a.ToFloat(), EPSILON);
//
//  auto b = clarinoid::fx_mf_u<8, 8, uint16_t>(42.5);  // explicit uint16_t
//  static_assert(std::is_same_v<decltype(b)::RawType, uint16_t>);
//  static_assert(decltype(b)::FormatType::MagBits == 8);
//  static_assert(decltype(b)::FormatType::FracBits == 8);
//  static_assert(!decltype(b)::FormatType::IsSigned);
//  EXPECT_NEAR(42.5, b.ToFloat(), 1e-3);
//}
//
//TEST_F(FixedPointTest, RuntimeConstruction_DefaultFx)
//{
//  auto d = clarinoid::fx(2.5);  // default Q15.16 on int32
//  static_assert(std::is_same_v<decltype(d)::RawType, int32_t>);
//  static_assert(decltype(d)::FormatType::FracBits == 16);
//  static_assert(decltype(d)::FormatType::MagBits == 15);
//  EXPECT_NEAR(2.5, d.ToFloat(), EPSILON);
//}


}  // namespace FixedPointTests