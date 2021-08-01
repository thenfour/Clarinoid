// benchmarking various fast tanh() implementations from around the net.
// results:
// x=0.00 -> 1119 micros tanhf
// x=0.74 -> 386 micros vox_fasttanh2
// x=16.04 -> 341 micros fastTanh_NIM
// x=16.04 -> 548 micros microtracker_fasttanh
// x=20.73 -> 306 micros discohead_fastTanh   
// x=0.03 -> 632 micros zangtw_fasttanh       
// x=16.79 -> 406 micros fastertanh
// x=1339.42 -> 431 micros arm_cos_f32        

// conclusion: savings is huge considering the number of tanh() calls which are
// made.
// best bang for the buck: vox_fasttanh2
// dirt cheap and probably acceptable: discohead_fastTanh


// https://www.kvraudio.com/forum/viewtopic.php?f=33&t=388650&start=45
inline float vox_fasttanh2(const float x)
{
    const float ax = fabsf(x);
    const float x2 = x * x;

    return (x * (2.45550750702956f + 2.45550750702956f * ax + (0.893229853513558f + 0.821226666969744f * ax) * x2) /
            (2.44506634652299f + (2.44506634652299f + x2) * fabsf(x + 0.814642734961073f * x * ax)));
}

// less accurate, a bit faster than vox_
// https://github.com/ftsf/nimsynth/blob/57d4e56cd0370309a12a0bf902d5d3115539adea/src/core/filter.nim
inline float fastTanh_NIM(float x)
{
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// https://github.com/magnusjonsson/microtracker/blob/86dd19c55601c3054252ea70ea886fb9dbc6f353/plugins/src/shared/fasttanh.c
double microtracker_fasttanh(double x)
{
    double x2 = x * x;
    return x * (27 + x2) / (27 + 9 * x2);
}

// https://github.com/npisanti/ofxPDSP/blob/e106991f4abf4314116d4e7c4ef7ad69d6ca005f/src/math/trig/fasttanh.h
inline float npisanti_fasttanh(float angle)
{

    return angle / (fabsf(2 * angle) + 3 / (2 + 2 * angle * 2 * angle));
}

// https://github.com/discohead/LXR_JCM/blob/14b4b06ce5c9f4a60528d0c2d181f47227ae87df/mainboard/LxrStm32/src/DSPAudio/ResonantFilter.c
inline float discohead_fastTanh(float var)
{
    return 4.15f * var / (4.29f + var * var);
}

static inline float fastpow2(float p)
{
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp = (p < -126) ? -126.0f : p;
    int w = clipp;
    float z = clipp - w + offset;
    union {
        uint32_t i;
        float f;
    } v = {static_cast<uint32_t>((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z))};

    return v.f;
}

static inline float fastexp(float p)
{
    return fastpow2(1.442695040f * p);
}

// https://github.com/zangtw/fastapprox/blob/8cc1dc8d888758f002758a98a393e41e8e43366e/fastapprox/src/fasthyperbolic.h
static inline float zangtw_fasttanh(float p)
{
    return -1.0f + 2.0f / (1.0f + fastexp(-2.0f * p));
}



static inline float
fasterpow2 (float p)
{
  float clipp = (p < -126) ? -126.0f : p;
  union { uint32_t i; float f; } v = { static_cast<uint32_t> ( (1 << 23) * (clipp + 126.94269504f) ) };
  return v.f;
}

static inline float
fasterexp (float p)
{
  return fasterpow2 (1.442695040f * p);
}


static inline float
fastertanh (float p)
{
  return -1.0f + 2.0f / (1.0f + fasterexp (-2.0f * p));
}
















    // 3000 iterations, because for 1 audio block,
    // 128 samples * 2 stereo * 12 voices = 3072 tanh calls at least, per audio block.
    static const int iterations = 3000;
    float comparison[iterations];
    volatile float x = 0.0f;
    for (float i = 0; i < iterations; ++i)
    {
        float f = ::tanhf(i / iterations);
        comparison[(int)i] = f;
        x += f;
    }


    // REAL: 1119 micros for 3000 iterations. it's true this is VERY much a bottleneck. literally half the audio time is spent on this 1 trig call.
    x = 0.0f;
    auto m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - ::tanhf(i / iterations));
    }
    auto m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros tanhf");

    // error 0.74 (very good), 386 micros. So 1/3 the time for excellent result.
    x = 0.0f;
    m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - ::vox_fasttanh2(i / iterations));
    }
    m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros vox_fasttanh2");

    // error 16 (meh) and 358 micros. So worse results for about the same performance as above.
    x = 0.0f;
    m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - ::fastTanh_NIM(i / iterations));
    }
    m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros fastTanh_NIM");

    // error 16 again, but 531 micros. Nope.
    x = 0.0f;
    m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - ::microtracker_fasttanh(i / iterations));
    }
    m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros microtracker_fasttanh");

    // error 20, so even worse results. but 300 micros so this is blazing fast.
    x = 0.0f;
    m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - ::discohead_fastTanh(i / iterations));
    }
    m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros discohead_fastTanh");

    // error 0.03, most accurate. And 632 micros so 1/2 processing, but i think it's probably not justified.
    x = 0.0f;
    m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - ::zangtw_fasttanh(i / iterations));
    }
    m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros zangtw_fasttanh");


    // error 16 (meh), and 400 micros. We have better elsewhere.
    x = 0.0f;
    m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - ::fastertanh(i / iterations));
    }
    m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros fastertanh");

    // just to prove what a totally incorrect function is like.
    // error 1339, 583 micros.
    x = 0.0f;
    m1 = micros();
    for (float i = 0; i < iterations; ++i)
    {
        x += fabsf(comparison[(int)i] - arm_cos_f32(i / iterations));
    }
    m2 = micros();
    Serial.print(String("x=") + x);
    Serial.println(String(" -> ") + (m2 - m1) + " micros arm_cos_f32");
