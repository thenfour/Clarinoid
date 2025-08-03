#pragma once

namespace clarinoid {

// this needs to be a POD for the QuickStringList optimized vector.
template <typename _Char> struct QuickStringData {
  size_t m_len = 0;
  size_t m_allocated = 0;
  static const size_t staticBufferSize = 16;
  _Char staticBuffer[staticBufferSize];
  _Char *dynBuffer = nullptr;
  _Char *p = nullptr;
};

// attaches to QuickStringData to act like a std::string.
template <typename _Char> struct QuickString {
  // private:
  //	QuickString<_Char>& operator =(QuickString<_Char>& rhs)
  //	{
  //		return *this;
  //	}
  //	QuickString(QuickString<_Char>& rhs)
  //	{
  //	}

public:
  QuickString(QuickStringData<_Char> *data_) : data(data_) {}

  inline const _Char *c_str() const { return data->p; }

  inline size_t size() const { return data->m_len; }

  inline void append(const _Char *c) {
    size_t inputLen = LibCC::StringLength(c);
    AddAlloc(inputLen);
    _Char *i = data->p + data->m_len;
    memcpy(i, c, sizeof(_Char) * inputLen);
    i += inputLen;
    *i = 0;
    data->m_len += inputLen;
  }

  inline void push_back(_Char ch) {
    AddAlloc(1);
    _Char *i = data->p + data->m_len;
    *i = ch;
    ++i;
    *i = 0;
    data->m_len++;
  }

  inline void clear_and_reserve(size_t n) {
    if (data->m_allocated >= n)
      return;

    if (data->p != data->staticBuffer) {
      HeapFree(GetProcessHeap(), 0, data->p);
    }
    data->m_len = 0;
    data->m_allocated = n + 1;
    data->dynBuffer = (_Char *)HeapAlloc(GetProcessHeap(), 0,
                                         data->m_allocated * sizeof(_Char));
    data->p = data->dynBuffer;
  }

  bool empty() const { return data->p[0] == 0; }

  typedef _Char *iterator;
  typedef const _Char *const_iterator;
  iterator begin() { return data->p; }
  iterator end() { return data->p + data->m_len; }

  const_iterator begin() const { return data->p; }
  const_iterator end() const { return data->p + data->m_len; }

  void assign(const _Char *rhs) {
    size_t inputLen = LibCC::StringLength(rhs) + 1;
    reserve(inputLen);
    memcpy(data->p, rhs, sizeof(_Char) * inputLen);
  }

private:
  inline void AddAlloc(size_t additional) {
    if (data->m_allocated < (data->m_len + 1 + additional)) // 1 for null term
    {
      data->m_allocated = std::max(additional + 1, data->m_allocated << 1);
      _Char *newp = (_Char *)HeapAlloc(GetProcessHeap(), 0,
                                       data->m_allocated * sizeof(_Char));
      memcpy(newp, data->p, data->m_len * sizeof(_Char));
      if (data->p != data->staticBuffer) {
        HeapFree(GetProcessHeap(), 0, data->p);
      }
      data->p = newp;
    }
  }

  QuickStringData<_Char> *data;
};

} // namespace clarinoid
