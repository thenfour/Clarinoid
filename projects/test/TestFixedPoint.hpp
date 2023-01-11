

#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

void TestFixedPoint()
{
    //{

    //    Fixed<4, 12> a{-13.6f};
    //    Fixed<4, 12> ar{-.5f};
    //    Fixed<2, 14> o{-.5f};
    //    Fixed<4, 12> b{-.9};
    //    Fixed<4, 27> r{-1.11111};
    //    // -4.7
    //    auto c = a.DivideSlow<6>(b);
    //    auto d = b.ReciprocalSlow<4>();
    //    auto e = a.Modulo(o);

    //    // tests for modulo:
    //    //
    //    https://stackoverflow.com/questions/4003232/how-to-code-a-modulo-operator-in-c-c-obj-c-that-handles-negative-numbers
    //    auto t1 = Fixed<8, 8>{10.2f}.Modulo(Fixed<10, 10>{2.0f});   // 0.2
    //    auto t2 = Fixed<8, 8>{10.2f}.Modulo(Fixed<10, 10>{-2.0f});  // -1.8
    //    auto t3 = Fixed<8, 8>{-10.2f}.Modulo(Fixed<10, 10>{2.0f});  // 1.8
    //    auto t4 = Fixed<8, 8>{-10.2f}.Modulo(Fixed<10, 10>{-2.0f}); // -0.2

    //    Fixed<8> g{-8.25};
    //    auto h = g.Floor();
    //    auto h2 = g.Fract();

    //    Fixed<8> j{8.25};
    //    auto i = j.Floor();
    //    auto i2 = j.Fract();
    //}
    //// make signed
    //{
    //    Fixed<8, 8> x{1.25};
    //    auto y = x.Negate();
    //    auto z = y.Abs();

    //    int x__ = 0;
    //}

    {
        auto a1 = FixedInteger<1>();
        auto a2 = FixedInteger<0>();
        auto a3 = FixedInteger<-1>();
        auto a4 = FixedInteger<15>();
        auto a5 = FixedInteger<16>();
        auto a6 = FixedInteger<-15>();
        auto a7 = FixedInteger<-16>();

        int x__ = 0;
    }

    // floor
    {
        Fixed<4, 4, uint8_t> x{1.25};
        Test(x.Floor().IsApproximatelyEqualTo(1));
        Test(FixedInteger<14>().Floor().IsApproximatelyEqualTo(14));
        Test(Fixed<4>(14.55).Floor().IsApproximatelyEqualTo(14));
        Test(!Fixed<4>(15.55).Floor().IsApproximatelyEqualTo(14));
        Test(!Fixed<4>(4.55).Floor().IsApproximatelyEqualTo(14));

        Test((Fixed<5, 4, int16_t>{21.25}.Floor().IsApproximatelyEqualTo(21)));
        Test((Fixed<5, 4, int16_t>{-21.25}.Floor().IsApproximatelyEqualTo(-22)));

        Test((Fixed<5, 26, int32_t>{21.2}.Floor().IsApproximatelyEqualTo(21)));
        Test((Fixed<5, 26, int32_t>{-21.2}.Floor().IsApproximatelyEqualTo(-22)));

        Test((Fixed<5, 26 + 32, int64_t>{31.2}.Floor().IsApproximatelyEqualTo(31)));
        Test((Fixed<6, 25 + 32, int64_t>{-31.2}.Floor().IsApproximatelyEqualTo(-32)));

        Test((Fixed<5, 27 + 32, uint64_t>{31.2}.Floor().IsApproximatelyEqualTo(31)));

        int x__ = 0;
    }

    // fract
    {
        Test((Fixed<5, 4, int16_t>{21.25}.Fract().IsApproximatelyEqualTo(0.25)));
        Test((Fixed<5, 4, int16_t>{-21.25}.Fract().IsApproximatelyEqualTo(0.75)));

        Test((Fixed<5, 10, int16_t>{21.25}.Fract().IsApproximatelyEqualTo(0.25)));
        Test((Fixed<5, 10, int16_t>{-21.25}.Fract().IsApproximatelyEqualTo(0.75)));

        Test((Fixed<5, 11, uint16_t>{21.25}.Fract().IsApproximatelyEqualTo(0.25)));
        Test((Fixed<5, 11, uint16_t>{-21.25}.Fract().IsApproximatelyEqualTo(0.75)));

        Test((Fixed<0, 32>{0.25}.Fract().IsApproximatelyEqualTo(0.25)));
        Test((Fixed<0, 32>{0.99}.Fract().IsApproximatelyEqualTo(0.99)));

        Test((Fixed<0, 8, uint8_t>{0.25}.Fract().IsApproximatelyEqualTo(0.25)));

        auto a = Fixed<0, 7, int8_t>{-0.99};
        auto a1 = Fixed<0, 7, int8_t>::FromFixed(0x7f);
        auto a2 = Fixed<0, 7, int8_t>::FromFixed(0x01);
        auto b = a.Fract();
        Test(b.IsApproximatelyEqualTo(0.01));

        Test((Fixed<0, 15, int16_t>{0.1212}.Fract().IsApproximatelyEqualTo(0.1212)));
        Test((Fixed<0, 15, int16_t>{-0.1212}.Fract().IsApproximatelyEqualTo(0.8788)));

        Test((Fixed<0, 31, int32_t>{0.1212}.Fract().IsApproximatelyEqualTo(0.1212)));
        Test((Fixed<0, 31, int32_t>{-0.1212}.Fract().IsApproximatelyEqualTo(0.8788)));
        Test((Fixed<0, 32, uint32_t>{0.1212}.Fract().IsApproximatelyEqualTo(0.1212)));

        Test((Fixed<0, 63, int64_t>{0.1212}.Fract().IsApproximatelyEqualTo(0.1212)));
        Test((Fixed<0, 63, int64_t>{-0.1212}.Fract().IsApproximatelyEqualTo(0.8788)));
        Test((Fixed<0, 64, uint64_t>{0.1212}.Fract().IsApproximatelyEqualTo(0.1212)));

        int x__ = 0;
    }

    {
        // test integer division
        auto x1 = Fixed<4, 9, uint32_t>{13.5};
        auto x2 = Fixed<3, 3, uint16_t>{2.5};
        auto x3 = x1.DivideFast(x2);
        Test(x3.IsApproximatelyEqualTo(5.4));

        auto x11 = Fixed<4, 54, uint64_t>{13.5};
        auto x21 = Fixed<3, 3, uint16_t>{2.5};
        auto x31 = x11.DivideFast(x21);
        Test(x31.IsApproximatelyEqualTo(5.4));

        Test(FixedInteger<9>().DivideFast(FixedInteger<2,2>()).IsApproximatelyEqualTo(4.5));
        int x__ = 0;
    }

    //// shift left
    //{
    //    Fixed<8, 4> x{1.2};
    //    auto x1 = x.ShiftLeft<0>();
    //    auto x2 = x.ShiftLeft<1>();
    //    auto x3 = x.ShiftLeft<2>();
    //    auto x4 = x.ShiftLeft<4>();
    //    auto x5 = x.ShiftLeft<5>();

    //    Fixed<28, 4> y{float((1<<27)-1)};
    //    auto x6 = y.ShiftRight<0>();
    //    auto x7 = y.ShiftRight<1>();
    //    auto x8 = y.ShiftRight<2>();
    //    auto x9 = y.ShiftRight<4>();
    //    auto xa = y.ShiftRight<5>();
    //    auto xb = y.ShiftRight<28>();
    //    auto xc = y.ShiftRight<29>();

    //    int x__ = 0;
    //}
    //// compare
    //{
    //    Fixed<1, 3> a{.2f};
    //    Fixed<2, 5> a1{.2f};
    //    auto a2 = a.IsGreaterThan(a1);
    //    auto a3 = a.IsGreaterThanOrEquals(a1);
    //    auto a4 = a.IsLessThan(a1);
    //    auto a5 = a.IsLessThanOrEquals(a1);
    //    auto a6 = a.IsApproximatelyEqualTo(a1);
    //
    //    int x__ = 0;
    //}

    //// lerp
    //{
    //    Fixed<2> a{1.0};
    //    Fixed<3> b{3.0};
    //    auto a6 = Lerp(a, b, .5f);

    //    auto r1 = Lerp(2.5f, 3.5f, 0.99f);
    //    auto a7 = Lerp(Fixed<2>{1.5}, Fixed<3>{3.5}, .99);

    //    auto a8 = Lerp(Fixed<2>{1.5}, Fixed<3>{3.5}, Fixed<16, 16>{.99});

    //    int x__ = 0;
    //}

    //// clamp
    //{
    //    Fixed<15, 15> a{13.56};

    //    auto b = a.Clamp(FixedInteger<0>(), FixedInteger<1>());
    //    auto b1 = Fixed<15, 15>{-1.5}.Clamp(FixedInteger<0>(), FixedInteger<1>());
    //    auto b2 = Fixed<15, 15>{-0.5}.Clamp(FixedInteger<0>(), FixedInteger<1>());
    //    auto b3 = Fixed<15, 15>{0.0f}.Clamp(FixedInteger<0>(), FixedInteger<1>());
    //    auto b4 = Fixed<15, 15>{0.5}.Clamp(FixedInteger<0>(), FixedInteger<1>());
    //    auto b5 = Fixed<15, 15>{1.0f}.Clamp(FixedInteger<0>(), FixedInteger<1>());
    //    auto b6 = Fixed<15, 15>{1.5}.Clamp(FixedInteger<0>(), FixedInteger<1>());

    //    int x__ = 0;
    //}

    //// clamp
    //{
    //    auto a1 = FixedInteger<0>();
    //    auto a2 = FixedInteger<1>();
    //    auto a3 = FixedInteger<-1>();
    //    auto a4 = FixedInteger<-255>();
    //    auto a5 = FixedInteger<-256>();
    //    auto a6 = FixedInteger<255>();
    //    auto a7 = FixedInteger<256>();
    //    auto a8 = FixedInteger<2>();

    //    int x__ = 0;
    //}

    //// saturate
    //{
    //    auto a1 = Fixed<2>(-2.50);
    //    auto a2 = a1.SignedSaturate<1, 14>();

    //    auto a3 = Fixed<2>(2.50);
    //    auto a4 = a3.UnsignedSaturate<1, 14>();

    //    int x__ = 0;
    //}

    //// sqrt
    //{
    //    auto a3 = Fixed<0, 32>(.01);
    //    auto a3r = Fixed<0, 32>(sqrtf(a3.ToFloat()));
    //    //auto a4 = sqrt(a3);

    //    int x__ = 0;
    //}

    //// sine
    //{
    //    auto a3 = Fixed<0, 32>(.01);
    //    auto a1 = a3.Sine_2pi();
    //    auto a2 = Fixed<0, 22>(0.0).Sine_2pi();
    //    auto a4 = Fixed<0, 23>(.25).Sine_2pi();
    //    auto a5 = Fixed<4, 24>(4.5).Sine_2pi();
    //    auto a6 = Fixed<3, 25>(.75).Sine_2pi();
    //    auto a7 = Fixed<0, 15>(1.0).Sine_2pi();

    //    auto a21 = Fixed<0, 32>(0.0).Sine_2pi();
    //    auto a4x = Fixed<0, 32>(.66).Sine_2pi();
    //    auto a41 = Fixed<0, 32>(.25).Sine_2pi();
    //    auto a51 = Fixed<0, 32>(.5).Sine_2pi();
    //    auto a61 = Fixed<0, 32>(.75).Sine_2pi();
    //    auto a71 = Fixed<0, 32>(1.0).Sine_2pi();
    //    int x__ = 0;
    //}
}

} // namespace clarinoid
