// minimal_format.hpp  -----------------------------------------------
#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <iterator>
#include <ostream>

/*--------------------------------------------------------------------
 * 1. Generic "Sink" concept: any type that provides
 *      void write(char)             – single-byte
 *      void write(const char*, n)   – block
 *------------------------------------------------------------------*/

namespace clarinoid
{

// sinks ------------------------------------------------
struct BufferSink
{
  char* ptr;
  size_t cap;
  size_t len{0};
  void write(const char* s, size_t n)
  {
    size_t room = (len < cap) ? cap - len : 0;
    size_t copy = (n < room) ? n : room;
    for (size_t i = 0; i < copy; ++i)
      ptr[len + i] = s[i];
    len += n;
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
  char align = '>';  // '<', '>', '^'
  char fill = ' ';
  char sign = '\0';  // '+', ' '
  uint16_t width = 0;
  int16_t prec = -1;  // -1 = not specified
  char type = '\0';   // 'x','f','e'
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


inline void write_decimal(BufferSink& s, int value)
{  // fast enough for demo
  char buf[12];
  char* p = buf + sizeof(buf);
  bool neg = value < 0;
  unsigned v = neg ? -static_cast<unsigned>(value) : value;
  do
  {
    *--p = char('0' + v % 10);
    v /= 10;
  } while (v);
  if (neg)
    *--p = '-';
  s.write(p, buf + sizeof(buf) - p);
}

template <class Sink>
void write_arg(Sink& sink, const char* str)
{
  sink.write(str, std::char_traits<char>::length(str));
}

template <class Sink>
void write_arg(Sink& sink, std::string_view sv)
{
  sink.write(sv.data(), sv.size());
}

template <class Sink>
std::enable_if_t<std::is_integral<int>::value, void>  // only int for brevity
write_arg(Sink& sink, int v)
{
  write_decimal(sink, v);
}

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
    default:
      pad(s, fs.fill, padlen);
      s.write(data.data(), data.size());
      break;
  }
}

inline char prefix_for(bool neg, const FormatSpec& fs)
{
  if (neg)
    return '-';
  if (fs.sign == '+')
    return '+';
  if (fs.sign == ' ')
    return ' ';
  return '\0';
}

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
  char buf[12];
  char* p = buf + sizeof(buf);
  bool neg = v < 0;
  unsigned val = neg ? -static_cast<unsigned>(v) : v;

  unsigned base = (fs.type == 'x') ? 16u : 10u;
  auto digit = [&](unsigned d)
  {
    return d < 10 ? '0' + d : 'a' + (d - 10);
  };

  do
  {
    *--p = digit(val % base);
    val /= base;
  } while (val);
  if (neg)
    *--p = '-';

  write_padded(out, {p, static_cast<size_t>(buf + sizeof(buf) - p)}, fs);
}

template <class Sink>
void write_arg(Sink& s, const char* str, const FormatSpec& fs)
{
  write_padded(s, {str, std::strlen(str)}, fs);
}
template <class Sink>
void write_arg(Sink& s, std::string_view sv, const FormatSpec& fs)
{
  write_padded(s, sv, fs);
}

template <size_t I = 0, class Sink, class Tup>
inline void write_by_index(Sink&, size_t, const FormatSpec&, Tup&)
{
}

template <size_t I = 0, class Sink, class Tup>
inline void write_by_idx(Sink&, size_t, const FormatSpec&, Tup&)
{
}
template <size_t I = 0, class Sink, class Tup, class T0, class... Rest>
inline void write_by_idx(Sink& s, size_t idx, const FormatSpec& fs, Tup& tup)
{
  if (idx == I)
    write_arg(s, std::get<I>(tup), fs);
  else
    write_by_idx<I + 1, Sink, Tup>(s, idx, fs, tup);
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
      FormatSpec fs = parse_spec(fmt.substr(start, i - start));
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
size_t format_to(char (&buf)[N], std::string_view fmt, Ts&&... ts)
{
  BufferSink sink{buf, N};
  vformat(sink, fmt, std::forward<Ts>(ts)...);
  if (sink.len < N)
    buf[sink.len] = '\0';  // NUL-terminate if space
  return sink.len;
}

template <class It, class... Ts>
It format_to(It it, std::string_view fmt, Ts&&... ts)
{
  IteratorSink<It> sink{it};
  vformat(sink, fmt, std::forward<Ts>(ts)...);
  return sink.it;
}

template <class... Ts>
void format_to(std::ostream& os, std::string_view fmt, Ts&&... ts)
{
  OStreamSink sink{os};
  vformat(sink, fmt, std::forward<Ts>(ts)...);
}

}  // namespace clarinoid