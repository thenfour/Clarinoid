
// arduino-ish libraries
#define ENCODER_DO_NOT_USE_INTERRUPTS
#define AUDIO_BLOCK_SAMPLES 128 // other values are currently producing artifacts.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <Arduino.h>

#include <Wire.h>
#include <EasyTransfer.h>
#include <FastCRC.h>
#include <Bounce.h>
#include <SPI.h>
#include <MIDI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Audio.h>
#include <Encoder.h>
#include <WS2812Serial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_MCP23017.h>

#pragma GCC diagnostic pop

#include "testDeviceApp.hpp"

void ModCurveMain()
{
    while (!Serial)
    {
    } // when you are debugging with serial, uncomment this to ensure you see startup msgs
    Serial.println("starting normally....");

    clarinoid::Stopwatch sw;
    srand(1500);

    clarinoid::CCEncoder<4 /* step increment each increment */, 14 /* pin #1 */, 15 /* pin #2 */> encoder;
    CCAdafruitSSD1306 display = {128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL};
    display.begin(SSD1306_SWITCHCAPVCC);

    float k = 0.5f;
    static constexpr int graphWidth = 62;
    static constexpr int graphHeight = 62;

    decltype(gModCurveLUT) &lut = gModCurveLUT;

    int16_t q15[128];
    static constexpr size_t iterations = 70;
    sw.Pause();
    sw.Restart();
    sw.Pause();
    for (size_t i = 0; i < iterations; ++i)
    {
        for (int16_t &q : q15)
        {
            q = rand() - RAND_MAX / 2;
        }
        float k = ((float(rand()) / RAND_MAX) * 2) - 1;
        sw.Unpause();
        auto state = lut.BeginLookupF(k);
        for (int16_t &q : q15)
        {
            q += lut.Transfer16(q, state);
        }
        sw.Pause();
    }
    Serial.println(String("micros: ") + (int)sw.ElapsedTime().ElapsedMicros());

    while (true)
    {
        display.clearDisplay();
        display.drawRect(0, 0, graphWidth + 3, graphHeight + 3, SSD1306_WHITE);
        // display.DrawDottedRect(0, 0, graphWidth + 3, graphHeight + 3, SSD1306_WHITE);
        // display.fillRect(1, 1, graphWidth, graphHeight, SSD1306_BLACK);

        delay(5);
        encoder.Update();
        k = clarinoid::Frac(0.5 + (encoder.CurrentValue() / 25)) * 2 - 1;
        display.setCursor(127 - 7 * 8, 0);
        display.setTextColor(SSD1306_WHITE);
        display.print(String("k=") + k);

        auto lookupState = lut.BeginLookupF(k);

        display.setCursor(127 - 8 * 8, 8);
        display.println(String("1=") + lut.Transfer16(32767, lookupState));
        display.setCursor(127 - 8 * 8, 16);
        display.println(String("0=") + lut.Transfer16(0, lookupState));
        display.setCursor(127 - 8 * 8, 24);
        display.println(String("-1=") + lut.Transfer16(-32768, lookupState));

        for (size_t ix = 1; ix < graphWidth + 1; ++ix)
        {
            // static line
            // int16_t s1 = float(ix - 1) * 7 / graphWidth)) * 32767;
            int16_t s1 = ((float(ix - 1) / graphWidth) * 2 - 1) * 32767;
            int16_t s2 = ((float(ix) / graphWidth) * 2 - 1) * 32767;

            // moving sine
            // int16_t s1 = ::sinf(float(millis()) / 1000 + (float(ix - 1) * 7 / graphWidth)) * 32767;
            // int16_t s2 = ::sinf(float(millis()) / 1000 + (float(ix) * 7 / graphWidth)) * 32767;
            int y1x = ((float(lut.Transfer16(s1, lookupState)) / 32767) * .5 + .5) * graphHeight;
            int y2x = ((float(lut.Transfer16(s2, lookupState)) / 32767) * .5 + .5) * graphHeight;
            display.DrawDottedVLineWithGlobalParity(ix, graphHeight - y1x + 1, y1x, false);
            display.drawLine(ix, graphHeight - y1x, ix + 1, graphHeight - y2x, SSD1306_WHITE);
        }

        display.display();
    }
}

