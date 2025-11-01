// fixed_vector.hpp  (C++14, no heap)
// A vector-like container with fixed capacity and in-place storage.
// - No dynamic allocation
// - Trivially relocatable (iterator = T*)
// - Provides a familiar subset: size/capacity, push_back/emplace_back, pop_back,
//   operator[], at(), front/back, data(), begin/end, clear(), resize(), insert(), erase()
// Notes:
// - On overflow, push/insert return false (or assert, see FIXED_VECTOR_ASSERT).
// - Strong exception safety for element construction/moves (if T throws, already-constructed elements remain valid).

#ifndef FIXED_VECTOR_HPP
#define FIXED_VECTOR_HPP

#include <type_traits>
#include <utility>
#include <cassert>
#include <initializer_list>

#ifndef FIXED_VECTOR_ASSERT
#define FIXED_VECTOR_ASSERT(expr) assert(expr)
#endif

namespace clarinoid
{

template <class T, std::size_t N>
class fixed_vector
{
    static_assert(N > 0, "fixed_vector capacity N must be > 0");

    using storage_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;
    using iterator = T *;
    using const_iterator = const T *;

    // --- ctors/dtor/assign ---
    fixed_vector() noexcept : size_(0)
    {
    }

    fixed_vector(std::initializer_list<T> ilist) : size_(0)
    {
        FIXED_VECTOR_ASSERT(ilist.size() <= N);
        for (const auto &v : ilist)
            emplace_back(v);
    }

    fixed_vector(const fixed_vector &other) : size_(0)
    {
        for (size_type i = 0; i < other.size_; ++i)
            emplace_back(other[i]);
    }

    fixed_vector(fixed_vector &&other) noexcept(std::is_nothrow_move_constructible<T>::value) : size_(0)
    {
        // Move-construct elements in order
        for (size_type i = 0; i < other.size_; ++i)
            emplace_back(std::move(other[i]));
        other.clear();
    }

    fixed_vector &operator=(const fixed_vector &other)
    {
        if (this == &other)
            return *this;
        // copy-assign into existing where possible; otherwise rebuild
        if (other.size_ <= size_)
        {
            for (size_type i = 0; i < other.size_; ++i)
                data_ptr()[i] = other[i];
            // destroy extras
            destroy_range(other.size_, size_);
            size_ = other.size_;
        }
        else
        {
            // assign into overlap
            for (size_type i = 0; i < size_; ++i)
                data_ptr()[i] = other[i];
            // construct the remainder
            for (size_type i = size_; i < other.size_; ++i)
                ::new (void_ptr(i)) T(other[i]);
            size_ = other.size_;
        }
        return *this;
    }

    fixed_vector &operator=(fixed_vector &&other) noexcept(std::is_nothrow_move_assignable<T>::value &&
                                                           std::is_nothrow_move_constructible<T>::value)
    {
        if (this == &other)
            return *this;
        clear();
        for (size_type i = 0; i < other.size_; ++i)
            emplace_back(std::move(other[i]));
        other.clear();
        return *this;
    }

    ~fixed_vector()
    {
        clear();
    }

    // --- capacity/size ---
    static constexpr size_type capacity() noexcept
    {
        return N;
    }
    size_type size() const noexcept
    {
        return size_;
    }
    bool empty() const noexcept
    {
        return size_ == 0;
    }
    size_type max_size() const noexcept
    {
        return N;
    }
    void shrink_to_fit() noexcept
    {
    } // no-op

    // --- iterators ---
    iterator begin() noexcept
    {
        return data_ptr();
    }
    const_iterator begin() const noexcept
    {
        return data_ptr();
    }
    const_iterator cbegin() const noexcept
    {
        return data_ptr();
    }

    iterator end() noexcept
    {
        return data_ptr() + size_;
    }
    const_iterator end() const noexcept
    {
        return data_ptr() + size_;
    }
    const_iterator cend() const noexcept
    {
        return data_ptr() + size_;
    }

    // --- element access ---
    reference operator[](size_type i) noexcept
    {
        return data_ptr()[i];
    }
    const_reference operator[](size_type i) const noexcept
    {
        return data_ptr()[i];
    }

