
#pragma once

namespace clarinoid
{
  // as you push items, old items are overwritten. useful for plotting on a screen fe.
  // when READING items, they're returned in the order they were written.
  template<typename Tval, size_t N>
  struct CircularArray
  {
  private:
    size_t mUsed = 0;
    size_t mWriteCursor = 0;
    size_t mReadCursor = 0; // when the array isn't full, this is always 0. otherwise it's just the write cursor + 1 because of wrapping.
    Tval mArray[N];

  public:
    void Clear()
    {
      mUsed = 0;
      mWriteCursor = 0;
      mReadCursor = 0;
    }

    size_t GetSize()
    {
      return mUsed;
    }

    Tval& GetElementAt(size_t n)
    {
      CCASSERT(mUsed);
      return mArray[(n + mReadCursor) % mUsed];
    }

    void Push(const Tval& val)
    {
      if (mUsed < N) {
        mArray[mUsed] = val;
        mUsed++;
        mWriteCursor++;
        // read cursor stays 0.
        return;
      }

      // the array is filled; overwrite.
      mWriteCursor = mWriteCursor % N;
      mReadCursor++;
      mArray[mWriteCursor] = val;
      mWriteCursor++;
    }

  };

} // namespace clarinoid
