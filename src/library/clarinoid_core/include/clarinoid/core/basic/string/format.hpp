#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>    // for std::sprintf, std::snprintf
#include <cstring>   // for std::strlen
#include <iterator>
#include <ostream>
#include <string_view>
#include <tuple>     // for std::tuple_size_v
#include <type_traits>

/*--------------------------------------------------------------------
 * 1. Generic "Sink" concept: any type that provides
 *      void write(char)             – single-byte
 *      void write(const char*, n)   – block
 *------------------------------------------------------------------*/

namespace clarinoid
{

// Helper function for safe buffer copying
inline void safe_buffer_copy(char* dest, size_t dest_cap, size_t& dest_len, 
                            const char* src, size_t src_len)
{
  size_t room = (dest_len < dest_cap) ? dest_cap - dest_len : 0;
  size_t copy = (src_len < room) ? src_len : room;
  for (size_t i = 0; i < copy; ++i)
    dest[dest_len + i] = src[i];
  dest_len += src_len;  // Update length by actual amount requested, not just copied
}

// sinks ------------------------------------------------
struct BufferSink
{
  char* ptr;
  size_t cap;
  size_t len{0};
  
  void write(const char* s, size_t n)
  {
    safe_buffer_copy(ptr, cap, len, s, n);
  }
  
