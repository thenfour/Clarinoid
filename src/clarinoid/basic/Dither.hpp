#pragma once

#include "Geometry.hpp"

namespace clarinoid {

struct IDitherMatrix
{
  virtual bool getDitheredColor(int shade, const PointI& pt) const = 0;
};

template<size_t N>
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
    for (size_t i = 0; i < N * N; i++) {
      if (values[i] > maxVal)
        maxVal = values[i];
    }
    return maxVal;
  }

  int getScaledThreshold(const PointI& pt) const
  {
    int mx = (pt.x) % N;
    int my = (pt.y) % N;
    return valuesScaled[my * N + mx];
  }

public:
  // explicit DitherMatrix(const int (&inputValues)[N][N])
  // {
  //     for (size_t y = 0; y < N; y++)
  //     {
  //         for (size_t x = 0; x < N; x++)
  //         {
  //             values[y * N + x] = inputValues[y][x];
  //         }
  //     }
  //     maxThreshold = findMaxThreshold();
  //     int scale = 256 / (maxThreshold + 1);
  //     for (size_t i = 0; i < N * N; i++)
  //     {
  //         valuesScaled[i] = (values[i] + 1) * scale;
  //     }
  // }

  explicit DitherMatrix(std::initializer_list<std::initializer_list<int>> init)
  {
    size_t y = 0;
    for (auto row : init) {
      size_t x = 0;
      for (auto val : row) {
        values[y * N + x] = val;
        x++;
      }
      y++;
    }
    maxThreshold = findMaxThreshold();
    int scale = 256 / (maxThreshold + 1);
    for (size_t i = 0; i < N * N; i++) {
      valuesScaled[i] = (values[i] + 1) * scale;
    }
  }

  virtual bool getDitheredColor(int shade, const PointI& pt) const override
  {
    int thresh = getScaledThreshold(pt) + 1; // why +1?
    return (shade > thresh);
  }
};


static DitherMatrix<2> gBayer2x2Matrix{ { 0, 2, 3, 1 } };

static DitherMatrix<4> gBayer4x4Matrix{ { 0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5 } };

static DitherMatrix<8> gBayer8x8Matrix{ { 0,  48, 12, 60, 3,  51, 15, 63, 32, 16, 44, 28, 35, 19, 47, 31,
                                          8,  56, 4,  52, 11, 59, 7,  55, 40, 24, 36, 20, 43, 27, 39, 23,
                                          2,  50, 14, 62, 1,  49, 13, 61, 34, 18, 46, 30, 33, 17, 45, 29,
                                          10, 58, 6,  54, 9,  57, 5,  53, 42, 26, 38, 22, 41, 25, 37, 21 } };


} // namespace clarinoid