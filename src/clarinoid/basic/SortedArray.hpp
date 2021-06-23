
#pragma once

namespace clarinoid
{

// keep maximal values at front. pushes out too-low items.
template <typename Tval, size_t N, typename Tlessthan>
struct SortedArray
{
    size_t mSize = 0;
    Tval mArray[N];
    Tlessthan mLessThan;

    SortedArray(Tlessthan &&lt) : mLessThan(lt)
    {
    }

    void Clear()
    {
        mSize = 0;
    }

    bool Insert(Tval &&newVal)
    {
        size_t newIndex = 0;
        // std::optional<Tval> ret;
        bool ret = false;
        if (mSize > 0)
        {
            // figure out where to place it; use binary search.
            size_t windowLeft = 0;
            // place at END, not last index. It means we never actually compare windowRight.
            // this plays well with finding the edge.
            size_t windowRight = std::min(mSize, N - 1);
            while (true)
            {
                size_t edge = (windowRight + windowLeft) / 2;
                CCASSERT(windowLeft != windowRight); // because R is never compared ("end"), they should never meet.
                if (mLessThan(newVal, mArray[edge]))
                {
                    //     E          where E == L
                    //     L--------R where R = L+1
                    //      ====V===|   <-- val is less than L which means it's R.
                    // OR
                    //     L--------E--------R
                    //     |         ====V===|
                    // =>    -->    |        |
                    if (windowLeft == edge)
                    {
                        CCASSERT(windowRight == (windowLeft + 1));
                        newIndex = windowRight;
                        break;
                    }
                    windowLeft = edge;
                }
                else
                {
                    //     E            where E == L
                    //     L--------R   where R = L+1
                    //     V therefore V == E, and therefore should be inserted at E.
                    //    Here,
                    // OR
                    //     L--------E--------R
                    //     |====V====        |
                    // =>  |        |    <--
                    if (windowLeft == edge)
                    {
                        CCASSERT(windowRight == (windowLeft + 1));
                        newIndex = edge;
                        break;
                    }
                    windowRight = edge;
                }
            }

            // insert it there...
            // remember last item.
            size_t itemsToSlide = mSize - newIndex;
            if (mSize == N)
            {
                // we'll be pushing an item off.
                --itemsToSlide;
                ret = true;
                // ret.emplace(std::move(mArray[N - 1]));
            }

            for (size_t i = newIndex + itemsToSlide; i > newIndex; --i)
            {
                mArray[i] = std::move(mArray[i - 1]);
            }
        }

        mArray[newIndex] = std::move(newVal);
        if (mSize < N)
        {
            ++mSize;
        }

        return ret;
    }
};

} // namespace clarinoid