    reference at(size_type i)
    {
        FIXED_VECTOR_ASSERT(i < size_);
        return data_ptr()[i];
    }
    const_reference at(size_type i) const
    {
        FIXED_VECTOR_ASSERT(i < size_);
        return data_ptr()[i];
    }

    reference front()
    {
        return data_ptr()[0];
    }
    const_reference front() const
    {
        return data_ptr()[0];
    }
    reference back()
    {
        return data_ptr()[size_ - 1];
    }
    const_reference back() const
    {
        return data_ptr()[size_ - 1];
    }

    pointer data() noexcept
    {
        return data_ptr();
    }
    const_pointer data() const noexcept
    {
        return data_ptr();
    }

    // --- modifiers ---
    void clear() noexcept
    {
        destroy_range(0, size_);
        size_ = 0;
    }

    // returns false if full
    bool push_back(const T &v)
    {
        if (size_ >= N)
            return false;
        ::new (void_ptr(size_)) T(v);
        ++size_;
        return true;
    }
    bool push_back(T &&v)
    {
        if (size_ >= N)
            return false;
        ::new (void_ptr(size_)) T(std::move(v));
        ++size_;
        return true;
    }

    template <class... Args>
    bool emplace_back(Args &&...args)
    {
        if (size_ >= N)
            return false;
        ::new (void_ptr(size_)) T(std::forward<Args>(args)...);
        ++size_;
        return true;
    }

    void pop_back()
    {
        FIXED_VECTOR_ASSERT(size_ > 0);
        --size_;
        data_ptr()[size_].~T();
    }

    // resize to new_size; returns false if new_size > capacity
    bool resize(size_type new_size)
    {
        if (new_size > N)
            return false;
        if (new_size > size_)
        {
            for (size_type i = size_; i < new_size; ++i)
                ::new (void_ptr(i)) T();
        }
        else if (new_size < size_)
        {
            destroy_range(new_size, size_);
        }
        size_ = new_size;
        return true;
    }

    bool resize(size_type new_size, const T &value)
    {
        if (new_size > N)
            return false;
        if (new_size > size_)
        {
            for (size_type i = size_; i < new_size; ++i)
                ::new (void_ptr(i)) T(value);
        }
        else if (new_size < size_)
        {
            destroy_range(new_size, size_);
        }
        size_ = new_size;
        return true;
    }

    // insert single element at pos; returns iterator to inserted or end() if full
    iterator insert(const_iterator pos, const T &value)
    {
        return do_insert(pos, value);
    }
    iterator insert(const_iterator pos, T &&value)
    {
        return do_insert(pos, std::move(value));
    }

    // erase single element; returns iterator to next element
    iterator erase(const_iterator pos)
    {
        size_type idx = static_cast<size_type>(pos - cbegin());
        FIXED_VECTOR_ASSERT(idx < size_);
        // destroy element at idx
        data_ptr()[idx].~T();
        // move-construct down the tail
        for (size_type i = idx; i + 1 < size_; ++i)
        {
            ::new (void_ptr(i)) T(std::move(data_ptr()[i + 1]));
            data_ptr()[i + 1].~T();
        }
        --size_;
        return begin() + idx;
    }

    // Add inside fixed_vector<T, N> public section:

    // 1) assign(count, value)
    void assign(size_type count, const T &value)
    {
        FIXED_VECTOR_ASSERT(count <= N);
        // Reuse existing elements where possible
        size_type common = (count < size_) ? count : size_;
        for (size_type i = 0; i < common; ++i)
        {
            data_ptr()[i] = value;
        }

        if (count < size_)
        {
            // destroy the tail
            destroy_range(count, size_);
            size_ = count;
            return;
        }

        // construct the remainder
        // try {
        for (size_type i = size_; i < count; ++i)
        {
            ::new (void_ptr(i)) T(value);
            ++size_;
        }
        // catch (...) {
        //	// strong guarantee: clear everything if construction fails
        //	destroy_range(0, size_);
        //	size_ = 0;
        //	throw;
        // }
    }

