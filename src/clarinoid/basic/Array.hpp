
#pragma once

namespace clarinoid {

template <typename T, size_t N> struct StaticArray {
  T (&mArray)[N];
  static constexpr size_t Size = N;
  StaticArray(T (&x)[N]) : mArray(x) {}
};

template <typename T, size_t N>
constexpr size_t SizeofStaticArray(const T (&x)[N]) {
  return N;
}

template <typename T, size_t N>
void CopyPODArray(const T (&from)[N], T (&to)[N]) {
  memcpy(to, from, sizeof(T) * N);
}

template <typename T> void CopyPODArray(const T *from, T *to, size_t N) {
  memcpy(to, from, sizeof(T) * N);
}

template <typename T> struct array_view {
  size_t mSize = 0;
  T *mData = nullptr;

  array_view() = default;
  size_t size() const { return mSize; }

  template <size_t N> array_view(T (&a)[N]) : mSize(N), mData(a) {}

  T &operator[](size_t i) {
    //CCASSERT(i < mSize);
    return mData[i];
  }
};

template <typename T, size_t N> array_view<T> make_array_view(T (&a)[N]) {
  return array_view<T>(a);
}

// https://stackoverflow.com/questions/26351587/how-to-create-stdarray-with-initialization-list-without-providing-size-directl
template <typename V, typename... T>
constexpr auto array_of(T &&...t) -> std::array<V, sizeof...(T)> {
  return {{std::forward<T>(t)...}};
}



} // namespace clarinoid