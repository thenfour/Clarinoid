#pragma once


template<class T>
class ComPtr
{
public:
  ComPtr() noexcept = default;
  ComPtr(std::nullptr_t) noexcept {}
  explicit ComPtr(T* p) noexcept
    : p_(p)
  {
  }

  ComPtr(const ComPtr& o) noexcept
    : p_(o.p_)
  {
    if (p_)
      p_->AddRef();
  }
  ComPtr(ComPtr&& o) noexcept
    : p_(o.p_)
  {
    o.p_ = nullptr;
  }

  ~ComPtr()
  {
    if (p_)
      p_->Release();
  }

  ComPtr& operator=(const ComPtr& o) noexcept
  {
    if (this != &o) {
      if (o.p_)
        o.p_->AddRef();
      if (p_)
        p_->Release();
      p_ = o.p_;
    }
    return *this;
  }
  ComPtr& operator=(ComPtr&& o) noexcept
  {
    if (this != &o) {
      if (p_)
        p_->Release();
      p_ = o.p_;
      o.p_ = nullptr;
    }
    return *this;
  }

  // Basic access
  T* Get() const noexcept { return p_; }
  T* operator->() const noexcept { return p_; }
  explicit operator bool() const noexcept { return p_ != nullptr; }

  // Out-parameter helpers for CreateXxx(..., T**)
  T** GetAddressOf() noexcept { return &p_; }
  T** ReleaseAndGetAddressOf() noexcept
  {
    if (p_) {
      p_->Release();
      p_ = nullptr;
    }
    return &p_;
  }

  // Ownership helpers
  void Reset() noexcept
  {
    if (p_) {
      p_->Release();
      p_ = nullptr;
    }
  }
  void Attach(T* p) noexcept
  {
    if (p_)
      p_->Release();
    p_ = p;
  }
  T* Detach() noexcept
  {
    T* t = p_;
    p_ = nullptr;
    return t;
  }

  // QueryInterface to another interface on the same object
  template<class U>
  HRESULT As(ComPtr<U>& out) const noexcept
  {
    if (!p_) {
      out.Reset();
      return E_POINTER;
    }
    U* q = nullptr;
    HRESULT hr = p_->QueryInterface(__uuidof(U), reinterpret_cast<void**>(&q));
    if (SUCCEEDED(hr))
      out.Attach(q);
    return hr;
  }

private:
  T* p_ = nullptr;
};
