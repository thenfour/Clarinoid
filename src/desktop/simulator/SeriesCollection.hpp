#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <cmath>

// ---------------- Color scheme ----------------
struct SeriesColorScheme
{
  ImU32 bg = IM_COL32(0, 0, 0, 255);      // recommended plot bg (optional)
  ImU32 grid = IM_COL32(64, 64, 64, 255); // grid line color (optional)
  float targetLuma = 0.30f;               // relative luminance [0..1] to normalize to
  bool equalLuma = true;                  // apply luminance normalization to lines
  bool nameDeterministic = false;         // if true, fallback colors hash from name (stable across runs)
  float fallbackS = 0.90f;                // HSV saturation for fallback
  float fallbackV = 1.00f;                // HSV value/brightness for fallback
  std::vector<ImU32> palette;             // curated colors for first N series

  static SeriesColorScheme MakeDarkEqualLuma(float target = 0.30f);
};

// Utility: sRGB <-> linear conversions and luma normalization
namespace detail_sc {
inline float
srgb_to_linear(float c)
{
  return (c <= 0.04045f) ? (c / 12.92f) : std::pow((c + 0.055f) / 1.055f, 2.4f);
}
inline float
linear_to_srgb(float c)
{
  return (c <= 0.0031308f) ? (12.92f * c) : (1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f);
}
inline ImU32
adjust_to_luma(ImU32 col, float targetY)
{
  float rs = ((col >> IM_COL32_R_SHIFT) & 255) / 255.0f;
  float gs = ((col >> IM_COL32_G_SHIFT) & 255) / 255.0f;
  float bs = ((col >> IM_COL32_B_SHIFT) & 255) / 255.0f;
  float rl = srgb_to_linear(rs), gl = srgb_to_linear(gs), bl = srgb_to_linear(bs);
  float Yo = 0.2126f * rl + 0.7152f * gl + 0.0722f * bl;
  targetY = clamp(targetY, 0.0f, 1.0f);
  if (Yo < targetY) {
    float t = (targetY - Yo) / (1.0f - Yo + 1e-6f);
    rl = rl * (1 - t) + t;
    gl = gl * (1 - t) + t;
    bl = bl * (1 - t) + t;
  } else if (Yo > targetY) {
    float t = 1.0f - targetY / (Yo + 1e-6f);
    rl = rl * (1 - t);
    gl = gl * (1 - t);
    bl = bl * (1 - t);
  }
  int R = (int)std::round(clamp(linear_to_srgb(rl), 0.0f, 1.0f) * 255.0f);
  int G = (int)std::round(clamp(linear_to_srgb(gl), 0.0f, 1.0f) * 255.0f);
  int B = (int)std::round(clamp(linear_to_srgb(bl), 0.0f, 1.0f) * 255.0f);
  return IM_COL32(R, G, B, 255);
}
inline ImU32
hsv_fallback(int i, const SeriesColorScheme& s, const std::string* name)
{
  // Palette fallback: either golden-angle by index, or hashed hue from name.
  float h = 0.0f;
  if (s.nameDeterministic && name) {
    // 32-bit FNV-1a hash mapped to [0,1)
    uint32_t hash = 2166136261u;
    for (unsigned char c : *name) {
      hash ^= c;
      hash *= 16777619u;
    }
    h = (hash / 4294967296.0f); // [0,1)
  } else {
    h = std::fmod(0.13f + 0.61803398875f * i, 1.0f);
  }
  ImVec4 c = ImColor::HSV(h, s.fallbackS, s.fallbackV);
  ImU32 u = ImGui::ColorConvertFloat4ToU32(c);
  return s.equalLuma ? adjust_to_luma(u, s.targetLuma) : u;
}
} // namespace detail_sc

inline SeriesColorScheme
SeriesColorScheme::MakeDarkEqualLuma(float target)
{
  SeriesColorScheme s;
  s.bg = IM_COL32(0, 0, 0, 255);
  s.grid = IM_COL32(64, 64, 64, 255);
  s.targetLuma = target;
  s.equalLuma = true;
  // Okabe–Ito + Tol "bright"
  const ImU32 base[] = {
    IM_COL32(230, 159, 0, 255),   IM_COL32(86, 180, 233, 255),  IM_COL32(0, 158, 115, 255),
    IM_COL32(240, 228, 66, 255),  IM_COL32(0, 114, 178, 255),   IM_COL32(213, 94, 0, 255),
    IM_COL32(204, 121, 167, 255), IM_COL32(68, 119, 170, 255),  IM_COL32(238, 102, 119, 255),
    IM_COL32(34, 136, 51, 255),   IM_COL32(204, 187, 68, 255),  IM_COL32(102, 204, 238, 255),
    IM_COL32(170, 51, 119, 255),  IM_COL32(187, 187, 187, 255),
  };
  s.palette.reserve(sizeof(base) / sizeof(base[0]));
  for (ImU32 c : base)
    s.palette.push_back(s.equalLuma ? detail_sc::adjust_to_luma(c, s.targetLuma) : c);
  return s;
}