void RandomizeBuffer(int16_t *buf)
{
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
    {
        buf[i] = rand() - RAND_MAX / 2;
    }
}

int32_t ConvertPolarity(int32_t x,
                        clarinoid::ModulationPoleType inpPolarity,
                        clarinoid::ModulationPolarityTreatment outpPolarity)
{
    if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
        outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolar)
    {
        return (x - .5) * 2; // or x*2-1
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01)
    {
        return x * .5 + .5;
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01Inverted)
    {
        return 1 - x; // or x*-1+1
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01Inverted)
    {
        return .5 - x * .5; // or x*-.5+.5
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolarInverted)
    {
        return .5 - x * 2; // or x*-2+.5
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolarInverted)
    {
        return -x; // x*-1+0
    }
    return x; // x*1+0
}

static constexpr int32_t To15p16(int32_t num, int32_t den)
{
    return (int32_t(num) << 16) / den;
};

static constexpr int16_t ToSample16(int32_t num, int32_t den)
{
    return int16_t((int32_t(num) * 32767) / den); // or should i use 32768 and saturate? not sure.
};

// specifies params to use in x = x * mul + add
struct PolarityConversionValues15p16
{
    const int32_t mul; // is 15.16
    const int16_t add; // is NOT 15.16. just an integer.
};

PolarityConversionValues15p16 GetPolarityConversion15p16(clarinoid::ModulationPoleType inpPolarity,
                                                         clarinoid::ModulationPolarityTreatment outpPolarity)
{
    if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
        outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolar)
    {
        // return (x - .5) * 2; // or x*2-1
        // return {To15p16(2, 1), To15p16(-1, 1)};
        return {To15p16(2, 1), ToSample16(-1, 1)};
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01)
    {
        // return x * .5 + .5;
        return {To15p16(1, 2), ToSample16(1, 2)};
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01Inverted)
    {
        // return 1 - x; // or x*-1+1
        return {To15p16(-1, 1), ToSample16(1, 1)};
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01Inverted)
    {
        // return .5 - x * .5; // or x*-.5+.5
        return {To15p16(-1, 2), ToSample16(1, 2)};
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolarInverted)
    {
        // return .5 - x * 2; // or x*-2+.5
        return {To15p16(-2, 1), ToSample16(1, 2)};
    }
    else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
             outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolarInverted)
    {
        // return -x; // x*-1+0
        return {To15p16(-1, 1), ToSample16(0, 1)};
    }
    // return x; // x*1+0
    return {To15p16(1, 1), ToSample16(0, 1)};
}

