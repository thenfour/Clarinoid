
#pragma once

// GuiLog.h
// Single-file drop-in. Requires imgui.h
#include <imgui.h>
#include <vector>
#include <deque>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstring>
#include <algorithm>

struct GuiLog
{
  // capacity_bytes is the maximum number of UTF-8 bytes kept (excluding the trailing '\0').
  explicit GuiLog(size_t capacity_bytes = 1 * 1024 * 1024) // 1 MiB
  {
    setCapacityBytes(capacity_bytes);
    mBuf[0] = '\0';
    mLen = 0;
    mLineOffsets.clear();
    mLineOffsets.push_back(0);
  }

  // Append a message (thread-safe). A '\n' is appended if not present.
  void append(const char* msg)
  {
    if (!msg)
      return;
    std::lock_guard<std::mutex> lock(mMutex);
    appendUnlocked(msg, std::strlen(msg));
    mNewDataSinceLastRender = true;
  }

  void append(const std::string& msg)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    appendUnlocked(msg.data(), msg.size());
    mNewDataSinceLastRender = true;
  }

  // printf-style append
  void appendf(const char* fmt, ...) IM_FMTARGS(2)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    char scratch[2048];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(scratch, sizeof(scratch), fmt, ap);
    va_end(ap);
    if (n <= 0)
      return;
    if ((size_t)n < sizeof(scratch)) {
      appendUnlocked(scratch, (size_t)n);
    } else {
      // fallback for very long lines
      std::string big;
      big.resize((size_t)n);
      va_start(ap, fmt);
      vsnprintf(big.data(), big.size() + 1, fmt, ap);
      va_end(ap);
      appendUnlocked(big.data(), big.size());
    }
    mNewDataSinceLastRender = true;
  }

  void clear()
  {
    std::lock_guard<std::mutex> lock(mMutex);
    mLen = 0;
    mBuf[0] = '\0';
    mLineOffsets.clear();
    mLineOffsets.push_back(0);
    mPending.clear();
    mNewDataSinceLastRender = false;
  }

  void setCapacityBytes(size_t bytes)
  {
    if (bytes < 4096)
      bytes = 4096;
    std::lock_guard<std::mutex> lock(mMutex);
    mCapacity = bytes;
    // +1 for trailing '\0', keep pointer stable by reserving once.
    mBuf.assign(mCapacity + 1, 0);
    mLen = 0;
    mLineOffsets.clear();
    mLineOffsets.push_back(0);
    mPending.clear();
    mNewDataSinceLastRender = false;
  }

  size_t capacityBytes() const { return mCapacity; }
  size_t sizeBytes() const { return mLen; }
  size_t lineCount() const { return mLineOffsets.size() - 1 + (mLen > 0 && mBuf[mLen - 1] != '\n' ? 1 : 0); }

  // UI flags (customize from outside if you want)
  bool& pause() { return mPaused; }
  bool& followTail() { return mFollowTail; }
  bool& wordWrap() { return mWordWrap; }

  // Render the widget. Call from the main thread.
  void render(const char* id = "Log")
  {
    ImGui::PushID(id);

    // ------ Toolbar ------
    if (ImGui::BeginChild("##toolbar", ImVec2(0, ImGui::GetFrameHeightWithSpacing() + 200), ImGuiChildFlags_Borders, ImGuiWindowFlags_MenuBar)) {

        {
        ImGui::BeginMenuBar();
          if (ImGui::Button("Clear")) {
            clear();
            mSelected.clear();
            mSelectAnchor = -1;
          }
          ImGui::SameLine();
          ImGui::Checkbox("Pause", &mPaused);
          ImGui::SameLine();
          ImGui::Checkbox("Word-wrap", &mWordWrap);
          ImGui::SameLine();
          // if (ImGui::Button("Copy selected"))
          //   copySelectedToClipboard_();
          ImGui::SameLine();
          ImGui::TextDisabled("  lines:%zu  size:%zu/%zu KB", lineCount(), sizeBytes() / 1024, capacityBytes() / 1024);

          ImGui::EndMenuBar();
        }

    }
    //ImGui::EndChild();

    //ImGui::Separator();

    // ------ List area ------
    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysVerticalScrollbar;
    if (!mWordWrap)
      flags |= ImGuiWindowFlags_HorizontalScrollbar;

    //ImGui::BeginChild("##log_list", ImVec2(0, 0), true, flags);

    // Lock while reading buffer/offsets
    std::lock_guard<std::mutex> lock(mMutex);

    const int lines = (int)lineCount();

    // Maintain selection array size. If trimming happened, safest is to clear selection.
    if ((int)mSelected.size() != lines) {
      mSelected.assign((size_t)lines, 0); // clear on resize
      mSelectAnchor = -1;
    }

    if (mWordWrap)
      ImGui::PushTextWrapPos(0.0f);

    ImGuiListClipper clipper;
    clipper.Begin(lines);

    // Keyboard shortcuts (when the child is hovered)
    const ImGuiIO& io = ImGui::GetIO();
    const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootAndChildWindows);
    if (hovered && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
      // Ctrl+A -> select all
      for (int i = 0; i < lines; ++i)
        mSelected[i] = 1;
      if (lines)
        mSelectAnchor = 0;
    }
    //if (hovered && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
    //  copySelectedToClipboard_();
    //}

    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
        const char* b;
        const char* e;
        if (!getLineRange(i, b, e))
          continue;

        // Compute wrapped height so the highlight covers wrapped text
        float wrap_width = mWordWrap ? ImGui::GetContentRegionAvail().x : -1.0f;
        ImVec2 text_size = ImGui::CalcTextSize(b, e, false, wrap_width);
        float row_h = std::max(text_size.y, ImGui::GetTextLineHeight()); // at least one line

        ImGui::PushID(i);
        bool selected = mSelected[i] != 0;
        ImGuiSelectableFlags sel_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
        if (ImGui::Selectable("##sel", selected, sel_flags, ImVec2(0, row_h))) {
          handleClickSelection_(i, io.KeyShift, io.KeyCtrl);
        }
        // Draw text next to the selection hit rect
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextUnformatted(b, e);
        ImGui::PopID();
      }
    }
    clipper.End();

    if (mWordWrap)
      ImGui::PopTextWrapPos();

    // Auto-scroll (no focus stealing)
    if (!mPaused) {
      ImGui::SetScrollY(ImGui::GetScrollMaxY());
    }

    ImGui::EndChild();
    ImGui::PopID();
  }


