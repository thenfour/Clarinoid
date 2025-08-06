#include <windows.h>

#include <gtest/gtest.h>

#include <clarinoid/core/basic/basic.hpp>
#include <clarinoid/core/basic/string/format.hpp>
#include <sstream>
#include <vector>

#if __cplusplus >= 202002L  // C++20 or later
  #include <format>
#endif


namespace StringFormat
{


template <size_t N, class... Ts>
size_t test_format_to(char (&buf)[N], std::string_view fmt, Ts&&... ts)
{
  const size_t ret = clarinoid::format_to(buf, fmt, std::forward<Ts>(ts)...);

#if __cplusplus >= 202002L  // C++20 or later
  // Only compare if there was no truncation; otherwise you'd compare partial output.
  if (ret < N)
  {
    // MSVC requires lvalues for make_format_args; bind as const lvalues.
    const std::string expected = std::vformat(fmt,
                                              std::make_format_args<std::format_context>(
                                                  static_cast<const std::remove_reference_t<Ts>&>(ts)...));

    std::string_view sv(buf, ret);
    EXPECT_STREQ(sv.data(), expected.data()) << "C++20 std::format mismatch for: " << fmt;
  }
#endif
  return ret;
}

template <class It, class... Ts>
It test_format_to(It it, const std::string_view fmt, Ts&&... ts)
{
  // wrapper for format_to which, if this is C++20, tests equality with the std implementation.
  auto ret = clarinoid::format_to(it, fmt, std::forward<Ts>(ts)...);
#if __cplusplus >= 202002L  // C++20 or later
  {
    std::string std_ret = std::vformat(fmt,
                                       std::make_format_args<std::format_context>(
                                           static_cast<const std::remove_reference_t<Ts>&>(ts)...));
    std::string ourResult;
    clarinoid::format_to(std::back_inserter(ourResult), fmt, std::forward<Ts>(ts)...);
    EXPECT_STREQ(ourResult.data(), std_ret.data()) << "C++20 std::format mismatch for: " << fmt;
  }
#endif
  return ret;
}

namespace ParseSpec
{
TEST(FormatSpecTest, DefaultSpec)
{
  auto spec = clarinoid::parse_spec("");
  EXPECT_EQ('>', spec.align);
  EXPECT_EQ(' ', spec.fill);
  EXPECT_EQ('\0', spec.sign);
  EXPECT_EQ(0, spec.width);
  EXPECT_EQ(-1, spec.prec);
  EXPECT_EQ('\0', spec.type);
}

TEST(FormatSpecTest, ParseAlign)
{
  auto spec1 = clarinoid::parse_spec("<");
  EXPECT_EQ('<', spec1.align);
  EXPECT_EQ(' ', spec1.fill);

  auto spec2 = clarinoid::parse_spec(">");
  EXPECT_EQ('>', spec2.align);

  auto spec3 = clarinoid::parse_spec("^");
  EXPECT_EQ('^', spec3.align);
}

TEST(FormatSpecTest, ParseFillAndAlign)
{
  auto spec1 = clarinoid::parse_spec("0<");
  EXPECT_EQ('<', spec1.align);
  EXPECT_EQ('0', spec1.fill);

  auto spec2 = clarinoid::parse_spec("*>");
  EXPECT_EQ('>', spec2.align);
  EXPECT_EQ('*', spec2.fill);

  auto spec3 = clarinoid::parse_spec("-^");
  EXPECT_EQ('^', spec3.align);
  EXPECT_EQ('-', spec3.fill);
}

TEST(FormatSpecTest, ParseSign)
{
  auto spec1 = clarinoid::parse_spec("+");
  EXPECT_EQ('+', spec1.sign);

  auto spec2 = clarinoid::parse_spec(" ");
  EXPECT_EQ(' ', spec2.sign);
}

TEST(FormatSpecTest, ParseWidth)
{
  auto spec1 = clarinoid::parse_spec("10");
  EXPECT_EQ(10, spec1.width);

  auto spec2 = clarinoid::parse_spec("5");
  EXPECT_EQ(5, spec2.width);

  auto spec3 = clarinoid::parse_spec("123");
  EXPECT_EQ(123, spec3.width);
}

TEST(FormatSpecTest, ParsePrecision)
{
  auto spec1 = clarinoid::parse_spec(".2");
  EXPECT_EQ(2, spec1.prec);

  auto spec2 = clarinoid::parse_spec(".0");
  EXPECT_EQ(0, spec2.prec);

  auto spec3 = clarinoid::parse_spec(".10");
  EXPECT_EQ(10, spec3.prec);
}

TEST(FormatSpecTest, ParseType)
{
  auto spec1 = clarinoid::parse_spec("f");
  EXPECT_EQ('f', spec1.type);

  auto spec2 = clarinoid::parse_spec("e");
  EXPECT_EQ('e', spec2.type);

  auto spec3 = clarinoid::parse_spec("x");
  EXPECT_EQ('x', spec3.type);
}

// Test that parse_spec does NOT expect leading ':'
TEST(FormatSpecTest, ParseSpecWithoutColon)
{
  auto spec1 = clarinoid::parse_spec("x");
  EXPECT_EQ('x', spec1.type);

  auto spec2 = clarinoid::parse_spec("#x");
  EXPECT_EQ('x', spec2.type);
  EXPECT_TRUE(spec2.alt);

  auto spec3 = clarinoid::parse_spec("08x");
  EXPECT_EQ('x', spec3.type);
  EXPECT_EQ(8, spec3.width);
  EXPECT_EQ('0', spec3.fill);
}

// Test zero-padding format specifications
TEST(FormatSpecTest, ZeroPadding)
{
  // Test basic zero-padding
  auto spec1 = clarinoid::parse_spec("08x");
  EXPECT_EQ('x', spec1.type);
  EXPECT_EQ(8, spec1.width);
  EXPECT_EQ('0', spec1.fill);
  EXPECT_EQ('>', spec1.align);  // Should default to right align for zero-padding

  // Test zero-padding with different widths
  auto spec2 = clarinoid::parse_spec("05d");
  EXPECT_EQ('d', spec2.type);
  EXPECT_EQ(5, spec2.width);
  EXPECT_EQ('0', spec2.fill);
  EXPECT_EQ('>', spec2.align);

  // Test that explicit fill/align overrides zero-padding
  auto spec3 = clarinoid::parse_spec("*<08x");
  EXPECT_EQ('x', spec3.type);
  EXPECT_EQ(8, spec3.width);
  EXPECT_EQ('*', spec3.fill);   // Explicit fill should override
  EXPECT_EQ('<', spec3.align);  // Explicit align should override

  // Test zero-padding with other format specs
  auto spec4 = clarinoid::parse_spec("#08x");
  EXPECT_EQ('x', spec4.type);
  EXPECT_EQ(8, spec4.width);
  EXPECT_EQ('0', spec4.fill);
  EXPECT_EQ('>', spec4.align);
  EXPECT_TRUE(spec4.alt);

  //#if __cplusplus >= 202002L  // C++20 or later
  //
  //  auto a1 = std::format("{:8x}", 3);   // right-aligns with spaces: "       3"
  //  auto a2 = std::format("{:08x}", 3);  // right-aligns with '0' padding: "00000003"
  //  auto a3 = std::format(
  //      "{:88x}",
  //      3);  // right-align with 88 spaces "                                                                                       3"
  //  auto a4 = std::format(
  //      "{:088x}",
  //      3);  // right-align with 88 spaces "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000003"
  //
  //#endif
  //
  //  // Test zero-padding with other format specs
  //  auto spec5 = clarinoid::parse_spec("8x");
  //  EXPECT_EQ('x', spec5.type);
  //  EXPECT_EQ(8, spec5.width);
  //  EXPECT_EQ('0', spec5.fill);
  //  EXPECT_EQ('>', spec5.align);
  //  EXPECT_FALSE(spec5.alt);
}

TEST(FormatSpecTest, PaddingEdgeCases)
{
  //auto a1 = std::format("{:8x}", 3);   // right-aligns with spaces: "       3"
  //auto a2 = std::format("{:08x}", 3);  // right-aligns with '0' padding: "00000003"
  //auto a3 = std::format(
  //    "{:88x}",
  //    3);  // right-align with 88 spaces "                                                                                       3"
  //auto a4 = std::format(
  //    "{:088x}",
  //    3);  // right-align with 88 spaces "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000003"
  {
    auto spec5 = clarinoid::parse_spec("08x");
    EXPECT_EQ('x', spec5.type);
    EXPECT_EQ(8, spec5.width);
    EXPECT_EQ('0', spec5.fill);
    EXPECT_EQ('>', spec5.align);
    EXPECT_FALSE(spec5.alt);
  }
  {
    auto spec5 = clarinoid::parse_spec("8x");
    EXPECT_EQ('x', spec5.type);
    EXPECT_EQ(8, spec5.width);
    EXPECT_EQ(' ', spec5.fill);
    EXPECT_EQ('>', spec5.align);
    EXPECT_FALSE(spec5.alt);
  }
  {
    auto spec5 = clarinoid::parse_spec("088x");
    EXPECT_EQ('x', spec5.type);
    EXPECT_EQ(88, spec5.width);
    EXPECT_EQ('0', spec5.fill);
    EXPECT_EQ('>', spec5.align);
    EXPECT_FALSE(spec5.alt);
  }
  {
    auto spec5 = clarinoid::parse_spec("88x");
    EXPECT_EQ('x', spec5.type);
    EXPECT_EQ(88, spec5.width);
    EXPECT_EQ(' ', spec5.fill);
    EXPECT_EQ('>', spec5.align);
    EXPECT_FALSE(spec5.alt);
  }
  {
    auto spec5 = clarinoid::parse_spec(">4c");
    EXPECT_EQ('c', spec5.type);
    EXPECT_EQ(4, spec5.width);
    EXPECT_EQ(' ', spec5.fill);
    EXPECT_EQ('>', spec5.align);
    EXPECT_FALSE(spec5.alt);
  }
}

TEST(FormatSpecTest, PaddingHugeBuffer)
{
  auto spec5 = clarinoid::parse_spec("0123x");
  EXPECT_EQ('x', spec5.type);
  EXPECT_EQ(123, spec5.width);
  EXPECT_EQ('0', spec5.fill);
  EXPECT_EQ('>', spec5.align);
  EXPECT_FALSE(spec5.alt);
}


}  // namespace ParseSpec

namespace Sink
{


// BufferSink tests
TEST(BufferSinkTest, WriteChar)
{
  char buf[10];
  clarinoid::BufferSink sink{buf, sizeof(buf)};

  sink.write('a');
  EXPECT_EQ(1, sink.len);
  EXPECT_EQ('a', buf[0]);
}

TEST(BufferSinkTest, WriteString)
{
  char buf[10];
  clarinoid::BufferSink sink{buf, sizeof(buf)};

  sink.write("hello", 5);
  EXPECT_EQ(5, sink.len);
  EXPECT_EQ(0, memcmp(buf, "hello", 5));
}

TEST(BufferSinkTest, WriteOverflow)
{
  char buf[5];
  clarinoid::BufferSink sink{buf, sizeof(buf)};

  sink.write("hello world", 11);
  EXPECT_EQ(11, sink.len);                // len reflects total written, not just what fit
  EXPECT_EQ(0, memcmp(buf, "hello", 5));  // only first 5 chars fit
}

// IteratorSink tests
TEST(IteratorSinkTest, WriteToVector)
{
  std::vector<char> vec;
  auto sink = clarinoid::make_iterator_sink(std::back_inserter(vec));

  sink.write('a');
  sink.write("bc", 2);

  EXPECT_EQ(3, vec.size());
  EXPECT_EQ('a', vec[0]);
  EXPECT_EQ('b', vec[1]);
  EXPECT_EQ('c', vec[2]);
}

// OStreamSink tests
TEST(OStreamSinkTest, WriteToStream)
{
  std::ostringstream oss;
  clarinoid::OStreamSink sink{oss};

  sink.write('a');
  sink.write("bc", 2);

  EXPECT_EQ("abc", oss.str());
}

}  // namespace Sink

// format_to array tests
TEST(FormatToArrayTest, SimpleString)
{
  char buf[20];
  size_t len = test_format_to(buf, "hello");
  EXPECT_EQ(5, len);
  EXPECT_STREQ("hello", buf);
}

TEST(FormatToArrayTest, IntegerFormatting)
{
  char buf[20];
  size_t len = test_format_to(buf, "value: {}", 42);
  EXPECT_EQ(9, len);
  EXPECT_STREQ("value: 42", buf);
}

TEST(FormatToArrayTest, NegativeInteger)
{
  char buf[20];
  size_t len = test_format_to(buf, "value: {}", -123);
  EXPECT_EQ(11, len);
  EXPECT_STREQ("value: -123", buf);
}

TEST(FormatToArrayTest, HexadecimalFormat)
{
  char buf[20];
  size_t len = test_format_to(buf, "hex: {:x}", 255);
  EXPECT_STREQ("hex: ff", buf);
}

// Base format specification tests
TEST(BaseFormatTest, HexadecimalLowercase)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:x}", 255);
  EXPECT_EQ(2, len);
  EXPECT_STREQ("ff", buf);
}

