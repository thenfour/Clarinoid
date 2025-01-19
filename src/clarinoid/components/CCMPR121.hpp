
#pragma once

// make sure to #include <Adafruit_MPR121.h>

#include <clarinoid/basic/Basic.hpp>

#include "MCP23017.hpp" // for bitbutton
#include "./MPR121.hpp"

namespace clarinoid
{

struct CCMPR121
{
    MPR121::MPR121Device mMpr121;
    uint16_t mCurrentValue = 0;
    int mElectrodesInUse;

    BitButton mButtons[12] = {
        BitButton{mCurrentValue, 0},
        BitButton{mCurrentValue, 1},
        BitButton{mCurrentValue, 2},
        BitButton{mCurrentValue, 3},
        BitButton{mCurrentValue, 4},
        BitButton{mCurrentValue, 5},
        BitButton{mCurrentValue, 6},
        BitButton{mCurrentValue, 7},
        BitButton{mCurrentValue, 8},
        BitButton{mCurrentValue, 9},
        BitButton{mCurrentValue, 10},
        BitButton{mCurrentValue, 11},
    };

    void SoftReset() {
        NoInterrupts _ni;
        mMpr121.SoftReset();
        mCurrentValue = 0;
    }

    explicit CCMPR121(TwoWire *theWire, int address, uint8_t electrodesInUse) : mElectrodesInUse(electrodesInUse)
    {
        NoInterrupts _ni;
        mMpr121.begin(address, theWire, 24, 18, electrodesInUse, true);
    }

    void Update()
    {
        mCurrentValue = mMpr121.touched();
    }
};

    struct ElectrodeQueryResult
    {
        float mBaselineValue01 = 0;
        float mFilteredData01 = 0;
        float mTouchThreshold01 = 0;
        float mReleaseThreshold01 = 0;
        uint8_t mElectrodeIndex = 0;
        int16_t mBaselineValue10bit = 0;
        int16_t mFilteredData10bit = 0;
        int16_t mTouchThreshold10bit = 0;
        int16_t mReleaseThreshold10bit = 0;
        bool mIsTouched = false;
        MPR121::MPR121Device *mDevice = nullptr;
    };

struct ElectrodeStatusQuerier : ITask
{
    static constexpr uint8_t gMaxElectrodeCount = 16;
    ElectrodeQueryResult mElectrodeData[gMaxElectrodeCount];
    int mCurrentIndex = 0;
    CCMPR121 &mDevice;

    ElectrodeStatusQuerier(CCMPR121 &device) : mDevice(device)
    {
        for (int i = 0; i < gMaxElectrodeCount; ++i)
        {
            mElectrodeData[i].mDevice = &mDevice.mMpr121;
            mElectrodeData[i].mElectrodeIndex = i;
            mElectrodeData[i].mTouchThreshold10bit = mDevice.mMpr121.mTouchThreshold;
            mElectrodeData[i].mReleaseThreshold10bit = mDevice.mMpr121.mReleaseThreshold;
            mElectrodeData[i].mTouchThreshold01 = Clamp01(mDevice.mMpr121.mTouchThreshold / 1024.0f);
            mElectrodeData[i].mReleaseThreshold01 = Clamp01(mDevice.mMpr121.mReleaseThreshold / 1024.0f);
        }
    }

    virtual void TaskRun()
    {
        auto baselineRaw = mDevice.mMpr121.GetBaselineData(mCurrentIndex);
        auto filteredRaw = mDevice.mMpr121.filteredData(mCurrentIndex);
        mElectrodeData[mCurrentIndex].mBaselineValue10bit = baselineRaw;
        mElectrodeData[mCurrentIndex].mFilteredData10bit = filteredRaw;
        mElectrodeData[mCurrentIndex].mBaselineValue01 = Clamp01(baselineRaw / 1024.0f);
        mElectrodeData[mCurrentIndex].mFilteredData01 = Clamp01(filteredRaw / 1024.0f);
        mElectrodeData[mCurrentIndex].mIsTouched = !!(mDevice.mCurrentValue & (1 << mCurrentIndex));
        mCurrentIndex = (mCurrentIndex + 1) % gMaxElectrodeCount;
    }
};


} // namespace clarinoid
