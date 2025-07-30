#pragma once

#include <windows.h>
#include <string>
#include <iterator>
#include <system_error>

static float
lerp(float v0, float v1, float t)
{
  return v0 + t * (v1 - v0);
}
static float
clamp(float v, float v0, float v1)
{
  return std::max(v0, std::min(v1, v));
}

constexpr int
parse_hex_digit(char c)
{
  return (c >= '0' && c <= '9')   ? (c - '0')
         : (c >= 'a' && c <= 'f') ? (c - 'a' + 10)
         : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                                  : -1;
}

constexpr unsigned
parse_hex_byte(char a, char b)
{
  int x = parse_hex_digit(a), y = parse_hex_digit(b);
  return (x < 0 || y < 0) ? 256u : unsigned((x << 4) | y); // 256 = invalid sentinel
}

inline std::string
wstring_to_bytes(const std::wstring& w, UINT code_page = CP_ACP)
{
  if (w.empty())
    return {};

  // First: size
  int needed = ::WideCharToMultiByte(code_page, 0, w.data(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
  if (needed == 0)
    throw std::system_error(::GetLastError(), std::system_category(), "WideCharToMultiByte(size)");

  std::string out(static_cast<size_t>(needed), '\0');

  // Second: convert
  int written =
    ::WideCharToMultiByte(code_page, 0, w.data(), static_cast<int>(w.size()), out.data(), needed, nullptr, nullptr);
  if (written == 0)
    throw std::system_error(::GetLastError(), std::system_category(), "WideCharToMultiByte(data)");

  return out;
}

inline std::wstring
bytes_to_wstring(const std::string& s, UINT code_page = CP_ACP)
{
  if (s.empty())
    return {};

  // First: size
  int needed = ::MultiByteToWideChar(code_page, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
  if (needed == 0)
    throw std::system_error(::GetLastError(), std::system_category(), "MultiByteToWideChar(size)");

  std::wstring out(static_cast<size_t>(needed), L'\0');

  // Second: convert
  int written = ::MultiByteToWideChar(code_page, 0, s.data(), static_cast<int>(s.size()), out.data(), needed);
  if (written == 0)
    throw std::system_error(::GetLastError(), std::system_category(), "MultiByteToWideChar(data)");

  return out;
}

std::string
removeAll(const std::string& s, char ch)
{
  std::string result;
  std::copy_if(s.begin(), s.end(), std::back_inserter(result), [ch](char c) { return c != ch; });
  return result;
}

inline std::vector<std::string>
splitIntoLines(const std::string& s)
{
  std::vector<std::string> lines;
  size_t start = 0;
  while (start < s.size()) {
    size_t end = s.find('\n', start);
    if (end == std::string::npos) {
      std::string line = s.substr(start);
      lines.push_back(removeAll(line, '\r'));
      break;
    } else {
      std::string line = s.substr(start, end - start);
      lines.push_back(removeAll(line, '\r'));
      start = end + 1; // skip '\n'
    }
  }
  return lines;
}