TEST(BaseFormatTest, HexadecimalUppercase)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:X}", 255);
  EXPECT_EQ(2, len);
  EXPECT_STREQ("FF", buf);
}

TEST(BaseFormatTest, OctalFormat)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:o}", 64);
  EXPECT_EQ(3, len);
  EXPECT_STREQ("100", buf);
}

TEST(BaseFormatTest, BinaryLowercase)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:b}", 5);
  EXPECT_EQ(3, len);
  EXPECT_STREQ("101", buf);
}

TEST(BaseFormatTest, BinaryUppercase)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:B}", 5);
  EXPECT_EQ(3, len);
  EXPECT_STREQ("101", buf);
}

TEST(BaseFormatTest, DecimalExplicit)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:d}", 42);
  EXPECT_EQ(2, len);
  EXPECT_STREQ("42", buf);
}

TEST(BaseFormatTest, CharacterFormat)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:c}", 65);
  EXPECT_EQ(1, len);
  EXPECT_STREQ("A", buf);
}

// Alternate form tests
TEST(AlternateFormTest, HexWithPrefix)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:#x}", 255);
  EXPECT_EQ(4, len);
  EXPECT_STREQ("0xff", buf);
}

TEST(AlternateFormTest, HexUppercaseWithPrefix)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:#X}", 255);
  EXPECT_EQ(4, len);
  EXPECT_STREQ("0XFF", buf);
}

