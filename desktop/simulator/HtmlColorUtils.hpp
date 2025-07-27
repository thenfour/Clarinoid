



// --- compile-time helpers ---
constexpr int
nibble(char c)
{
  return (c >= '0' && c <= '9')   ? (c - '0')
         : (c >= 'a' && c <= 'f') ? (c - 'a' + 10)
         : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                                  : -1;
}
constexpr unsigned
byte2(char a, char b)
{
  int x = nibble(a), y = nibble(b);
  return (x < 0 || y < 0) ? 256u : unsigned((x << 4) | y); // 256 == invalid
}

// Returns IM_COL32 from a compile-time char array (without leading '#')
template<std::size_t M>
constexpr ImU32
parse_nohash(const std::array<char, M>& s)
{
  static_assert(M == 3 || M == 4 || M == 6 || M == 8, "color: use #RGB[A] or #RRGGBB[AA]");
  if constexpr (M == 3 || M == 4) {
    const int R = nibble(s[0]), G = nibble(s[1]), B = nibble(s[2]);
    const int A = (M == 4) ? nibble(s[3]) : 15;
    static_assert(R >= 0 && G >= 0 && B >= 0 && A >= 0, "color: bad hex digit");
    return IM_COL32((R << 4) | R, (G << 4) | G, (B << 4) | B, (A << 4) | A);
  } else { // 6 or 8
    const unsigned R = byte2(s[0], s[1]);
    const unsigned G = byte2(s[2], s[3]);
    const unsigned B = byte2(s[4], s[5]);
    const unsigned A = (M == 8) ? byte2(s[6], s[7]) : 255u;
    static_assert(R < 256 && G < 256 && B < 256 && A < 256, "color: bad hex digit");
    return IM_COL32(R, G, B, A);
  }
}

// Strip leading '#' at compile time and dispatch to parse_nohash
template<char... Cs>
struct parsed_u32
{
  static constexpr std::array<char, sizeof...(Cs)> s{ Cs... };
  static constexpr bool has_hash = (s.size() > 0 && s[0] == '#');
  // Build an array without the leading '#'
  static constexpr auto nohash()
  {
    if constexpr (!has_hash)
      return s;
    else {
      std::array<char, s.size() - 1> t{};
      for (std::size_t i = 1; i < s.size(); ++i)
        t[i - 1] = s[i];
      return t;
    }
  }
  static constexpr auto nh = nohash();
  static constexpr ImU32 value = parse_nohash(nh);
};

template<char... Cs>
constexpr ImU32
operator"" _imu32()
{
  return parsed_u32<Cs...>::value;
}

constexpr int
hex1(char c)
{
  return (c >= '0' && c <= '9')   ? (c - '0')
         : (c >= 'a' && c <= 'f') ? (c - 'a' + 10)
         : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                                  : -1;
}
constexpr unsigned
hex2(char a, char b)
{
  int x = hex1(a), y = hex1(b);
  return (x < 0 || y < 0) ? 256u : unsigned((x << 4) | y); // 256 = invalid sentinel
}

constexpr ImU32
parse_html_hex_cstr(const char* s, std::size_t n)
{
  // strip leading '#'
  if (n && s[0] == '#') {
    ++s;
    --n;
  }

  unsigned r = 0, g = 0, b = 0, a = 255;

  if (n == 3 || n == 4) { // #RGB / #RGBA (nibbles)
    int R = hex1(s[0]), G = hex1(s[1]), B = hex1(s[2]);
    int A = (n == 4) ? hex1(s[3]) : 15;
    if ((R | G | B | A) < 0)
      return IM_COL32(255, 0, 255, 255); // magenta on error
    return IM_COL32((R << 4) | R, (G << 4) | G, (B << 4) | B, (A << 4) | A);
  } else if (n == 6 || n == 8) { // #RRGGBB / #RRGGBBAA (bytes)
    r = hex2(s[0], s[1]);
    g = hex2(s[2], s[3]);
    b = hex2(s[4], s[5]);
    a = (n == 8) ? hex2(s[6], s[7]) : 255u;
    if (r == 256u || g == 256u || b == 256u || a == 256u)
      return IM_COL32(255, 0, 255, 255);
    return IM_COL32(r, g, b, a);
  } else {
    return IM_COL32(255, 0, 255, 255); // unsupported length
  }
}

constexpr ImU32
operator"" _imu32(const char* s, std::size_t n)
{
  return parse_html_hex_cstr(s, n);
}