// ---------------- Series collection ----------------
class SeriesCollection
{
public:
  using Id = int;

  struct SeriesInfo
  {
    std::string name;
    ImU32 color = IM_COL32_WHITE;
    bool visible = true;
    bool colorLocked = false; // true if user overrode the color
    int order = 0;            // discovery order (for stable ordering)
  };

  struct Snapshot
  {
    struct Item
    {
      Id id;
      std::string name;
      ImU32 color;
      bool visible;
      int order;
    };
    std::vector<Item> items;
  };

  explicit SeriesCollection(SeriesColorScheme scheme = SeriesColorScheme::MakeDarkEqualLuma())
    : mScheme(std::move(scheme))
  {
  }

  ~SeriesCollection() { std::lock_guard<std::mutex> lock(mMutex); }

  // Lookup or create; assigns color if new
  Id getOrCreate(const std::string& name)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mNameToId.find(name);
    if (it != mNameToId.end())
      return it->second;
    Id id = (Id)mInfos.size();
    SeriesInfo inf;
    inf.name = name;
    inf.order = id;
    inf.color = assignColorForIndex_(id, &name);
    mInfos.push_back(inf);
    mNameToId.emplace(inf.name, id);
    return id;
  }

  // Returns -1 if not found
  Id find(const std::string& name) const
  {
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mNameToId.find(name);
    return (it == mNameToId.end()) ? -1 : it->second;
  }

  // Visibility/color overrides
  void setVisible(Id id, bool v)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    if (valid_(id))
      mInfos[id].visible = v;
  }
  bool isVisible(Id id) const
  {
    std::lock_guard<std::mutex> lock(mMutex);
    return valid_(id) ? mInfos[id].visible : false;
  }

  //void setColor(Id id, ImU32 c, bool lock = true)
  //{
  //  std::lock_guard<std::mutex> lock(mMutex);
  //  if (!valid_(id))
  //    return;
  //  mInfos[id].color = c;
  //  mInfos[id].colorLocked = lock;
  //}
  ImU32 color(Id id) const
  {
    std::lock_guard<std::mutex> lock(mMutex);
    return valid_(id) ? mInfos[id].color : IM_COL32_WHITE;
  }
  const std::string& name(Id id) const
  {
    std::lock_guard<std::mutex> lock(mMutex);
    return mInfos[id].name;
  }

  // Rename (keeps id/color). Returns false if newName already exists.
  bool rename(Id id, const std::string& newName)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    if (!valid_(id))
      return false;
    if (mNameToId.count(newName))
      return false;
    mNameToId.erase(mInfos[id].name);
    mInfos[id].name = newName;
    mNameToId.emplace(newName, id);
    return true;
  }

  // Scheme management
  void setScheme(const SeriesColorScheme& s)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    mScheme = s;
    // Reassign colors for non-locked series
    for (size_t i = 0; i < mInfos.size(); ++i) {
      if (!mInfos[i].colorLocked) {
        mInfos[i].color = assignColorForIndex_((int)i, &mInfos[i].name);
      }
    }
  }
  const SeriesColorScheme& scheme() const { return mScheme; }

  // Stable order snapshot for UI
  Snapshot snapshot(bool onlyVisible = false) const
  {
    std::lock_guard<std::mutex> lock(mMutex);
    Snapshot s;
    s.items.reserve(mInfos.size());
    for (Id id = 0; id < (Id)mInfos.size(); ++id) {
      const auto& inf = mInfos[id];
      if (onlyVisible && !inf.visible)
        continue;
      s.items.push_back({ id, inf.name, inf.color, inf.visible, inf.order });
    }
    std::sort(s.items.begin(), s.items.end(), [](const Snapshot::Item& a, const Snapshot::Item& b) {
      if (a.order != b.order)
        return a.order < b.order;
      return a.name < b.name;
    });
    return s;
  }

  // Reset all (keeps scheme)
  void clear()
  {
    std::lock_guard<std::mutex> lock(mMutex);
    mNameToId.clear();
    mInfos.clear();
  }

private:
  bool valid_(Id id) const { return id >= 0 && (size_t)id < mInfos.size(); }

  ImU32 assignColorForIndex_(int i, const std::string* name) const
  {
    if (i < (int)mScheme.palette.size())
      return mScheme.palette[i];
    return detail_sc::hsv_fallback(i, mScheme, mScheme.nameDeterministic ? name : nullptr);
  }

  mutable std::mutex mMutex;
  SeriesColorScheme mScheme;
  std::vector<SeriesInfo> mInfos;
  std::unordered_map<std::string, Id> mNameToId;
};