TEST(AlternateFormTest, OctalWithPrefix)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:#o}", 64);
  EXPECT_EQ(4, len);
  EXPECT_STREQ("0100", buf);
}

TEST(AlternateFormTest, BinaryWithPrefix)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:#b}", 5);
  EXPECT_EQ(5, len);
  EXPECT_STREQ("0b101", buf);
}

TEST(AlternateFormTest, BinaryUppercaseWithPrefix)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:#B}", 5);
  EXPECT_EQ(5, len);
  EXPECT_STREQ("0B101", buf);
}

TEST(AlternateFormTest, ZeroValues)
{
  char buf[20];

  // Hex zero with prefix
  size_t len1 = test_format_to(buf, "{:#x}", 0);
  EXPECT_EQ(3, len1);
  EXPECT_STREQ("0x0", buf);

  // Octal zero (no extra prefix for zero)
  size_t len2 = test_format_to(buf, "{:#o}", 0);
  EXPECT_EQ(1, len2);
  EXPECT_STREQ("0", buf);

  // Binary zero with prefix
  size_t len3 = test_format_to(buf, "{:#b}", 0);
  EXPECT_EQ(3, len3);
  EXPECT_STREQ("0b0", buf);
}

// Complex base formatting with other specs
TEST(BaseFormatComplexTest, HexWithWidthAndFill)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:0>8x}", 255);
  EXPECT_EQ(8, len);
  EXPECT_STREQ("000000ff", buf);
}