private:
  // -------- Storage --------
  std::vector<char> mBuf; // fixed-capacity byte buffer (+1 for trailing '\0')
  size_t mCapacity = 0;
  size_t mLen = 0;                 // bytes in use (excludes trailing '\0')
  std::deque<size_t> mLineOffsets; // start offset of each line (0-based)

  // Concurrency
  mutable std::mutex mMutex;
  std::vector<std::string> mPending; // pending lines while paused

  // UI state
  bool mPaused = false;
  bool mWordWrap = true;
  bool mFollowTail = true;                  // user toggle
  //bool mUserForcedNotFollowing = false;     // becomes true when user scrolls away
  bool mRequestScrollToEnd = false;         // do in callback when focused/active
  bool mRequestFocusAndScrollToEnd = false; // set by "Bottom"
  bool mHadFocusLastFrame = false;
  bool mNewDataSinceLastRender = false;

  int mSelectAnchor = -1;      // for Shift-click range
  std::vector<char> mSelected; // per-line selection flags; cleared on trim
  //bool mPaused = false;
  //bool mWordWrap = true;


  // -------- Helpers --------
  void flushPending()
  {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mPending.empty())
      return;
    for (const auto& s : mPending)
      appendUnlocked(s.data(), s.size()); // unlocked variant but we hold the lock here
    mPending.clear();
    mNewDataSinceLastRender = true;
  }

  bool getLineRange(int i, const char*& begin, const char*& end) const
  {
    if (i < 0)
      return false;
    const int n = (int)mLineOffsets.size();
    if (n == 0)
      return false;
    if (i >= (int)lineCount())
      return false;
    size_t b = mLineOffsets[i];
    size_t e = (i + 1 < n) ? mLineOffsets[i + 1] - 1 /*drop '\n'*/ : mLen;
    begin = mBuf.data() + b;
    end = mBuf.data() + e;
    return true;
  }

  void handleClickSelection_(int idx, bool shift, bool ctrl)
  {
    const int lines = (int)mSelected.size();
    if (idx < 0 || idx >= lines)
      return;

    if (shift && mSelectAnchor >= 0) {
      int a = std::min(mSelectAnchor, idx);
      int b = std::max(mSelectAnchor, idx);
      for (int i = 0; i < lines; ++i)
        mSelected[i] = (i >= a && i <= b);
    } else if (ctrl) {
      mSelected[idx] ^= 1; // toggle
      if (mSelectAnchor < 0)
        mSelectAnchor = idx;
    } else {
      // single select
      for (int i = 0; i < lines; ++i)
        mSelected[i] = (i == idx);
      mSelectAnchor = idx;
    }
  }

  //void copySelectedToClipboard_()
  //{
  //  std::lock_guard<std::mutex> lock(mMutex);
  //  std::string out;
  //  const int lines = (int)mSelected.size();
  //  for (int i = 0; i < lines; ++i) {
  //    if (!mSelected[i])
  //      continue;
  //    const char* b;
  //    const char* e;
  //    if (!getLineRange(i, b, e))
  //      continue;
  //    out.append(b, e);
  //    out.push_back('\n');
  //  }
  //  if (!out.empty())
  //    ImGui::SetClipboardText(out.c_str());
  //}

  void appendUnlocked(const char* data, size_t len)
  {
    if (mPaused) {
      mPending.emplace_back(data, len);
      return;
    }

    // Normalize line endings to '\n' and ensure newline at end
    const bool has_trailing_nl = (len > 0 && data[len - 1] == '\n');
    size_t needed = len + (has_trailing_nl ? 0 : 1);

    // Make room (trim from front on line boundaries)
    ensureFreeUnlocked(needed);

    // Copy data, tracking line offsets
    for (size_t i = 0; i < len; ++i) {
      char c = data[i];
      if (c == '\r')
        continue; // drop CR
      mBuf[mLen++] = c;
      if (c == '\n')
        mLineOffsets.push_back(mLen);
    }
    if (!has_trailing_nl) {
      mBuf[mLen++] = '\n';
      mLineOffsets.push_back(mLen);
    }
    mBuf[mLen] = '\0';

    // If user was following before (caret at end last frame), keep following.
    if (mFollowTail)// && !mUserForcedNotFollowing)
      mRequestScrollToEnd = true;
  }

  void ensureFreeUnlocked(size_t bytes_needed)
  {
    if (bytes_needed <= (mCapacity - mLen))
      return;

    // Target: keep at least bytes_needed free.
    size_t want_free = bytes_needed;
    size_t need_to_drop = mLen + want_free > mCapacity ? (mLen + want_free - mCapacity) : 0;
    if (need_to_drop == 0)
      return;

    // Drop whole lines from the front until enough space is available
    size_t cut = 0;
    while (!mLineOffsets.empty() && (mLen - cut + want_free > mCapacity)) {
      if (mLineOffsets.size() >= 2) {
        size_t next = mLineOffsets[1]; // start of 2nd line => remove the first line [0, next)
        cut = next;
        mLineOffsets.pop_front();
      } else {
        // Single very long line or no newline yet -> cut chunk
        cut = std::min(mLen, need_to_drop);
        break;
      }
    }

    if (cut > 0) {
      // Shift remaining bytes left
      const size_t remain = mLen - cut;
      std::memmove(mBuf.data(), mBuf.data() + cut, remain);
      mLen = remain;
      mBuf[mLen] = '\0';

      // Adjust offsets
      for (size_t& off : mLineOffsets)
        off -= cut;
      if (mLineOffsets.empty() || mLineOffsets.front() != 0)
        mLineOffsets.push_front(0);

      // User is no longer at the exact visual tail; keep follow-tail unless they scrolled.
      // (Nothing to change here.)
    }
  }

  // Called every frame while the text box is active (due to CallbackAlways).
  static int EditCallback(ImGuiInputTextCallbackData* data)
  {
    auto* self = static_cast<GuiLog*>(data->UserData);

    // If the caret moved off the end or there's a selection, stop following.
    //const bool caret_at_end = (data->CursorPos == data->BufTextLen) && (data->SelectionStart == data->SelectionEnd);
    //if (!caret_at_end && (ImGui::IsItemActive() || ImGui::IsItemFocused()))
    //  self->mUserForcedNotFollowing = true;

    // If we should follow the tail, move caret to end to make ImGui scroll the internal view.
    if (self->mFollowTail && /* !self->mUserForcedNotFollowing && */
        (self->mRequestScrollToEnd || self->mRequestFocusAndScrollToEnd)) {
      data->CursorPos = data->BufTextLen;
      data->SelectionStart = data->BufTextLen;
      data->SelectionEnd = data->BufTextLen;
      // After we scrolled once, clear requests.
      self->mRequestScrollToEnd = false;
      self->mRequestFocusAndScrollToEnd = false;
    }

    return 0;
  }
};