  void write(char c)
  {
    write(&c, 1);
  }
};

template <class It>
struct IteratorSink
{
  It it;
  void write(const char* s, size_t n)
  {
    while (n--)
      *it++ = *s++;
  }
  void write(char c)
  {
    *it++ = c;
  }
};

template <typename It>
auto make_iterator_sink(It it)
{
  return IteratorSink<It>{it};
}

struct OStreamSink
{
  std::ostream& os;
  void write(const char* s, size_t n)
  {
    os.write(s, static_cast<std::streamsize>(n));
  }
  void write(char c)
  {
    os.put(c);
  }
};


/*--------------------------------------------------------------------
 * 2. Small helpers
 *------------------------------------------------------------------*/
struct FormatSpec
{
  char align = '>';     // '<', '>', '^'
  char fill = ' ';
  char sign = '\0';     // '+', ' '
  uint16_t width = 0;
  int16_t prec = -1;    // -1 = not specified
  char type = '\0';     // 'x','X','o','b','B','d','c'
  bool alt = false;     // '#' prefix for alternate form
};

inline FormatSpec parse_spec(std::string_view sv)
{
  FormatSpec fs;
  size_t i = 0;

  // [fill][align]
  if (i + 1 < sv.size() && (sv[i + 1] == '<' || sv[i + 1] == '>' || sv[i + 1] == '^'))
  {
    fs.fill = sv[i];
    fs.align = sv[i + 1];
    i += 2;
  }
  else if (i < sv.size() && (sv[i] == '<' || sv[i] == '>' || sv[i] == '^'))
  {
    fs.align = sv[i++];
  }

  // sign
  if (i < sv.size() && (sv[i] == '+' || sv[i] == ' '))
  {
    fs.sign = sv[i++];
  }

  // alternate form
  if (i < sv.size() && sv[i] == '#')
  {
    fs.alt = true;
    ++i;
  }

  // zero-padding: if width starts with '0', it means zero-padding with right alignment
  bool zero_padding = false;
  if (i < sv.size() && sv[i] == '0' && fs.align == '>' && fs.fill == ' ') {
    // Only apply zero-padding if no explicit fill/align was specified
    zero_padding = true;
    fs.fill = '0';
    // fs.align remains '>' (right-aligned)
    ++i;
  }

  // width
  while (i < sv.size() && std::isdigit(static_cast<unsigned char>(sv[i])))
    fs.width = static_cast<uint16_t>(fs.width * 10 + (sv[i++] - '0'));

  // precision
  if (i < sv.size() && sv[i] == '.')
  {
    ++i;
    fs.prec = 0;
    while (i < sv.size() && std::isdigit(static_cast<unsigned char>(sv[i])))
      fs.prec = static_cast<int16_t>(fs.prec * 10 + (sv[i++] - '0'));
  }

  // type
  if (i < sv.size())
    fs.type = sv[i];
  return fs;
}

/*
 * parse_spec - Parses a format specification string
 * 
 * Input: A format specification string WITHOUT the leading ':' character
 * Examples of valid inputs:
 *   ""        -> default formatting
 *   "x"       -> hexadecimal lowercase
 *   "X"       -> hexadecimal uppercase  
 *   "08x"     -> hexadecimal with zero-padding to width 8 (equivalent to "0>8x")
 *   "#x"      -> hexadecimal with 0x prefix
 *   "0>+10.2f" -> complex spec: '0' fill, '>' align, '+' sign, width 10, precision 2, 'f' type
 * 
 * Format specification grammar (in order):
 *   [fill][align][sign][#][0][width][.precision][type]
 * 
 * Where:
 *   fill:      any character (when followed by align)
 *   align:     '<' (left), '>' (right), '^' (center)
 *   sign:      '+' (always show), ' ' (space for positive), '-' (default, only negative)
 *   #:         alternate form flag (adds prefixes like 0x, 0b, 0)
 *   0:         zero-padding flag (equivalent to fill='0', align='>')
 *   width:     decimal number specifying minimum width
 *   precision: decimal number after '.' specifying precision
 *   type:      format type character ('x', 'X', 'o', 'b', 'B', 'd', 'c', 'f', 'e', etc.)
 */

template <class Sink>
inline void pad(Sink& s, char ch, size_t n)
{
  while (n--)
    s.write(ch);
}

template <class Sink>
void write_padded(Sink& s, std::string_view data, const FormatSpec& fs)
{
  size_t padlen = fs.width > data.size() ? fs.width - data.size() : 0;
  switch (fs.align)
  {
    case '<':
      s.write(data.data(), data.size());
      pad(s, fs.fill, padlen);
      break;
    case '^':
      pad(s, fs.fill, padlen / 2);
      s.write(data.data(), data.size());
      pad(s, fs.fill, padlen - padlen / 2);
      break;
    default: // '>' or any other value defaults to right align
      pad(s, fs.fill, padlen);
      s.write(data.data(), data.size());
      break;
  }
}

// Helper function for writing strings with optional formatting
template <class Sink>
void write_string_arg(Sink& s, std::string_view sv, const FormatSpec* fs = nullptr)
{
  if (fs) {
    write_padded(s, sv, *fs);
  } else {
    s.write(sv.data(), sv.size());
  }
}

/*--------------------------------------------------------------------
 * 3. write_arg overloads for different types
 *------------------------------------------------------------------*/

// Helper function for decimal conversion without formatting
template <class Sink, typename IntType>
void write_decimal_simple(Sink& sink, IntType v)
{
  static_assert(std::is_integral_v<IntType>);
  
  char buf[32]; // Large enough for any integral type
  char* p = buf + sizeof(buf);
  
  // Handle signed types
  bool neg = false;
  typename std::make_unsigned<IntType>::type val;
  if constexpr (std::is_signed_v<IntType>) {
    neg = v < 0;
    val = neg ? -static_cast<typename std::make_unsigned<IntType>::type>(v) : v;
  } else {
    val = v;
  }
  
  do {
    *--p = char('0' + val % 10);
    val /= 10;
  } while (val);
  
  if (neg) {
    *--p = '-';
  }
  
  sink.write(p, buf + sizeof(buf) - p);
}

// Helper function for integer formatting with FormatSpec
template <class Sink, typename IntType>
void write_integer_formatted(Sink& out, IntType v, const FormatSpec& fs)
{
  static_assert(std::is_integral_v<IntType>);
  
  char buf[64]; // Large enough for binary representation and prefixes
  char* p = buf + sizeof(buf);
  
  // Handle signed types for negative values
  bool neg = false;
  typename std::make_unsigned<IntType>::type val;
  if constexpr (std::is_signed_v<IntType>) {
    neg = v < 0;
    val = neg ? -static_cast<typename std::make_unsigned<IntType>::type>(v) : v;
  } else {
    val = v;
  }
  
  // Determine base and formatting options
  unsigned base;
  bool uppercase = false;
  
  switch (fs.type) {
    case 'x':  // lowercase hex
      base = 16;
      break;
    case 'X':  // uppercase hex
      base = 16;
      uppercase = true;
      break;
    case 'o':  // octal
      base = 8;
      break;
    case 'b':  // lowercase binary
      base = 2;
      break;
    case 'B':  // uppercase binary
      base = 2;
      uppercase = true;
      break;
    case 'c':  // character
      if (val <= 127) {  // ASCII range
        out.write(static_cast<char>(val));
        return;
      }
      // Fall through to decimal for non-ASCII
      [[fallthrough]];
    case 'd':  // decimal (explicit)
    case '\0': // decimal (default)
    default:
      base = 10;
      break;
  }

  // Generate digits
  auto digit = [&](unsigned d) -> char {
    if (d < 10) {
      return '0' + d;
    } else {
      return (uppercase ? 'A' : 'a') + (d - 10);
    }
  };

  if (val == 0) {
    *--p = '0';
  } else {
    while (val > 0) {
      *--p = digit(val % base);
      val /= base;
    }
  }

  // Add alternate form prefixes if requested
  if (fs.alt && base != 10) {
    switch (base) {
      case 16:
        *--p = uppercase ? 'X' : 'x';
        *--p = '0';
        break;
      case 8:
        if (*(p) != '0') {  // Don't add prefix if number is already 0
          *--p = '0';
        }
        break;
      case 2:
        *--p = uppercase ? 'B' : 'b';
        *--p = '0';
        break;
    }
  }

  // Add sign for decimal numbers
  if (base == 10) {
    if constexpr (std::is_signed_v<IntType>) {
      if (neg) {
        *--p = '-';
      } else if (fs.sign == '+') {
        *--p = '+';
      } else if (fs.sign == ' ') {
        *--p = ' ';
      }
    } else {
      // Unsigned types can still have + or space for positive values
      if (fs.sign == '+') {
        *--p = '+';
      } else if (fs.sign == ' ') {
        *--p = ' ';
      }
    }
  }

  write_padded(out, {p, static_cast<size_t>(buf + sizeof(buf) - p)}, fs);
}

// Basic write_arg without formatting
template <class Sink>
void write_arg(Sink& sink, const char* str)
{
  write_string_arg(sink, {str, std::strlen(str)});
}

template <class Sink>
void write_arg(Sink& sink, std::string_view sv)
{
  write_string_arg(sink, sv);
}

template <class Sink>
void write_arg(Sink& sink, int v)
{
  write_decimal_simple(sink, v);
}

template <class Sink>
void write_arg(Sink& sink, unsigned int v)
{
  write_decimal_simple(sink, v);
}

// Add support for common integer types
template <class Sink>
void write_arg(Sink& sink, long v)
{
  write_decimal_simple(sink, v);
}

template <class Sink>
void write_arg(Sink& sink, unsigned long v)
{
  write_decimal_simple(sink, v);
}

template <class Sink>
void write_arg(Sink& sink, long long v)
{
  write_decimal_simple(sink, v);
}

template <class Sink>
void write_arg(Sink& sink, unsigned long long v)
{
  write_decimal_simple(sink, v);
}

// write_arg with formatting support
template <class Sink>
void write_arg(Sink& s, double v, const FormatSpec& fs)
{
  char tmp[48];
  // Build a tiny printf format like "%+10.3f"
  char fmt[16];
  char* f = fmt;
  *f++ = '%';
  if (fs.sign)
    *f++ = fs.sign;
  if (fs.width)
  {
    f += std::sprintf(f, "%u", fs.width);
  }
  if (fs.prec >= 0)
  {
    *f++ = '.';
    f += std::sprintf(f, "%u", static_cast<unsigned>(fs.prec));
  }
  *f++ = (fs.type == 'e') ? 'e' : 'f';
  *f = '\0';

  int n = std::snprintf(tmp, sizeof(tmp), fmt, v);
  if (n < 0)
    n = 0;
  // tmp now has sign already, so ignore fs.sign in padding
  FormatSpec padSpec = fs;
  padSpec.sign = '\0';  // prevent double sign
  write_padded(s, {tmp, static_cast<size_t>(n)}, padSpec);
}

template <class Sink>
void write_arg(Sink& out, int v, const FormatSpec& fs)
{
  write_integer_formatted(out, v, fs);
}

template <class Sink>
void write_arg(Sink& out, unsigned int v, const FormatSpec& fs)
{
  write_integer_formatted(out, v, fs);
}

// Add formatted support for common integer types
template <class Sink>
void write_arg(Sink& out, long v, const FormatSpec& fs)
{
  write_integer_formatted(out, v, fs);
}

template <class Sink>
void write_arg(Sink& out, unsigned long v, const FormatSpec& fs)
{
  write_integer_formatted(out, v, fs);
}

template <class Sink>
void write_arg(Sink& out, long long v, const FormatSpec& fs)
{
  write_integer_formatted(out, v, fs);
}

template <class Sink>
void write_arg(Sink& out, unsigned long long v, const FormatSpec& fs)
{
  write_integer_formatted(out, v, fs);
}

template <class Sink>
void write_arg(Sink& s, const char* str, const FormatSpec& fs)
{
  write_string_arg(s, {str, std::strlen(str)}, &fs);
}

template <class Sink>
void write_arg(Sink& s, std::string_view sv, const FormatSpec& fs)
{
  write_string_arg(s, sv, &fs);
}

/*--------------------------------------------------------------------
 * 4. Tuple argument indexing - with bounds checking
 *------------------------------------------------------------------*/

template <size_t I, class Sink, class Tup>
struct write_by_idx_helper
{
  static void call(Sink& s, size_t idx, const FormatSpec& fs, Tup& tup)
  {
    constexpr size_t tuple_size = std::tuple_size<typename std::remove_reference<Tup>::type>::value;
    
    if constexpr (I < tuple_size) {
      if (idx == I) {
        // Check if we have any non-default formatting specs
        if (fs.width > 0 || fs.prec >= 0 || fs.type != '\0' || fs.align != '>' || fs.fill != ' ' || fs.sign != '\0' || fs.alt) {
          // Use formatting version if any format specs are present
          write_arg(s, std::get<I>(tup), fs);
        } else {
          // Use simple version for no formatting
          write_arg(s, std::get<I>(tup));
        }
      } else {
        write_by_idx_helper<I + 1, Sink, Tup>::call(s, idx, fs, tup);
      }
    }
    // If I >= tuple_size, do nothing (no more elements to check)
  }
};

template <class Sink, class Tup>
void write_by_idx(Sink& s, size_t idx, const FormatSpec& fs, Tup& tup)
{
  constexpr size_t tuple_size = std::tuple_size<typename std::remove_reference<Tup>::type>::value;
  if (tuple_size > 0 && idx < tuple_size) {
    write_by_idx_helper<0, Sink, Tup>::call(s, idx, fs, tup);
  }
  // If tuple is empty or index out of bounds, do nothing
}

/*--------------------------------------------------------------------
 * The formatter – parses "{}" and forwards each arg
 *------------------------------------------------------------------*/
template <class Sink, class... Ts>
void vformat(Sink& s, std::string_view fmt, Ts&&... args)
{
  auto tup = std::forward_as_tuple(std::forward<Ts>(args)...);
  size_t argi = 0;
  for (size_t i = 0; i < fmt.size();)
  {
    if (fmt[i] == '{')
    {
      if (i + 1 < fmt.size() && fmt[i + 1] == '{')
      {
        s.write('{');
        i += 2;
        continue;
      }
      size_t start = ++i;
      while (i < fmt.size() && fmt[i] != '}')
        ++i;
      if (i == fmt.size())
        break;
      
      // Extract the format specification without the leading ':'
      std::string_view spec_str = fmt.substr(start, i - start);
      if (!spec_str.empty() && spec_str[0] == ':') {
        spec_str = spec_str.substr(1);  // Skip the ':' character
      }
      
      FormatSpec fs = parse_spec(spec_str);
      write_by_idx(s, argi++, fs, tup);
      ++i;
    }
    else if (fmt[i] == '}' && i + 1 < fmt.size() && fmt[i + 1] == '}')
    {
      s.write('}');
      i += 2;
    }
    else
      s.write(fmt[i++]);
  }
}

/*--------------------------------------------------------------------
 * Convenience wrappers
 *------------------------------------------------------------------*/
template <size_t N, class... Ts>
size_t format_to(char (&buf)[N], const std::string_view fmt, Ts&&... ts)
{
  BufferSink sink{buf, N};
  vformat(sink, fmt, std::forward<Ts>(ts)...);
  if (sink.len < N)
    buf[sink.len] = '\0';  // NUL-terminate if space
  return sink.len;
}

template <class It, class... Ts>
It format_to(It it, const std::string_view fmt, Ts&&... ts)
{
  IteratorSink<It> sink{it};
  vformat(sink, fmt, std::forward<Ts>(ts)...);
  return sink.it;
}

template <class... Ts>
void format_to(std::ostream& os, const std::string_view fmt, Ts&&... ts)
{
  OStreamSink sink{os};
  vformat(sink, fmt, std::forward<Ts>(ts)...);
}

}  // namespace clarinoid