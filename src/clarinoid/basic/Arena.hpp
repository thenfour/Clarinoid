#include <clarinoid/basic/assert.hpp>

namespace clarinoid
{

template <size_t TBytes>
struct MemoryArena
{
    uint8_t *base = nullptr;
    size_t cap = TBytes;
    size_t off = 0;

    explicit MemoryArena(void *basePtr) : base(static_cast<uint8_t *>(basePtr))
    {
    }

    // construct from literal array
    template <typename T, size_t N>
    explicit MemoryArena(T (&arr)[N]) : base(reinterpret_cast<uint8_t *>(arr)), cap(N * sizeof(T))
    {
    }

    void die()
    {
        CCDIE("Arena out of memory");
    }

    size_t GetFreeBytes() const
    {
        return cap - off;
    }

    template <typename T>
    T *alloc()
    {
        constexpr size_t A = alignof(T);
        size_t aligned = (off + (A - 1)) & ~(A - 1);
        if (aligned + sizeof(T) > cap)
        {
            die();
            return nullptr;
        }
        off = aligned + sizeof(T);
        return reinterpret_cast<T *>(base + aligned);
    }

    template <typename T>
    T *alloc_array(size_t count)
    { // raw array of T (not std::array)
        constexpr size_t A = alignof(T);
        size_t bytes = sizeof(T) * count;
        size_t aligned = (off + (A - 1)) & ~(A - 1);
        if (aligned + bytes > cap)
        {
            die();
            return nullptr;
        }
        off = aligned + bytes;
        return reinterpret_cast<T *>(base + aligned);
    }

    // allocate with placement new
    template <typename T, typename... Args>
    T *instantiate(Args &&...args)
    {
        T *mem = alloc<T>();
        return new (mem) T(std::forward<Args>(args)...);
    }

    // allocate array with placement new using default constructor
    template <typename T, size_t N>
    array_view<T, N> instantiate_array()
    {
        T *mem = alloc_array<T>(N);
        // if (mem == nullptr)
        //     return nullptr;
        for (size_t i = 0; i < N; ++i)
        {
            new (&mem[i]) T();
        }
        return array_view<T, N>(mem);
    }

    // instantiate array with values
    template <typename T, size_t N, typename... Args>
    array_view<T, N> instantiate_array(Args &&...args)
    {
        static_assert(sizeof...(args) == N, "Number of arguments must match array size N");
        T *mem = alloc_array<T>(N);
        // if (mem == nullptr)
        //     return nullptr;
        size_t i = 0;
        (new (&mem[i++]) T(std::forward<Args>(args)), ...);
        return array_view<T, N>(mem);
    }

    void reset()
    {
        off = 0;
    }

    // free last allocated block (must be last)
    template <typename T>
    void free_last(const T *ptr)
    {
        size_t ptrOffset = reinterpret_cast<const uint8_t *>(ptr) - base;
        if (ptrOffset + sizeof(T) == off)
        {
            off = ptrOffset;
        }
    }
};

// make_memory_arena
template <size_t TBytes>
inline MemoryArena<TBytes> make_memory_arena(void *basePtr)
{
    return MemoryArena<TBytes>(basePtr);
}

// make memory arena from static array
template <typename T, size_t N>
inline MemoryArena<N * sizeof(T)> make_memory_arena(T (&arr)[N])
{
    return MemoryArena<N * sizeof(T)>(arr);
}

} // namespace clarinoid