TEST(BaseFormatComplexTest, HexWithPrefixAndWidth)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:#08x}", 255);
  EXPECT_EQ(8, len);
  EXPECT_STREQ("0x0000ff", buf);
}

TEST(BaseFormatComplexTest, BinaryWithAlignment)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:^10b}", 5);
  EXPECT_EQ(10, len);
  EXPECT_STREQ("   101    ", buf);
}

// Padding and alignment tests
TEST(FormatPaddingTest, RightAlign)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:>5}", "hi");
  EXPECT_EQ(5, len);
  EXPECT_STREQ("   hi", buf);
}

TEST(FormatPaddingTest, LeftAlign)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:<5}", "hi");
  EXPECT_EQ(5, len);
  EXPECT_STREQ("hi   ", buf);
}

TEST(FormatPaddingTest, CenterAlign)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:^5}", "hi");
  EXPECT_EQ(5, len);
  EXPECT_STREQ(" hi  ", buf);
}

TEST(FormatPaddingTest, CustomFill)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:*>5}", "hi");
  EXPECT_EQ(5, len);
  EXPECT_STREQ("***hi", buf);
}

TEST(FormatPaddingTest, IntegerPadding)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:05}", 42);
  EXPECT_EQ(5, len);
  EXPECT_STREQ("00042", buf);
}

