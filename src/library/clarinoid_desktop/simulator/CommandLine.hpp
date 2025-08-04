// CommandLine.hpp
#pragma once
#include <imgui.h>
#include <vector>
#include <string>
#include <functional>
#include <cstring>
#include <cctype>
#include <algorithm>

struct CommandLine
{
  // --- public API ---------------------------------------------------------
  using ExecuteFn = std::function<void(const char* cmd)>;                                    // called on Enter
  using CompleteFn = std::function<void(const char* prefix, std::vector<std::string>& out)>; // Tab-completion

  explicit CommandLine(int buf_size = 1024, size_t max_history = 128)
    : mBufSize(buf_size)
    , mMaxHistory(max_history)
  {
    mBuf.resize((size_t)mBufSize);
    clearInput_();
  }

  void setExecuteCallback(ExecuteFn fn) { onExecute = std::move(fn); }
  void setCompleteCallback(CompleteFn fn) { onComplete = std::move(fn); }
  void setPrompt(const char* p) { mPrompt = (p ? p : ""); }

  // Call each frame to draw the input line. Returns true if a command was executed this frame.
  bool Render(const char* id = "##cmdline", const char* hint = "")
  {
    bool executed = false;

    // Optional prompt
    if (!mPrompt.empty()) {
      ImGui::TextUnformatted(mPrompt.c_str());
      ImGui::SameLine();
    }

    // Keep focus after submit if requested
    if (mFocusNext) {
      ImGui::SetKeyboardFocusHere();
      mFocusNext = false;
    }

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory |
                                ImGuiInputTextFlags_CallbackCompletion;

    // We pass 'this' as user data to manipulate history/completion inside the callback.
    bool submit =
      ImGui::InputTextWithHint(id, hint, mBuf.data(), (size_t)mBufSize, flags, &CommandLine::TextEditCallback_, this);

    if (submit) {
      // Trim whitespace
      char* s = mBuf.data();
      while (*s && std::isspace((unsigned char)*s))
        ++s;
      char* e = s + std::strlen(s);
      while (e > s && std::isspace((unsigned char)e[-1]))
        --e;
      *e = '\0';

      if (*s) {
        // push to history (dedupe against most recent)
        pushHistory_(s);
        // exec
        if (onExecute)
          onExecute(s);
        executed = true;
      }
      // reset editing state to accept a new command
      clearInput_();
      mHistoryPos = -1;
      mEditBackup.clear();
      mFocusNext = true; // keep cursor in the input
    }

    return executed;
  }

  // Optional: programmatically inject a command into history
  void addHistory(const char* cmd)
  {
    if (cmd && *cmd)
      pushHistory_(cmd);
  }

  // Optional: clear history
  void clearHistory()
  {
    mHistory.clear();
    mHistoryPos = -1;
    mEditBackup.clear();
  }

  // Read-only accessors
  const std::vector<std::string>& history() const { return mHistory; }

  // Prompt + buffer size configuration (buffer resize clears content)
  void setBufferSize(int new_size)
  {
    new_size = std::max(64, new_size);
    if (new_size == mBufSize)
      return;
    mBufSize = new_size;
    mBuf.assign((size_t)mBufSize, '\0');
    clearInput_();
  }

private:
  // --- state --------------------------------------------------------------
  std::string mPrompt = "> ";
  int mBufSize = 1024;
  std::vector<char> mBuf; // persistent edit buffer
  std::vector<std::string> mHistory;
  int mHistoryPos = -1;    // -1 = not browsing; otherwise index into mHistory
  std::string mEditBackup; // original line prior to first ↑ (so we can restore)
  size_t mMaxHistory = 128;
  bool mFocusNext = false; // request focus next frame (after submit)

  ExecuteFn onExecute;
  CompleteFn onComplete;

  // --- helpers ------------------------------------------------------------
  static int TextEditCallback_(ImGuiInputTextCallbackData* data)
  {
    auto* self = static_cast<CommandLine*>(data->UserData);
    switch (data->EventFlag) {
      case ImGuiInputTextFlags_CallbackHistory:
        return self->onHistory_(data);
      case ImGuiInputTextFlags_CallbackCompletion:
        return self->onCompletion_(data);
      default:
        return 0;
    }
  }

  int onHistory_(ImGuiInputTextCallbackData* data)
  {
    // First time pressing ↑ or ↓? remember current edit line
    if (mHistoryPos == -1)
      mEditBackup.assign(data->Buf);

    if (data->EventKey == ImGuiKey_UpArrow) {
      if (!mHistory.empty())
        mHistoryPos = (mHistoryPos < 0) ? (int)mHistory.size() - 1 : std::max(0, mHistoryPos - 1);
    } else if (data->EventKey == ImGuiKey_DownArrow) {
      if (!mHistory.empty()) {
        if (mHistoryPos >= 0) {
          mHistoryPos++;
          if (mHistoryPos >= (int)mHistory.size())
            mHistoryPos = -1; // go back to edit buffer
        }
      }
    }

    const char* replacement = nullptr;
    if (mHistoryPos == -1) {
      replacement = mEditBackup.c_str();
    } else {
      replacement = mHistory[(size_t)mHistoryPos].c_str();
    }

    data->DeleteChars(0, data->BufTextLen);
    data->InsertChars(0, replacement);
    return 0;
  }

  int onCompletion_(ImGuiInputTextCallbackData* data)
  {
    if (!onComplete)
      return 0;

    // Find word start at cursor: [word = [A-Za-z0-9_-.]]
    int cur = data->CursorPos;
    int start = cur;
    auto is_word = [](char c) { return std::isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.'; };
    while (start > 0 && is_word(data->Buf[start - 1]))
      --start;

    std::string prefix(data->Buf + start, data->Buf + cur);
    std::vector<std::string> suggestions;
    onComplete(prefix.c_str(), suggestions);
    if (suggestions.empty())
      return 0;

    // If one suggestion -> replace; if many -> insert longest common prefix
    auto lcp = [](const std::string& a, const std::string& b) {
      size_t n = std::min(a.size(), b.size());
      size_t i = 0;
      for (; i < n && a[i] == b[i]; ++i) {
      }
      return a.substr(0, i);
    };

    std::string insert = suggestions[0];
    if (suggestions.size() > 1) {
      for (size_t i = 1; i < suggestions.size(); ++i)
        insert = lcp(insert, suggestions[i]);
      if (insert.empty())
        return 0; // nothing in common
    }

    // Replace the word [start, cur) with 'insert'
    data->DeleteChars(start, cur - start);
    data->InsertChars(start, insert.c_str());
    data->CursorPos = start + (int)insert.size();
    return 0;
  }

  void pushHistory_(const char* s)
  {
    if (!mHistory.empty() && mHistory.back() == s)
      return; // de-dupe last
    mHistory.emplace_back(s);
    if (mHistory.size() > mMaxHistory)
      mHistory.erase(mHistory.begin(), mHistory.begin() + (mHistory.size() - mMaxHistory));
  }

  void clearInput_() { std::fill(mBuf.begin(), mBuf.end(), 0); }
};
