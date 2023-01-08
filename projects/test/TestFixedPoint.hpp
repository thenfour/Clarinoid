

#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

//// r=(a[3] * b[4]); r >> 7
//// r << (7); r / b
// template <uint8_t fractBitsA, uint8_t fractBitsB>
// uint32_t fixed_point_division(uint32_t a, uint32_t b)
//{
//     auto a2 = uint64_t(a) << (fractBitsB);
//     return a2 / b;
// }
//
//
// template <uint8_t fractBitsA, uint8_t fractBitsB>
// uint32_t fixed_point_division_float(uint32_t a, uint32_t b)
//{
//     auto a2 = float(a) * (1 << fractBitsB);
//     return a2 / b;
// }

void TestFixedPoint()
{
    // tests needed:
    // test when converting a 8.23(int32) to a 8.5 int16, we shift so the left bits are preserved
    {

        Fixed<4, 12> a{-13.6f};
        Fixed<4, 12> ar{-.5f};
        Fixed<2, 14> o{-.5f};
        Fixed<4, 12> b{-.9};
        Fixed<4, 27> r{-1.11111};
        // -4.7
        auto c = a.DivideSlow<6>(b);
        auto d = b.ReciprocalSlow<4>();
        auto e = a.Modulo(o);

        // tests for modulo:
        // https://stackoverflow.com/questions/4003232/how-to-code-a-modulo-operator-in-c-c-obj-c-that-handles-negative-numbers
        auto t1 = Fixed<8, 8>{10.2f}.Modulo(Fixed<10, 10>{2.0f});   // 0.2
        auto t2 = Fixed<8, 8>{10.2f}.Modulo(Fixed<10, 10>{-2.0f});  // -1.8
        auto t3 = Fixed<8, 8>{-10.2f}.Modulo(Fixed<10, 10>{2.0f});  // 1.8
        auto t4 = Fixed<8, 8>{-10.2f}.Modulo(Fixed<10, 10>{-2.0f}); // -0.2

        Fixed<8> g{-8.25};
        auto h = g.Floor();
        auto h2 = g.Fract();

        Fixed<8> j{8.25};
        auto i = j.Floor();
        auto i2 = j.Fract();
    }
    // make signed
    {
        Fixed<8, 8> x{1.25};
        auto y = x.Negate();
        auto z = y.Abs();

        int x__ = 0;
    }

    // floor
    {
        Fixed<8, 8> x{1.25};
        auto y = x.Floor();
        auto x1 = x.Fract();

        auto x2 = x.Add(x);

        int x__ = 0;
    }

    // shift left
    {
        Fixed<8, 4> x{1.2};
        auto x1 = x.ShiftLeft<0>();
        auto x2 = x.ShiftLeft<1>();
        auto x3 = x.ShiftLeft<2>();
        auto x4 = x.ShiftLeft<4>();
        auto x5 = x.ShiftLeft<5>();

        Fixed<28, 4> y{float((1<<27)-1)};
        auto x6 = y.ShiftRight<0>();
        auto x7 = y.ShiftRight<1>();
        auto x8 = y.ShiftRight<2>();
        auto x9 = y.ShiftRight<4>();
        auto xa = y.ShiftRight<5>();
        auto xb = y.ShiftRight<28>();
        auto xc = y.ShiftRight<29>();

        int x__ = 0;
    }
    // compare
    {
        Fixed<1, 3> a{.2f};
        Fixed<2, 5> a1{.2f};
        auto a2 = a.IsGreaterThan(a1);
        auto a3 = a.IsGreaterThanOrEquals(a1);
        auto a4 = a.IsLessThan(a1);
        auto a5 = a.IsLessThanOrEquals(a1);
        auto a6 = a.IsApproximatelyEqualTo(a1);
        
        int x__ = 0;
    }

    // lerp
    {
        Fixed<2> a{1.0};
        Fixed<3> b{3.0};
        auto a6 = Lerp(a, b, .5f);

        auto r1 = Lerp(2.5f, 3.5f, 0.99f);
        auto a7 = Lerp(Fixed<2>{1.5}, Fixed<3>{3.5}, .99);

        auto a8 = Lerp(Fixed<2>{1.5}, Fixed<3>{3.5}, Fixed<16, 16>{.99});

        int x__ = 0;
    }

    int x = 0;

    //
    // auto a = fixed_point_division<16, 8>(0x66664444, 0x200);
    // auto b = fixed_point_division_float<16, 8>(0x66664444, 0x200);

    // auto x = 1;
    //         TestFixed<uint8_t, 5> c;
    //  c.Multiply<4>(c);
    //  Fixed<int8_t, 7> t{0.5};
    //  Fixed<int16_t, 15> m{0.80};
    //  this works only if there's enough headroom
    //  auto n = m.PromotingMultiply(t);

    // int8 / 7
    // int8 / 4
    // intbits, fractbits
    // float a1f, a2f;
    // std::cin >> a1f;
    // std::cin >> a2f;

    // Fixed<int8_t, 2, 5> a1{-2.9}; // -2.9
    // Fixed<uint32_t, 2, 16> a2{3.4}; // 3.4 =  -9.86
    // Fixed<uint16_t, 12> a7{10.36};

    // auto r = FPMul<uint8_t, 5, uint16_t, 12>(a1.mValue, a2.mValue);
    // Fixed<uint16_t, 12> aa = Fixed<uint16_t, 12>::FromFixed(std::get<0>(r));
    // auto ax = a2.Multiply(a1);

    // printf("%f", ax.ToFloat());
}

} // namespace clarinoid