// Test zero-padding functionality
TEST(FormatPaddingTest, ZeroPaddingFunctionality)
{
  char buf[20];

  // Test basic zero-padding
  size_t len1 = test_format_to(buf, "{:05d}", 42);
  EXPECT_EQ(5, len1);
  EXPECT_STREQ("00042", buf);

  // Test hex zero-padding
  size_t len2 = test_format_to(buf, "{:08x}", 255);
  EXPECT_EQ(8, len2);
  EXPECT_STREQ("000000ff", buf);

  // Test binary zero-padding
  size_t len3 = test_format_to(buf, "{:08b}", 5);
  EXPECT_EQ(8, len3);
  EXPECT_STREQ("00000101", buf);

  // Test that it's equivalent to explicit 0> formatting
  size_t len4 = test_format_to(buf, "{:0>8x}", 255);
  EXPECT_EQ(8, len4);
  EXPECT_STREQ("000000ff", buf);

  // Test zero-padding with alternate form
  size_t len5 = test_format_to(buf, "{:#08x}", 255);
  EXPECT_EQ(8, len5);
  EXPECT_STREQ("0x0000ff", buf);
}

// Sign formatting tests
TEST(FormatSignTest, ForceSign)
{
  char buf[20];
  size_t len = test_format_to(buf, "{:+}", 42);
  EXPECT_EQ(3, len);
  EXPECT_STREQ("+42", buf);
}

TEST(FormatSignTest, SpaceForPositive)
{
  char buf[20];
  size_t len = test_format_to(buf, "{: }", 42);
  EXPECT_EQ(3, len);
  EXPECT_STREQ(" 42", buf);
}

// Escape sequences tests
TEST(FormatEscapeTest, DoubleBraces)
{
  char buf[20];
  size_t len = test_format_to(buf, "{{hello}}");
  EXPECT_EQ(7, len);
  EXPECT_STREQ("{hello}", buf);
}

TEST(FormatEscapeTest, MixedBraces)
{
  char buf[30];
  size_t len = test_format_to(buf, "{{}} and {}", 42);
  EXPECT_STREQ("{} and 42", buf);
}

// format_to iterator tests
TEST(FormatToIteratorTest, BackInserter)
{
  std::vector<char> vec;
  auto it = test_format_to(std::back_inserter(vec), "test: {}", 123);

  std::string result(vec.begin(), vec.end());
  EXPECT_EQ("test: 123", result);
}

