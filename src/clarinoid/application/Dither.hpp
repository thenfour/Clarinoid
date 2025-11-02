#pragma once
#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct IDitherMatrix
{
    virtual bool getDitheredColor(int shade, const PointI &pt) const = 0;
};

template <size_t N>
class DitherMatrix : public IDitherMatrix
{
  private:
    static constexpr size_t matrixSize = N;
    int values[N * N];
    int valuesScaled[N * N];
    int maxThreshold;

    int findMaxThreshold()
    {
        int maxVal = -32768;
        for (size_t i = 0; i < N * N; i++)
        {
            if (values[i] > maxVal)
                maxVal = values[i];
        }
        return maxVal;
    }

    int getScaledThreshold(const PointI &pt) const
    {
        int mx = (pt.x) % N;
        int my = (pt.y) % N;
        return valuesScaled[my * N + mx];
    }

  public:
    explicit DitherMatrix(std::initializer_list<std::initializer_list<int>> init)
    {
        size_t y = 0;
        for (auto row : init)
        {
            size_t x = 0;
            for (auto val : row)
            {
                values[y * N + x] = val;
                x++;
            }
            y++;
        }
        maxThreshold = findMaxThreshold();
        int scale = 256 / (maxThreshold + 1);
        for (size_t i = 0; i < N * N; i++)
        {
            valuesScaled[i] = (values[i] + 1) * scale;
        }
    }

    virtual bool getDitheredColor(int shade, const PointI &pt) const override
    {
        int thresh = getScaledThreshold(pt);
        return (shade > thresh);
    }
};

} // namespace clarinoid