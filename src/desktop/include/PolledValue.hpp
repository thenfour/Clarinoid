#pragma once

// polls a value-fetching function on a background thread at regular intervals,
// and exposes the latest value as a thread-safe snapshot.
// the point is for callers to be able to use a slow-fetching or async value without
// blocking the thread or dealing with this logic. Just call `snapshot()` to get the latest value.

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

enum class AsyncStatus
{
  Loading,
  Ready,
  Error
};

template<class T>
struct SnapshotData
{
  AsyncStatus status = AsyncStatus::Loading;
  std::string error; // set when Error
  std::chrono::steady_clock::time_point last_update{};
  uint64_t version = 0;
  std::optional<T> value; // empty when Loading/Error
};

template<class T>
class Snapshot
{
public:
  Snapshot() = default;
  explicit Snapshot(std::shared_ptr<const SnapshotData<T>> p)
    : p_(std::move(p))
  {
  }

  AsyncStatus status() const { return p_ ? p_->status : AsyncStatus::Loading; }
  uint64_t version() const { return p_ ? p_->version : 0; }
  auto last_update() const { return p_ ? p_->last_update : std::chrono::steady_clock::time_point{}; }
  const std::string& error() const
  {
    static std::string empty;
    return p_ ? p_->error : empty;
  }
  bool has_value() const { return p_ && p_->value.has_value(); }
  const T* get() const { return has_value() ? &*p_->value : nullptr; }

  template<class Rep, class Period>
  bool is_stale(std::chrono::duration<Rep, Period> ttl) const
  {
    if (!p_ || p_->last_update.time_since_epoch().count() == 0)
      return true;
    return (std::chrono::steady_clock::now() - p_->last_update) > ttl;
  }

private:
  std::shared_ptr<const SnapshotData<T>> p_;
};

template<class T>
class Polled
{
public:
  using FetchFn = std::function<T()>; // throw on error or return T

  Polled(std::chrono::milliseconds interval, FetchFn fetch)
    : interval_(interval)
    , fetch_(std::move(fetch))
  {
    publish(make_loading(0));
    worker_ = std::thread([this] { run(); });
  }

  ~Polled()
  {
    {
      std::lock_guard<std::mutex> lk(wait_m_);
      stop_.store(true, std::memory_order_relaxed);
      refresh_requested_ = true;
    }
    wait_cv_.notify_one();
    if (worker_.joinable())
      worker_.join();
  }

  Snapshot<T> snapshot() const { return Snapshot<T>(std::atomic_load(&snapshot_)); }

  void request_refresh()
  {
    {
      std::lock_guard<std::mutex> lk(wait_m_);
      refresh_requested_ = true;
    }
    wait_cv_.notify_one();
  }

  void set_interval(std::chrono::milliseconds d)
  {
    {
      std::lock_guard<std::mutex> lk(wait_m_);
      interval_ = d;
    }
    wait_cv_.notify_one();
  }

private:
  static std::shared_ptr<SnapshotData<T>> make_loading(uint64_t ver)
  {
    auto s = std::make_shared<SnapshotData<T>>();
    s->status = AsyncStatus::Loading;
    s->version = ver;
    return s;
  }

  static std::shared_ptr<SnapshotData<T>> make_ready(T&& v, uint64_t ver)
  {
    auto s = std::make_shared<SnapshotData<T>>();
    s->status = AsyncStatus::Ready;
    s->value = std::move(v);
    s->last_update = std::chrono::steady_clock::now();
    s->version = ver;
    return s;
  }

  static std::shared_ptr<SnapshotData<T>> make_error(std::string msg, uint64_t ver)
  {
    auto s = std::make_shared<SnapshotData<T>>();
    s->status = AsyncStatus::Error;
    s->error = std::move(msg);
    s->last_update = std::chrono::steady_clock::now();
    s->version = ver;
    return s;
  }

  void publish(std::shared_ptr<SnapshotData<T>> s) { std::atomic_store(&snapshot_, std::move(s)); }

  void run()
  {
    uint64_t ver = 0;

    // Try an immediate fetch so first frames see data quickly.
    do_once_fetch(ver);

    while (!stop_.load(std::memory_order_relaxed)) {
      std::unique_lock<std::mutex> lk(wait_m_);
      auto local_interval = interval_;
      auto until = std::chrono::steady_clock::now() + local_interval;

      // Wake on stop, explicit refresh, or timeout.
      wait_cv_.wait_until(lk, until, [&] { return stop_.load(std::memory_order_relaxed) || refresh_requested_; });

      if (stop_.load(std::memory_order_relaxed))
        break;

      bool was_refresh = refresh_requested_;
      refresh_requested_ = false;
      lk.unlock();

      (void)was_refresh; // both refresh and timeout perform a fetch
      do_once_fetch(ver);
    }
  }

  void do_once_fetch(uint64_t& ver)
  {
    try {
      T v = fetch_();
      publish(make_ready(std::move(v), ++ver));
    } catch (const std::exception& e) {
      publish(make_error(e.what(), ++ver));
    } catch (...) {
      publish(make_error("unknown error", ++ver));
    }
  }

  // data
  mutable std::shared_ptr<SnapshotData<T>> snapshot_;
  std::thread worker_;
  std::atomic<bool> stop_{ false };
  FetchFn fetch_;

  // wait/refresh
  std::mutex wait_m_;
  std::condition_variable wait_cv_;
  bool refresh_requested_ = false;
  std::chrono::milliseconds interval_;
};