// Edge cases and error handling
TEST(FormatEdgeCasesTest, EmptyFormat)
{
  char buf[10];
  size_t len = test_format_to(buf, "");
  EXPECT_EQ(0, len);
  EXPECT_STREQ("", buf);
}

TEST(FormatEdgeCasesTest, NoArguments)
{
  char buf[20];
  size_t len = test_format_to(buf, "no args");
  EXPECT_STREQ("no args", buf);
}

TEST(FormatEdgeCasesTest, ZeroValue)
{
  char buf[20];
  size_t len = test_format_to(buf, "zero: {}", 0);
  EXPECT_EQ(7, len);
  EXPECT_STREQ("zero: 0", buf);
}

TEST(FormatEdgeCasesTest, NegativeHex)
{
  char buf[20];
  size_t len = test_format_to(buf, "hex: {:x}", -42);
  EXPECT_STREQ("hex: -2a", buf);
}

// Additional edge case tests for base formatting
TEST(BaseFormatEdgeCasesTest, NegativeNumbers)
{
  char buf[30];

  // Negative decimal should still show sign
  size_t len1 = test_format_to(buf, "{:d}", -42);
  EXPECT_STREQ("-42", buf);

  // Negative hex shows unsigned representation
  size_t len2 = test_format_to(buf, "{:x}", -42);
  EXPECT_STREQ("-2a", buf);

  // Negative binary shows unsigned representation
  size_t len3 = test_format_to(buf, "{:b}", -43);
  EXPECT_STREQ("-101011", buf);
}

TEST(BaseFormatEdgeCasesTest, SignsWithBases)
{
  char buf[20];

  // Plus sign with decimal
  size_t len1 = test_format_to(buf, "{:+d}", 42);
  EXPECT_STREQ("+42", buf);

  // Space sign with decimal
  size_t len2 = test_format_to(buf, "{: d}", 42);
  EXPECT_STREQ(" 42", buf);

  size_t len3 = test_format_to(buf, "{:+x}", 42);
  EXPECT_STREQ("+2a", buf);
}

TEST(BaseFormatEdgeCasesTest, NonASCIICharacter)
{
  char buf[20];

  // Non-ASCII value should fall back to decimal.
  // C++20 throws for this.
  size_t len = clarinoid::format_to(buf, "{:c}", 200);
  EXPECT_EQ(3, len);
  EXPECT_STREQ("200", buf);
}

// Quick test of new base formatting functionality
TEST(BaseFormatBasicTest, SanityCheck)
{
  char buf[20];

  // Test hex
  size_t len1 = test_format_to(buf, "{:x}", 255);
  EXPECT_EQ(2, len1);
  EXPECT_STREQ("ff", buf);

  // Test binary
  size_t len2 = test_format_to(buf, "{:b}", 5);
  EXPECT_EQ(3, len2);
  EXPECT_STREQ("101", buf);

  // Test octal
  size_t len3 = test_format_to(buf, "{:o}", 8);
  EXPECT_EQ(2, len3);
  EXPECT_STREQ("10", buf);

  // Test alternate form
  size_t len4 = test_format_to(buf, "{:#x}", 255);
  EXPECT_EQ(4, len4);
  EXPECT_STREQ("0xff", buf);
}

// Regression test for the colon parsing bug
TEST(BaseFormatBasicTest, ColonParsingRegression)
{
  char buf[20];

  // This was failing before the fix because ":x" was being passed to parse_spec
  // instead of "x", causing the type to be parsed as ':' instead of 'x'
  size_t len = test_format_to(buf, "{:x}", 255);
  EXPECT_EQ(2, len);
  EXPECT_STREQ("ff", buf);

  // Test a few more cases to be sure
  len = test_format_to(buf, "{:X}", 255);
  EXPECT_EQ(2, len);
  EXPECT_STREQ("FF", buf);

  len = test_format_to(buf, "{:o}", 8);
  EXPECT_STREQ("10", buf);

  len = test_format_to(buf, "{:b}", 5);
  EXPECT_EQ(3, len);
  EXPECT_STREQ("101", buf);
}