    // 2) assign(first, last)
    template <class InputIt, typename std::enable_if<!std::is_integral<InputIt>::value, int>::type = 0>
    void assign(InputIt first, InputIt last)
    {
        // Prefer to compute distance if we can (forward or better) to pre-check capacity.
        using cat = typename std::iterator_traits<InputIt>::iterator_category;
        assign_impl(first, last, cat{});
    }

    // 3) assign(initializer_list)
    void assign(std::initializer_list<T> ilist)
    {
        FIXED_VECTOR_ASSERT(ilist.size() <= N);
        clear();
        // try {
        for (const auto &v : ilist)
        {
            ::new (void_ptr(size_)) T(v);
            ++size_;
        }
        //}
        // catch (...) {
        //	destroy_range(0, size_);
        //	size_ = 0;
        //	throw;
        //}
    }

  private:
    // Helper for assign(first,last) when we have only an InputIterator (single-pass)
    template <class InputIt>
    void assign_impl(InputIt first, InputIt last, std::input_iterator_tag)
    {
        clear();
        // try {
        for (; first != last; ++first)
        {
            FIXED_VECTOR_ASSERT(size_ < N);
            ::new (void_ptr(size_)) T(*first);
            ++size_;
        }
        //}
        // catch (...) {
        //	destroy_range(0, size_);
        //	size_ = 0;
        //	throw;
        //}
    }

    // Helper for assign(first,last) when we have at least a ForwardIterator
    template <class ForwardIt>
    void assign_impl(ForwardIt first, ForwardIt last, std::forward_iterator_tag)
    {
        const size_type cnt = static_cast<size_type>(std::distance(first, last));
        FIXED_VECTOR_ASSERT(cnt <= N);

        // Reuse/construct/destroy like the count overload
        size_type i = 0;
        const size_type common = (cnt < size_) ? cnt : size_;

        // overwrite existing range
        for (; i < common; ++i, ++first)
        {
            data_ptr()[i] = *first;
        }

        if (cnt < size_)
        {
            destroy_range(cnt, size_);
            size_ = cnt;
            return;
        }

        // construct remainder
        // try {
        for (; i < cnt; ++i, ++first)
        {
            ::new (void_ptr(i)) T(*first);
            ++size_;
        }
        //}
        // catch (...) {
        //	destroy_range(0, size_);
        //	size_ = 0;
        //	throw;
        //}
    }

  private:
    pointer data_ptr() noexcept
    {
        return reinterpret_cast<pointer>(&storage_[0]);
    }
    const_pointer data_ptr() const noexcept
    {
        return reinterpret_cast<const_pointer>(&storage_[0]);
    }
    void *void_ptr(size_type i) noexcept
    {
        return static_cast<void *>(&storage_[i]);
    }

    void destroy_range(size_type first, size_type last) noexcept
    {
        for (size_type i = first; i < last; ++i)
            data_ptr()[i].~T();
    }

    template <class U>
    iterator do_insert(const_iterator cpos, U &&value)
    {
        if (size_ >= N)
            return end();
        size_type idx = static_cast<size_type>(cpos - cbegin());
        FIXED_VECTOR_ASSERT(idx <= size_);

        if (idx == size_)
        {
            // append
            ::new (void_ptr(size_)) T(std::forward<U>(value));
            ++size_;
            return end() - 1;
        }

        // Make room by move-constructing the last element into a new slot
        ::new (void_ptr(size_)) T(std::move(back()));
        // Shift middle range down by one (from back-1 to idx)
        for (size_type i = size_ - 1; i > idx; --i)
        {
            data_ptr()[i] = std::move(data_ptr()[i - 1]); // move-assign within constructed range
        }
        // Place new value
        data_ptr()[idx].~T();
        ::new (void_ptr(idx)) T(std::forward<U>(value));
        ++size_;
        return begin() + idx;
    }

    storage_t storage_[N];
    size_type size_;
};

} // namespace clarinoid
#endif // FIXED_VECTOR_HPP