void ModBenchmarkMain()
{
    int16_t bufSrc[AUDIO_BLOCK_SAMPLES];
    int16_t bufAux[AUDIO_BLOCK_SAMPLES];
    int16_t bufTemp[AUDIO_BLOCK_SAMPLES];
    int16_t bufDest[AUDIO_BLOCK_SAMPLES];

    while (!Serial)
    {
    } // when you are debugging with serial, uncomment this to ensure you see startup msgs
    Serial.println("starting normally....");

    clarinoid::Stopwatch sw;
    sw.Pause();

    clarinoid::SynthModulationSpec spec;
    spec.mSource = clarinoid::AnyModulationSource::LFO1;
    spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
    spec.mScaleN11 = 0.5f;
    spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
    spec.mCurveShape = 12; // integral, because we don't interpolate; mod curves are actually discrete. it also
                           // simplifies the "0" case where it's linear.
    spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
    spec.mAuxAmount01 = 0.3f; // amount of attenuation
    spec.mAuxEnabled = true;  // just allows bypassing without removing the aux source
    spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
    spec.mAuxCurveShape = -25;

    static constexpr size_t iterations = 70;

    {
        srand(1500);
        sw.PauseAndReset();
        for (size_t i = 0; i < iterations; ++i)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            arm_scale_q15(bufSrc, clarinoid::fast::Sample32To16(spec.mScaleN11), 0, bufTemp, AUDIO_BLOCK_SAMPLES);
            arm_add_q15(bufTemp, bufDest, bufDest, AUDIO_BLOCK_SAMPLES);
            sw.Pause();
        }
        Serial.println(String("scale only micros: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        srand(1500);
        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            float kSrc = ((float(rand()) / RAND_MAX) * 2) - 1;
            float kAux = ((float(rand()) / RAND_MAX) * 2) - 1;
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupF(kSrc);
            auto auxCurveState = gModCurveLUT.BeginLookupF(kAux);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScaleQ31 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScaleQ31 = int32_t(spec.mAuxAmount01 * 65536);

            for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
            {
                int32_t x = bufSrc[i];
                x = ConvertPolarity(x, modSrcInfo.mPoleType, spec.mSourcePolarity);
                x = gModCurveLUT.Transfer16(x, srcCurveState);
                x = (x * sourceScaleQ31) >> 16;

                int32_t aux = bufAux[i];
                aux = ConvertPolarity(aux, modAuxInfo.mPoleType, spec.mAuxPolarity);
                aux = gModCurveLUT.Transfer16(aux, auxCurveState);
                aux = (aux * auxScaleQ31); // aux is now a 16.16 fixed point
                aux = 32767 - aux;         // effectively 1-aux

                x = (x * aux) >> 16;

                bufDest[i] += x; // TODO: multiply & saturate
            }
            sw.Pause();
        }
        Serial.println(String("full treatment, naive micros: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        srand(1500);
        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            float kSrc = ((float(rand()) / RAND_MAX) * 2) - 1;
            float kAux = ((float(rand()) / RAND_MAX) * 2) - 1;
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupF(kSrc);
            auto auxCurveState = gModCurveLUT.BeginLookupF(kAux);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScaleQ31 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScaleQ31 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
            {
                int32_t x = bufSrc[i];
                x = x * srcPolarityConv.mul + srcPolarityConv.add;
                x = gModCurveLUT.Transfer16(x, srcCurveState);
                x = (x * sourceScaleQ31) >> 16;

                int32_t aux = bufAux[i];
                aux = aux * auxPolarityConv.mul;
                aux >>= 16;
                aux += auxPolarityConv.add;
                aux = gModCurveLUT.Transfer16(aux, auxCurveState);
                aux = (aux * auxScaleQ31); // aux is now a 16.16 fixed point
                aux = 32767 - aux;         // effectively 1-aux

                x = (x * aux) >> 16;

                bufDest[i] += x; // TODO: multiply & saturate
            }
            sw.Pause();
        }
        Serial.println(String("full, optimizing convertpolarity: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        srand(1500);

        spec.mSource = clarinoid::AnyModulationSource::LFO1;
        spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
        spec.mScaleN11 = 0.5f;
        spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mCurveShape = 12; // integral, because we don't interpolate; mod curves are actually discrete. it also
                               // simplifies the "0" case where it's linear.
        spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
        spec.mAuxAmount01 = 0.3f; // amount of attenuation
        spec.mAuxEnabled = true;  // just allows bypassing without removing the aux source
        spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mAuxCurveShape = -25;

        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupI(spec.mCurveShape);
            auto auxCurveState = gModCurveLUT.BeginLookupI(spec.mAuxCurveShape);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScale16p16 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScale16p16 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            uint32_t *bufSrc32 = (uint32_t *)bufSrc;
            uint32_t *bufAux32 = (uint32_t *)bufAux;
            uint32_t *bufDest32 = (uint32_t *)bufDest;

            for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
            {
                uint32_t x32 = bufSrc32[i32];

                int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
                int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
                x1 = x1 + srcPolarityConv.add;
                x2 = x2 + srcPolarityConv.add;
                x1 = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
                x2 = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);
                x1 = (x1 * sourceScale16p16) >> 16;
                x2 = (x2 * sourceScale16p16) >> 16;

                if (spec.mAuxEnabled && spec.mAuxSource != clarinoid::AnyModulationSource::None)
                {
                    int32_t aux32 = bufAux32[i32];
                    int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
                    int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
                    aux1 += auxPolarityConv.add;
                    aux2 += auxPolarityConv.add;
                    aux1 = gModCurveLUT.Transfer16(saturate16(aux1), auxCurveState);
                    aux2 = gModCurveLUT.Transfer16(saturate16(aux2), auxCurveState);
                    aux1 = (aux1 * auxScale16p16) >> 16;
                    aux2 = (aux2 * auxScale16p16) >> 16;
                    x1 = (x1 * aux1) >> 16; 
                    x2 = (x2 * aux2) >> 16;
                }

                x32 = pack_16b_16b(x1, x2);

                bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
            }
            sw.Pause();
        }
        Serial.println(String("full, optimized polarity & dual-sample processing: ") +
                       (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        srand(1500);

        spec.mSource = clarinoid::AnyModulationSource::LFO1;
        spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
        spec.mScaleN11 = 0.5f;
        spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mCurveShape = 12; // integral, because we don't interpolate; mod curves are actually discrete. it also
                               // simplifies the "0" case where it's linear.
        spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
        spec.mAuxAmount01 = 0.3f; // amount of attenuation
        spec.mAuxEnabled = true;  // just allows bypassing without removing the aux source
        spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mAuxCurveShape = -25;

        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupI(spec.mCurveShape);
            auto auxCurveState = gModCurveLUT.BeginLookupI(spec.mAuxCurveShape);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScale16p16 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScale16p16 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            uint32_t *bufSrc32 = (uint32_t *)bufSrc;
            uint32_t *bufAux32 = (uint32_t *)bufAux;
            uint32_t *bufDest32 = (uint32_t *)bufDest;

            for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
            {
                uint32_t x32 = bufSrc32[i32];

                int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
                int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
                x1 = x1 + srcPolarityConv.add;
                x2 = x2 + srcPolarityConv.add;
                x1 = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
                x2 = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);
                x1 = (x1 * sourceScale16p16) >> 16;
                x2 = (x2 * sourceScale16p16) >> 16;

                if (spec.mAuxEnabled && spec.mAuxSource != clarinoid::AnyModulationSource::None)
                {
                    int32_t aux32 = bufAux32[i32];
                    int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
                    int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
                    aux1 += auxPolarityConv.add;
                    aux2 += auxPolarityConv.add;
                    aux1 = gModCurveLUT.Transfer16(saturate16(aux1), auxCurveState);
                    aux2 = gModCurveLUT.Transfer16(saturate16(aux2), auxCurveState);
                    aux1 = (aux1 * auxScale16p16) >> 16;
                    aux2 = (aux2 * auxScale16p16) >> 16;

                    x1 = (x1 * aux1) >> 16;
                    x2 = (x2 * aux2) >> 16;
                }

                x32 = pack_16b_16b(x1, x2);

                bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
            }
            sw.Pause();
        }
        Serial.println(String("full, optimized polarity & dual-sample, manual mulshifts: ") +
                       (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        srand(1500);

        spec.mSource = clarinoid::AnyModulationSource::LFO1;
        spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
        spec.mScaleN11 = 0.5f;
        spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mCurveShape = 12; // integral, because we don't interpolate; mod curves are actually discrete. it also
                               // simplifies the "0" case where it's linear.
        spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
        spec.mAuxAmount01 = 0.3f; // amount of attenuation
        spec.mAuxEnabled = true;  // just allows bypassing without removing the aux source
        spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mAuxCurveShape = -25;

        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupI(spec.mCurveShape);
            auto auxCurveState = gModCurveLUT.BeginLookupI(spec.mAuxCurveShape);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScale16p16 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScale16p16 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            uint32_t *bufSrc32 = (uint32_t *)bufSrc;
            uint32_t *bufAux32 = (uint32_t *)bufAux;
            uint32_t *bufDest32 = (uint32_t *)bufDest;

            for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
            {
                uint32_t x32 = bufSrc32[i32];

                int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
                int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
                x1 = x1 + srcPolarityConv.add;
                x2 = x2 + srcPolarityConv.add;
                x1 = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
                x2 = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);

                x1 = signed_multiply_32x16b(sourceScale16p16, x1);
                x2 = signed_multiply_32x16b(sourceScale16p16, x2);

                if (spec.mAuxEnabled && spec.mAuxSource != clarinoid::AnyModulationSource::None)
                {
                    int32_t aux32 = bufAux32[i32];
                    int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
                    int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
                    aux1 += auxPolarityConv.add;
                    aux2 += auxPolarityConv.add;
                    aux1 = gModCurveLUT.Transfer16(saturate16(aux1), auxCurveState);
                    aux2 = gModCurveLUT.Transfer16(saturate16(aux2), auxCurveState);
                    aux1 = signed_multiply_32x16b(auxScale16p16, aux1);
                    aux2 = signed_multiply_32x16b(auxScale16p16, aux2);
                    x1 = signed_multiply_32x16b(aux1, x1);
                    x2 = signed_multiply_32x16b(aux2, x2);
                }

                x32 = pack_16b_16b(x1, x2);

                bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
            }
            sw.Pause();
        }
        Serial.println(String("same but with optimized mulshifts: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }




    {
        srand(1500);

        spec.mSource = clarinoid::AnyModulationSource::LFO1;
        spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
        spec.mScaleN11 = 0.5f;
        spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mCurveShape = 12; // integral, because we don't interpolate; mod curves are actually discrete. it also
                               // simplifies the "0" case where it's linear.
        spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
        spec.mAuxAmount01 = 0.3f; // amount of attenuation
        spec.mAuxEnabled = true;  // just allows bypassing without removing the aux source
        spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mAuxCurveShape = -25;

        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupI(spec.mCurveShape);
            auto auxCurveState = gModCurveLUT.BeginLookupI(spec.mAuxCurveShape);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScale16p16 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScale16p16 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            uint32_t *bufSrc32 = (uint32_t *)bufSrc;
            uint32_t *bufAux32 = (uint32_t *)bufAux;
            uint32_t *bufDest32 = (uint32_t *)bufDest;

            for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
            {
                uint32_t x32 = bufSrc32[i32]; // process 2 16-bit samples per loop to take advantage of 32-bit processing

                // this ordering is much faster than processing all x1 first then x2
                int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
                int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
                x1 = x1 + srcPolarityConv.add;
                x2 = x2 + srcPolarityConv.add;
                x1 = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
                x2 = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);
                x1 = signed_multiply_32x16b(sourceScale16p16, x1);
                x2 = signed_multiply_32x16b(sourceScale16p16, x2);

                if (spec.mAuxEnabled && spec.mAuxSource != clarinoid::AnyModulationSource::None)
                {
                    int32_t aux32 = bufAux32[i32];
                    int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
                    int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
                    aux1 += auxPolarityConv.add;
                    aux2 += auxPolarityConv.add;
                    aux1 = gModCurveLUT.Transfer16(saturate16(aux1), auxCurveState);
                    aux2 = gModCurveLUT.Transfer16(saturate16(aux2), auxCurveState);
                    aux1 = signed_multiply_32x16b(auxScale16p16, aux1);
                    aux2 = signed_multiply_32x16b(auxScale16p16, aux2);
                    x1 = signed_multiply_32x16b(aux1, x1);
                    x2 = signed_multiply_32x16b(aux2, x2);
                }

                x32 = pack_16b_16b(x1, x2);
                bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
            }
            sw.Pause();
        }
        Serial.println(String("same with reordered ops: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }



    {
        srand(1500);

        spec.mSource = clarinoid::AnyModulationSource::LFO1;
        spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
        spec.mScaleN11 = 0.5f;
        spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mCurveShape = 12; // integral, because we don't interpolate; mod curves are actually discrete. it also
                               // simplifies the "0" case where it's linear.
        spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
        spec.mAuxAmount01 = 0.3f; // amount of attenuation
        spec.mAuxEnabled = true;  // just allows bypassing without removing the aux source
        spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mAuxCurveShape = -25;

        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupI(spec.mCurveShape);
            auto auxCurveState = gModCurveLUT.BeginLookupI(spec.mAuxCurveShape);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScale16p16 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScale16p16 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            uint32_t *bufSrc32 = (uint32_t *)bufSrc;
            uint32_t *bufAux32 = (uint32_t *)bufAux;
            uint32_t *bufDest32 = (uint32_t *)bufDest;

            for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
            {
                uint32_t x32 = bufSrc32[i32]; // process 2 16-bit samples per loop to take advantage of 32-bit processing

                // this ordering is much faster than processing all x1 first then x2
                int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
                int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
                x1 = x1 + srcPolarityConv.add;
                x2 = x2 + srcPolarityConv.add;
                int16_t x1b = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
                int16_t x2b = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);
                x1 = signed_multiply_32x16b(sourceScale16p16, x1b);
                x2 = signed_multiply_32x16b(sourceScale16p16, x2b);

                if (spec.mAuxEnabled && spec.mAuxSource != clarinoid::AnyModulationSource::None)
                {
                    int32_t aux32 = bufAux32[i32];
                    int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
                    int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
                    aux1 += auxPolarityConv.add;
                    aux2 += auxPolarityConv.add;
                    int16_t aux1b = gModCurveLUT.Transfer16(saturate16(aux1), auxCurveState);
                    int16_t aux2b = gModCurveLUT.Transfer16(saturate16(aux2), auxCurveState);
                    aux1 = signed_multiply_32x16b(auxScale16p16, aux1b);
                    aux2 = signed_multiply_32x16b(auxScale16p16, aux2b);
                    x1 = signed_multiply_32x16b(aux1, x1);
                    x2 = signed_multiply_32x16b(aux2, x2);
                }

                x32 = pack_16b_16b(x1, x2);
                bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
            }
            sw.Pause();
        }
        Serial.println(String("same with int16_t: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }



    {
        srand(1500);

        spec.mSource = clarinoid::AnyModulationSource::LFO1;
        spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
        spec.mScaleN11 = 0.5f;
        spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mCurveShape = gModCurveLUT.LutSizeY / 2;
        spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
        spec.mAuxAmount01 = 0.3f; // amount of attenuation
        spec.mAuxEnabled = true;  // just allows bypassing without removing the aux source
        spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mAuxCurveShape = gModCurveLUT.LutSizeY / 2;

        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupI(spec.mCurveShape);
            auto auxCurveState = gModCurveLUT.BeginLookupI(spec.mAuxCurveShape);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScale16p16 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScale16p16 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            uint32_t *bufSrc32 = (uint32_t *)bufSrc;
            uint32_t *bufAux32 = (uint32_t *)bufAux;
            uint32_t *bufDest32 = (uint32_t *)bufDest;

            for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
            {
                uint32_t x32 = bufSrc32[i32]; // process 2 16-bit samples per loop to take advantage of 32-bit processing

                // this ordering is much faster than processing all x1 first then x2
                int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
                int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
                x1 = x1 + srcPolarityConv.add;
                x2 = x2 + srcPolarityConv.add;
                x1 = gModCurveLUT.Transfer16(x1, srcCurveState);
                x2 = gModCurveLUT.Transfer16(x2, srcCurveState);
                x1 = signed_multiply_32x16b(sourceScale16p16, x1);
                x2 = signed_multiply_32x16b(sourceScale16p16, x2);

                if (spec.mAuxEnabled && spec.mAuxSource != clarinoid::AnyModulationSource::None)
                {
                    int32_t aux32 = bufAux32[i32];
                    int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
                    int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
                    aux1 += auxPolarityConv.add;
                    aux2 += auxPolarityConv.add;
                    aux1 = gModCurveLUT.Transfer16(aux1, auxCurveState);
                    aux2 = gModCurveLUT.Transfer16(aux2, auxCurveState);
                    aux1 = signed_multiply_32x16b(auxScale16p16, aux1);
                    x1 = signed_multiply_32x16b(aux1, x1);
                    aux2 = signed_multiply_32x16b(auxScale16p16, aux2);
                    x2 = signed_multiply_32x16b(aux2, x2);
                }

                x32 = pack_16b_16b(x1, x2);
                bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
            }
            sw.Pause();
        }
        Serial.println(String("no curves: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }




    {
        srand(1500);

        spec.mSource = clarinoid::AnyModulationSource::LFO1;
        spec.mDest = clarinoid::AnyModulationDestination::Osc1Frequency;
        spec.mScaleN11 = 0.5f;
        spec.mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mCurveShape = gModCurveLUT.LutSizeY / 2;
        spec.mAuxSource = clarinoid::AnyModulationSource::Breath;
        spec.mAuxAmount01 = 0.3f; // amount of attenuation
        spec.mAuxEnabled = false;  // just allows bypassing without removing the aux source
        spec.mAuxPolarity = clarinoid::ModulationPolarityTreatment::AsPositive01;
        spec.mAuxCurveShape = 12;

        sw.PauseAndReset();
        for (size_t iter = 0; iter < iterations; ++iter)
        {
            RandomizeBuffer(bufSrc);
            RandomizeBuffer(bufAux);
            RandomizeBuffer(bufDest);
            sw.Unpause();
            auto srcCurveState = gModCurveLUT.BeginLookupI(spec.mCurveShape);
            auto auxCurveState = gModCurveLUT.BeginLookupI(spec.mAuxCurveShape);
            const auto &modSrcInfo = clarinoid::GetModulationSourceInfo(spec.mSource);
            const auto &modAuxInfo = clarinoid::GetModulationSourceInfo(spec.mAuxSource);
            int32_t sourceScale16p16 = int32_t(spec.mScaleN11 * 65536);
            int32_t auxScale16p16 = int32_t(spec.mAuxAmount01 * 65536);

            auto srcPolarityConv = GetPolarityConversion15p16(modSrcInfo.mPoleType, spec.mSourcePolarity);
            auto auxPolarityConv = GetPolarityConversion15p16(modAuxInfo.mPoleType, spec.mAuxPolarity);

            uint32_t *bufSrc32 = (uint32_t *)bufSrc;
            uint32_t *bufAux32 = (uint32_t *)bufAux;
            uint32_t *bufDest32 = (uint32_t *)bufDest;

            for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
            {
                uint32_t x32 = bufSrc32[i32]; // process 2 16-bit samples per loop to take advantage of 32-bit processing

                // this ordering is much faster than processing all x1 first then x2
                int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
                int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
                x1 = x1 + srcPolarityConv.add;
                x2 = x2 + srcPolarityConv.add;
                x1 = gModCurveLUT.Transfer16(x1, srcCurveState);
                x2 = gModCurveLUT.Transfer16(x2, srcCurveState);
                x1 = signed_multiply_32x16b(sourceScale16p16, x1);
                x2 = signed_multiply_32x16b(sourceScale16p16, x2);

                if (spec.mAuxEnabled && spec.mAuxSource != clarinoid::AnyModulationSource::None)
                {
                    int32_t aux32 = bufAux32[i32];
                    int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
                    int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
                    aux1 += auxPolarityConv.add;
                    aux2 += auxPolarityConv.add;
                    aux1 = gModCurveLUT.Transfer16(aux1, auxCurveState);
                    aux2 = gModCurveLUT.Transfer16(aux2, auxCurveState);
                    aux1 = signed_multiply_32x16b(auxScale16p16, aux1);
                    aux2 = signed_multiply_32x16b(auxScale16p16, aux2);
                    x1 = signed_multiply_32x16b(aux1, x1);
                    x2 = signed_multiply_32x16b(aux2, x2);
                }

                x32 = pack_16b_16b(x1, x2);
                bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
            }
            sw.Pause();
        }
        Serial.println(String("one curve, no aux: ") + (int)sw.ElapsedTime().ElapsedMicros());
    }

    // experimented a bit with doing block ops before looping, but it never helps even close.
    // basically, if you're looping per sample, you lose any benefit of block ops.
}

void setup()
{
    ModCurveMain();
    //ModBenchmarkMain();
    Serial.begin(9600);
    // while (!Serial)
    // {
    // } // when you are debugging with serial, uncomment this to ensure you see startup msgs
    // Serial.println("starting normally....");

    // auto* app = new clarinoid::TestDeviceApp;
    // app->Main();
}

void loop()
{
    // unreachable
}