// Test additional integer types
TEST(RefactoredCodeTest, AdditionalIntegerTypes)
{
  char buf[30];

  // Test long
  size_t len = test_format_to(buf, "long: {}", -12345L);
  EXPECT_STREQ("long: -12345", buf);

  // Test unsigned long
  len = test_format_to(buf, "ulong: {}", 12345UL);
  EXPECT_STREQ("ulong: 12345", buf);

  // Test long long
  len = test_format_to(buf, "ll: {:#x}", -0xffffffffffffLL);
  EXPECT_STREQ("ll: -0xffffffffffff", buf);

  // Test unsigned long long
  len = test_format_to(buf, "ull: {:#x}", 0xffffffffffffULL);
  EXPECT_STREQ("ull: 0xffffffffffff", buf);

  // Test formatted versions
  len = test_format_to(buf, "hex: {:x}", 255L);
  EXPECT_STREQ("hex: ff", buf);

  len = test_format_to(buf, "bin: {:b}", 5LL);
  EXPECT_STREQ("bin: 101", buf);
}


// ...existing code...

// ------------------------------------------------------------------
//  Additional coverage – floating point, sign/prefix/zero-pad combo,
//  char formatting with width/fill, width-vs-data, buffer overflow.
// ------------------------------------------------------------------

namespace FloatingPoint
{

TEST(FloatingPointTest, BasicFixedTwoDecimals)
{
  char buf[32];
  size_t len = test_format_to(buf, "{:.2f}", 3.14159);
  EXPECT_EQ(4, len);
  EXPECT_STREQ("3.14", buf);
}

TEST(FloatingPointTest, WidthAndSign)
{
  // width = 8, sign always, precision = 2  → "   +3.14"
  char buf[32];
  size_t len = test_format_to(buf, "{:+8.2f}", 3.14159);
  EXPECT_EQ(8, len);
  EXPECT_STREQ("   +3.14", buf);
}

TEST(FloatingPointTest, ExponentFormat)
{
  char buf[40];
  size_t len = test_format_to(buf, "{:.1e}", 1234.0);
  EXPECT_EQ(7, len);  // "1.2e+03"
  EXPECT_STREQ("1.2e+03", buf);
}

TEST(FloatingPointTest, NegativeNumber)
{
  char buf[32];
  size_t len = test_format_to(buf, "{:.3f}", -2.5);
  EXPECT_EQ(6, len);
  EXPECT_STREQ("-2.500", buf);
}

}  // namespace FloatingPoint

namespace SignPrefixZeroPad
{

TEST(SignPrefixZeroPadTest, PlusAltZeroPadHex)
{
  // + / # / width 8 / 0-pad → "+0x000ff"
  char buf[20];
  size_t len = test_format_to(buf, "{:+#08x}", 255);
  EXPECT_EQ(8, len);
  EXPECT_STREQ("+0x000ff", buf);
}

}  // namespace SignPrefixZeroPad

