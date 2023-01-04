
#include <Arduino.h>

template<uint8_t frac_bits>
uint32_t fixed_point_divide_1(uint32_t x, uint32_t y)
{
    uint64_t x_64 = (uint64_t)x;
    uint64_t y_64 = (uint64_t)y;
    uint64_t result = x_64 / y_64;
    result = result << frac_bits;
    return (uint32_t)result;
}

template<uint8_t frac_bits>
uint32_t fixed_point_divide_2(uint32_t x, uint32_t y)
{
    x = x >> frac_bits;
    uint32_t result = 0;
    uint32_t accumulator = 0;
    for (int i = 31; i >= frac_bits; i--)
    {
        accumulator = accumulator << 1;
        uint32_t bit = (x >> i) & 1;
        accumulator = accumulator | bit;
        if (accumulator >= y)
        {
            accumulator = accumulator - y;
            result = result | (1 << i);
        }
    }
    return result;
}

template<uint8_t frac_bits>
uint32_t fixed_point_divide_3(uint32_t x, uint32_t y)
{
    x = x >> frac_bits;
    uint32_t result = 0;
    uint32_t accumulator = 0;
    for (int i = 31; i >= 0; i--)
    {
        accumulator = accumulator << 1;
        uint32_t bit = (x >> i) & 1;
        accumulator = accumulator | bit;
        if (accumulator >= y)
        {
            accumulator = accumulator - y;
            result = result | (1 << i);
        }
    }
    return result;
}

template<uint8_t frac_bits>
uint32_t fixed_point_divide_4(uint32_t x, uint32_t y)
{
    x = x >> frac_bits;
    uint32_t result = 0;
    uint32_t accumulator = 0;
    for (int i = 31; i >= frac_bits; i--)
    {
        accumulator = accumulator << 1;
        uint32_t bit = (x >> i) & 1;
        accumulator = accumulator | bit;
        if (accumulator >= y)
        {
            accumulator = accumulator - y;
            result = result | (1 << i);
        }
    }
    return result;
}

template <uint8_t fractBitsA, uint8_t fractBitsB>
uint32_t fixed_point_divide_5(uint32_t a, uint32_t b)
{
    auto a2 = uint64_t(a) << (fractBitsB);
    return a2 / b;
}


template <uint8_t fractBitsA, uint8_t fractBitsB>
uint32_t fixed_point_divide_6(uint32_t a, uint32_t b)
{
    auto a2 = float(a) * (1 << fractBitsB);
    return a2 / b;
}

template <uint8_t fractBitsA, uint8_t fractBitsB>
uint32_t fixed_point_divide_7(uint32_t a, uint32_t b)
{
    auto a2 = double(a) * (1 << fractBitsB);
    return a2 / b;
}



volatile int y = 0;

int Overhead(int a, int b)
{
  return 0;
}


int gOverheadMicros = 0;

void TestOverhead()
{
  static constexpr int iterations = 100000;
  int x = 0;
  int m1 = micros();
  for (int i = 0; i < iterations; ++ i) {
    x += Overhead(rand(), rand()) + y;
  }
  int m2 = micros();
  int m = m2 - m1;
  gOverheadMicros = m;

  Serial.println(String("OVERHEAD") +
  "  x=" + String(int32_t(x),16) +
  ", micros:" + m
  );
}

template<typename T, typename Tfn>
void Test(const char * testName, Tfn fn)
{
  static constexpr int iterations = 100000;
  T x = 0;
  int m1 = micros();
  for (int i = 0; i < iterations; ++ i) {
    x += fn(rand(), rand()) + y;
  }
  int m2 = micros();
  int m = m2 - m1;
  m -= gOverheadMicros;

  Serial.println(String(testName) +
  "\t" + String(int32_t(x),16) + // necessary to prevent optimizing out.
  "\t" + m
  );
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  srand(99);
  
  TestOverhead();

  static constexpr uint8_t fract_bits = 4;
  Test<uint32_t>("fixed_point_divide_1", &fixed_point_divide_1<fract_bits>); // divide using 64-bit intermediates.
  Test<uint32_t>("fixed_point_divide_2", &fixed_point_divide_2<fract_bits>); // trying to avoid 64-bit values while staying in FP
  Test<uint32_t>("fixed_point_divide_3", &fixed_point_divide_3<fract_bits>); // trying to avoid 64-bit values while staying in FP
  Test<uint32_t>("fixed_point_divide_4", &fixed_point_divide_4<fract_bits>); // trying to avoid 64-bit values while staying in FP
  Test<uint32_t>("fixed_point_divide_5", &fixed_point_divide_5<fract_bits, 6>); // another impl of normal div with 64-bit interm (11000)
  Test<uint32_t>("fixed_point_divide_6", &fixed_point_divide_6<fract_bits, 6>); // converting to float & back. It's twice as fast as native 64-bit div. (5000)
  Test<uint32_t>("fixed_point_divide_7", &fixed_point_divide_7<fract_bits, 7>); // converting to double & back. it's about half way back to 64 bit impl. (8000)
  
}

void loop() {
  
}
