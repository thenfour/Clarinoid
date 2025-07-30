#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype>
#include <cstdlib> // strtod
#include <cstring> // memcpy
#include <utility> // pair
#include <algorithm>

struct TelemetryItem
{
  std::string name;
  float value;
};

struct TelemetryMessage
{
  std::vector<TelemetryItem> items; // in the order they appeared
  bool empty() const noexcept { return items.empty(); }
  void clear() { items.clear(); }
};

struct TelemetryParseOptions
{
  const char* auto_name_prefix = "ch"; // unlabeled tokens → ch0, ch1, ...
  int auto_name_start = 0;             // first index for unlabeled tokens
  bool accept_comments = true;         // stop on '#' or '//' if true
  bool require_alpha_in_label = true;  // left side must contain [A-Za-z_] to be a label
  bool force_lowercase_names = false;  // normalize names to lowercase
};

// Forward decl
inline bool
ParseTelemetryMessage(const char* begin,
                      const char* end,
                      TelemetryMessage& out,
                      const TelemetryParseOptions& opts = {});

// Convenience overload
inline bool
ParseTelemetryMessage(const std::string& line, TelemetryMessage& out, const TelemetryParseOptions& opts = {})
{
  return ParseTelemetryMessage(line.data(), line.data() + line.size(), out, opts);
}

// --- Implementation ---
inline bool
ParseTelemetryMessage(const char* begin, const char* end, TelemetryMessage& out, const TelemetryParseOptions& opts)
{
  out.clear();
  if (!begin || !end || begin >= end)
    return false;

  auto is_space = [](unsigned char c) -> bool { return std::isspace(c) != 0; };
  auto is_delim_or_space = [&](unsigned char c) -> bool { return is_space(c) || c == ',' || c == ';'; };
  auto trim_span = [&](const char*& a, const char*& b) {
    while (a < b && is_space((unsigned char)*a))
      ++a;
    while (b > a && is_space((unsigned char)b[-1]))
      --b;
  };
  auto has_alpha_or_underscore = [](const char* a, const char* b) {
    for (const char* p = a; p < b; ++p)
      if (std::isalpha((unsigned char)*p) || *p == '_')
        return true;
    return false;
  };
  auto to_lower_inplace = [](std::string& s) {
    for (char& c : s)
      c = (char)std::tolower((unsigned char)c);
  };
  auto parse_float = [](const char* a, const char* b, float& out_f) -> bool {
    char buf[64];
    const size_t len = (size_t)(b - a);
    if (len == 0)
      return false;
    if (len < sizeof(buf)) {
      std::memcpy(buf, a, len);
      buf[len] = '\0';
      char* endp = nullptr;
      double v = std::strtod(buf, &endp);
      if (endp == buf)
        return false;
      out_f = (float)v;
      return true;
    } else {
      std::string tmp(a, b);
      char* endp = nullptr;
      double v = std::strtod(tmp.c_str(), &endp);
      if (endp == tmp.c_str())
        return false;
      out_f = (float)v;
      return true;
    }
  };

  // Ensure unique names within this line (keeps first occurrence’s casing)
  std::unordered_set<std::string> used_names;
  auto make_unique = [&](std::string base) {
    if (opts.force_lowercase_names)
      to_lower_inplace(base);
    std::string name = base;
    int n = 0;
    while (used_names.find(name) != used_names.end()) {
      name = base + "_" + std::to_string(++n);
    }
    used_names.insert(name);
    return name;
  };

  const char* p = begin;
  while (p < end && is_delim_or_space((unsigned char)*p))
    ++p;

  int auto_idx = opts.auto_name_start;

  while (p < end) {
    // Comments
    if (opts.accept_comments) {
      if (*p == '#')
        break;
      if (*p == '/' && (p + 1) < end && p[1] == '/')
        break;
    }

    // Token start
    const char* t0 = p;
    // Advance to token end
    while (p < end && !is_delim_or_space((unsigned char)*p)) {
      if (opts.accept_comments && *p == '#')
        break;
      if (opts.accept_comments && *p == '/' && (p + 1) < end && p[1] == '/')
        break;
      ++p;
    }
    const char* t1 = p;
    trim_span(t0, t1);

    if (t0 < t1) {
      // Find first ':' or '=' inside token
      const char* sep = nullptr;
      for (const char* q = t0; q < t1; ++q) {
        if (*q == ':' || *q == '=') {
          sep = q;
          break;
        }
      }

      bool pushed = false;
      if (sep) {
        const char* n0 = t0;
        const char* n1 = sep;
        const char* v0 = sep + 1;
        const char* v1 = t1;
        trim_span(n0, n1);
        trim_span(v0, v1);

        bool treat_as_label = true;
        if (opts.require_alpha_in_label && !has_alpha_or_underscore(n0, n1))
          treat_as_label = false;

        if (treat_as_label) {
          float fv;
          if (parse_float(v0, v1, fv)) {
            std::string name(n0, n1);
            if (opts.force_lowercase_names)
              to_lower_inplace(name);
            // ensure uniqueness
            if (used_names.insert(name).second) {
              out.items.push_back({ std::move(name), fv });
              pushed = true;
            } else {
              // duplicate label in same line: disambiguate
              out.items.push_back({ make_unique(std::string(n0, n1)), fv });
              pushed = true;
            }
          }
        }
      }
      if (!pushed) {
        // Plain numeric
        float fv;
        if (parse_float(t0, t1, fv)) {
          std::string auto_name = std::string(opts.auto_name_prefix) + std::to_string(auto_idx++);
          auto_name = make_unique(std::move(auto_name));
          out.items.push_back({ std::move(auto_name), fv });
        }
      }
    }

    // Move past delimiters/spaces
    while (p < end && is_delim_or_space((unsigned char)*p))
      ++p;
    // If we hit a comment marker as a delimiter, stop
    if (opts.accept_comments) {
      if (p < end && *p == '#')
        break;
      if (p < end && *p == '/' && (p + 1) < end && p[1] == '/')
        break;
    }
  }

  return !out.items.empty();
}