namespace CharFormatting
{

TEST(CharFormattingTest, WidthDefaultLeftAlign)
{
  char buf[8];
  // default is left-alignment.
  size_t len = test_format_to(buf, "{:4c}", 65);  // 'A'
  EXPECT_EQ(4, len);
  EXPECT_STREQ("A   ", buf);
}

TEST(CharFormattingTest, WidthExplicitLeftAlign)
{
  char buf[8];
  // default is left-alignment.
  size_t len = test_format_to(buf, "{:<4c}", 65);  // 'A'
  EXPECT_EQ(4, len);
  EXPECT_STREQ("A   ", buf);
}

TEST(CharFormattingTest, WidthRightAlign)
{
  char buf[8];
  size_t len = test_format_to(buf, "{:>4c}", 65);  // 'A'
  EXPECT_EQ(4, len);
  EXPECT_STREQ("   A", buf);
}

TEST(CharFormattingTest, WidthLeftAlignCustomFill)
{
  char buf[8];
  size_t len = test_format_to(buf, "{:*<4c}", 65);
  EXPECT_EQ(4, len);
  EXPECT_STREQ("A***", buf);
}

TEST(CharFormattingTest, VeryLargePadding)
{
  char buf[1000];
  size_t len;
  
  //len = clarinoid::format_to(buf, "{:0123c}", 65);
  //EXPECT_EQ(123, len);
  //EXPECT_STREQ("A000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
  //             "00000000000000000000",
  //             buf);

  len = clarinoid::format_to(buf, "{:123c}", 65);
  EXPECT_EQ(123, len);
  EXPECT_STREQ("A                                                                                                      "
               "                    ",
               buf);

  len = test_format_to(buf, "{:0>123c}", 65);
  EXPECT_EQ(123, len);
  EXPECT_STREQ("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
               "0000000000000000000A",
               buf);

  len = test_format_to(buf, "{:>123c}", 65);
  EXPECT_EQ(123, len);
  EXPECT_STREQ("                                                                                                       "
               "                   A",
               buf);

  len = test_format_to(buf, "{:0<123c}", 65);
  EXPECT_EQ(123, len);
  EXPECT_STREQ("A000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
               "00000000000000000000",
               buf);

  len = test_format_to(buf, "{:<123c}", 65);
  EXPECT_EQ(123, len);
  EXPECT_STREQ("A                                                                                                      "
               "                    ",
               buf);

  len = test_format_to(buf, "{:0^123c}", 65);
  EXPECT_EQ(123, len);
  EXPECT_STREQ("0000000000000000000000000000000000000000000000000000000000000A0000000000000000000000000000000000000000000000000000000000000",
               buf);

  len = test_format_to(buf, "{:^123c}", 65);
  EXPECT_EQ(123, len);
  EXPECT_STREQ("                                                             A                                         "
               "                    ",
               buf);
}

}  // namespace CharFormatting

namespace WidthVsData
{

TEST(WidthVsDataTest, DataLongerThanWidthNoTruncation)
{
  char buf[16];
  size_t len = test_format_to(buf, "{:5}", "abcdef");
  EXPECT_EQ(6, len);
  EXPECT_STREQ("abcdef", buf);
}

}  // namespace WidthVsData

namespace BufferOverflow
{

TEST(BufferOverflowTest, ArrayTooSmallRetIsFullLength)
{
  char buf[5] = {};  // room for 4 chars + '\0'
  size_t len = clarinoid::format_to(buf, "overflow");
  EXPECT_GT(len, sizeof(buf));            // logical length > capacity
  EXPECT_EQ(0, strncmp(buf, "over", 4));  // first 4 chars copied
}

}  // namespace BufferOverflow


// ------------------------------------------------------------------
//  String-argument coverage – width / align / fill / no-truncation.
// ------------------------------------------------------------------
namespace StringArguments
{

TEST(StringArgTest, RightAlignWidth)
{
  char buf[32];
  size_t len = test_format_to(buf, "{:>10}", std::string_view("abc"));
  EXPECT_EQ(10, len);
  EXPECT_STREQ("       abc", buf);
}

TEST(StringArgTest, LeftAlignWidth)
{
  char buf[32];
  size_t len = test_format_to(buf, "{:<10}", "abc");
  EXPECT_EQ(10, len);
  EXPECT_STREQ("abc       ", buf);
}

TEST(StringArgTest, CenterAlignCustomFill)
{
  char buf[32];
  size_t len = test_format_to(buf, "{:*^9}", "abc");
  EXPECT_EQ(9, len);
  EXPECT_STREQ("***abc***", buf);
}

TEST(StringArgTest, WidthLessThanDataNoTruncation)
{
  char buf[32];
  size_t len = test_format_to(buf, "{:5}", "abcdefgh");
  EXPECT_EQ(8, len);  // data longer than width – stays intact
  EXPECT_STREQ("abcdefgh", buf);
}

TEST(StringArgTest, EmbeddedInSentence)
{
  char buf[64];
  size_t len = test_format_to(buf, "name: '{}'", "clarinoid");
  EXPECT_EQ(17, len);
  EXPECT_STREQ("name: 'clarinoid'", buf);
}

}  // namespace StringArguments


}  // namespace StringFormat