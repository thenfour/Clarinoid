#pragma once
#include <vector>
#include <mutex>
#include <limits>
#include <cmath>
#include "SeriesCollection.hpp"

class TimeSeriesStore
{
public:
  using Id = SeriesCollection::Id;

  // One series' view into the snapshot buffer.
  struct SeriesSnapshot
  {
    Id id;             // series id (from SeriesCollection)
    const float* data; // pointer into Snapshot::buffer (contiguous, read-only)
    size_t count;      // number of samples in this series window (same for all series in the snapshot)
    size_t stride;     // always 1 for now (kept for future-proofing)
  };

  // The whole snapshot returned to a view.
  struct Snapshot
  {
    std::vector<SeriesSnapshot> series; // descriptors per requested id
    std::vector<float> buffer;          // owns the contiguous copies (keeps data pointers valid)
    size_t count = 0;                   // window length (samples). 0 means “nothing to draw”
  };

  explicit TimeSeriesStore(size_t history = 8000)
    : mHistory(history)
  {
  }

  // Ensure a ring exists for this id.
  void ensure(Id id)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    ensure_nolock_(id);
  }

  // Always write NaN for series not present this step (so gaps are real gaps).
  void appendAligned(const std::vector<std::pair<Id, float>>& vals)
  {
    std::lock_guard<std::mutex> lock(mMutex);

    // ensure rings for referenced ids
    for (auto& kv : vals)
      ensure_nolock_(kv.first);

    const size_t wi = mHeadGlobal % mHistory;

    // Write NaN for all existing rings first (O(M))
    for (auto& r : mRings) {
      if (!r.data.empty())
        r.data[wi] = NaN();
    }
    // Overwrite values we *do* have
    for (auto& kv : vals) {
      Ring& r = mRings[kv.first];
      r.data[wi] = kv.second;
    }

    // Advance heads/counts uniformly
    for (auto& r : mRings) {
      if (r.data.empty())
        continue;
      r.head = (wi + 1) % mHistory;
      if (r.count < mHistory)
        r.count++;
    }
    mHeadGlobal++;
  }

  // Logical head sequence number (monotonic, grows each append)
  //size_t headSeq() const
  //{
  //  std::lock_guard<std::mutex> lock(mMutex);
  //  return mHeadGlobal;
  //}

  // Global clear (affects all views)
  void clearAll()
  {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto& r : mRings) {
      if (r.data.empty())
        continue;
      std::fill(r.data.begin(), r.data.end(), NaN());
      r.head = (mHeadGlobal % mHistory);
      r.count = 0;
    }
  }

  // Copy the last 'window' samples (<= history) for the requested ids into a contiguous snapshot.
  Snapshot snapshotLast(const std::vector<Id>& ids, size_t window = 0) const
  {
    Snapshot out;
    std::lock_guard<std::mutex> lock(mMutex);

    if (mRings.empty())
      return out;

    if (window == 0)
      window = mHistory;
    window = std::min(window, mHistory);
    // Window length is governed by the *shortest* ring count among requested series
    size_t avail = window;
    for (Id id : ids) {
      if ((size_t)id >= mRings.size() || mRings[id].data.empty()) {
        avail = 0;
        break;
      }
      avail = std::min(avail, mRings[id].count);
    }
    if (avail == 0)
      return out;

    out.count = avail;
    // Reserve buffer big enough for all requested series
    out.buffer.reserve(ids.size() * avail);

    for (Id id : ids) {
      const Ring& r = mRings[id];
      const size_t head = r.head; // next write index
      const size_t oldest = (head + mHistory - avail) % mHistory;

      // Copy to the tail of out.buffer and remember pointer
      size_t base = out.buffer.size();
      out.buffer.resize(base + avail);
      for (size_t i = 0, idx = oldest; i < avail; ++i, idx = (idx + 1) % mHistory)
        out.buffer[base + i] = r.data[idx];

      out.series.push_back({ id, out.buffer.data() + base, avail, 1 });
    }

    return out;
  }

private:
  static float NaN() { return std::numeric_limits<float>::quiet_NaN(); }

  void ensure_nolock_(Id id)
  {
    if ((size_t)id >= mRings.size())
      mRings.resize((size_t)id + 1);
    Ring& r = mRings[id];
    if (r.data.size() != mHistory) {
      r.data.assign(mHistory, NaN());
      r.head = (mHeadGlobal % mHistory);
      r.count = 0;
    }
  }

  struct Ring
  {
    std::vector<float> data;
    size_t head = 0;
    size_t count = 0;
  };

  size_t mHistory;
  mutable std::mutex mMutex;
  std::vector<Ring> mRings;
  size_t mHeadGlobal = 0; // aligned head for “time steps”
};
