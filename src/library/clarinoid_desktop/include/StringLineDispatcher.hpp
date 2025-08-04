
#pragma once

#include <string>
#include <functional>
#include <cstddef>

struct StringLineDispatcher
{
  // Called once per completed line (without the trailing newline).
  // The line has all '\r' characters removed.
  std::function<void(const std::string&)> mLineCallback;

  // Feed a chunk (may contain 0..N lines; may end on a partial line).
  void HandleIncomingString(const std::string& chunk)
  {
    if (chunk.empty())
      return;
    mBuf.append(chunk);

    size_t start = 0;
    for (;;) {
      // Find next newline from 'start'
      size_t nl = mBuf.find('\n', start);
      if (nl == std::string::npos) {
        // Keep unprocessed tail [start, end) for next time
        if (start > 0)
          mBuf.erase(0, start);
        break;
      }

      // Build line = mBuf[start..nl), stripping '\r'
      std::string line;
      line.reserve(nl - start);
      for (size_t i = start; i < nl; ++i) {
        char c = mBuf[i];
        if (c != '\r')
          line.push_back(c);
      }

      if (mLineCallback)
        mLineCallback(line);

      start = nl + 1; // continue after '\n'
    }

    // Optionally guard against unbounded growth if no newline ever arrives
    if (mMaxPendingBytes && mBuf.size() > mMaxPendingBytes) {
      // Strategy: either drop oldest, or flush as a line. Here we drop oldest.
      size_t drop = mBuf.size() - mMaxPendingBytes;
      mBuf.erase(0, drop);
    }
  }

  // Feed raw data without constructing a std::string
  void HandleIncomingData(const char* data, size_t len)
  {
    if (!data || len == 0)
      return;
    mBuf.append(data, len);
    HandleIncomingString(std::string{}); // reuse logic without double copy
                                         // Note: the above would create an empty string; instead, factor the loop:
                                         // For zero-copy, move the loop into a private method and call it here.
  }

  // Flush the pending tail (e.g., on stream close). If dispatchPartial=false,
  // just discard the buffer.
  void Flush(bool dispatchPartial = true)
  {
    if (mBuf.empty())
      return;
    if (dispatchPartial && mLineCallback) {
      std::string line;
      line.reserve(mBuf.size());
      for (char c : mBuf)
        if (c != '\r')
          line.push_back(c);
      mLineCallback(line);
    }
    mBuf.clear();
  }

  // Clear pending data without dispatching
  void Clear() { mBuf.clear(); }

  // Optional: cap pending buffer to avoid unbounded growth if no '\n' arrives.
  // 0 = unlimited.
  void SetMaxPendingBytes(size_t n) { mMaxPendingBytes = n; }

private:
  std::string mBuf; // accumulates incomplete tail (may contain '\r')
  size_t mMaxPendingBytes = 0;
};
