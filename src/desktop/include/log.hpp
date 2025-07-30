
#pragma once

#include <atomic>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace cc {

class Log
{
public:
  static void WriteLine(const std::string& s)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    WriteLineUnlocked(s);
  }

  static void WriteLineThenIncreaseIndent(const std::string& s)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    WriteLineUnlocked(s);
    ++indent_;
  }

  static void DecreaseIndentThenWriteLine(const std::string& s)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (indent_ > 0)
      --indent_;
    WriteLineUnlocked(s);
  }

  // Helper to format durations like "hh:mm:ss.mmm"
  template<class Duration>
  static std::string FormatDuration(Duration d)
  {
    using namespace std::chrono;
    auto ms_total = duration_cast<milliseconds>(d).count();
    auto hours = ms_total / 3'600'000;
    auto mins = (ms_total / 60'000) % 60;
    auto secs = (ms_total / 1'000) % 60;
    auto millis = ms_total % 1'000;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ':' << std::setw(2) << std::setfill('0') << mins << ':'
        << std::setw(2) << std::setfill('0') << secs << '.' << std::setw(3) << std::setfill('0') << millis;
    return oss.str();
  }

private:
  static std::string Iso8601UtcNow()
  {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto secs = time_point_cast<std::chrono::seconds>(now);
    const auto ms = duration_cast<std::chrono::milliseconds>(now - secs).count();

    std::time_t tt = system_clock::to_time_t(secs);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%FT%T") << '.' << std::setw(3) << std::setfill('0') << ms << 'Z';
    return oss.str();
  }

  static unsigned short ThreadId4()
  {
    const auto h = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return static_cast<unsigned short>(h % 10000);
  }

  static void WriteLineUnlocked(const std::string& s)
  {
    std::ostringstream oss;
    oss << "## CC:[" << Iso8601UtcNow() << "][" << std::setw(4) << std::setfill('0') << ThreadId4() << "] "
        << std::string(static_cast<size_t>(2 * indent_), ' ') << s;
    const auto line = oss.str();

    // Match Debug.WriteLine + Console.WriteLine: write to both clog and cout.
    std::clog << line << '\n';
    std::cout << line << '\n';
    std::cout.flush();
  }

  static inline std::mutex mutex_;
  static inline int indent_ = 0;
};

class LogScope
{
public:
  explicit LogScope(std::string s)
    : msg_(std::move(s))
    , start_(std::chrono::steady_clock::now())
    , id_(next_id_.fetch_add(1, std::memory_order_relaxed))
  {
    Log::WriteLineThenIncreaseIndent("{ #" + std::to_string(id_) + " " + msg_);
  }

  // Non-copyable
  LogScope(const LogScope&) = delete;
  LogScope& operator=(const LogScope&) = delete;

  // Movable (optional)
  LogScope(LogScope&&) = delete;
  LogScope& operator=(LogScope&&) = delete;

  ~LogScope() noexcept
  {
    using namespace std::chrono;
    const auto elapsed = steady_clock::now() - start_;
    Log::DecreaseIndentThenWriteLine("} #" + std::to_string(id_) + " " + msg_ + " (" + Log::FormatDuration(elapsed) +
                                     ")");
  }

private:
  std::string msg_;
  std::chrono::steady_clock::time_point start_;
  int id_;
  static inline std::atomic<int> next_id_{ 0 };
};

} // namespace cc
