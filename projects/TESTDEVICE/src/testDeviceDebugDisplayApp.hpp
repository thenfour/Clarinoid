
#pragma once

#include <algorithm>

#include <clarinoid/menu/MenuSettings.hpp>
#include <clarinoid/menu/Plotter.hpp>


namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct PerformanceApp : SettingsMenuApp
{
    size_t mSelectedTaskID = 0;
    TaskPlanner *mTaskManager = nullptr;
    MusicalStateTask *mpMusicalStateTask = nullptr;
    Clarinoid2ControlMapper *mpControls = nullptr;
    Metronome *mpMetronome = nullptr;

    virtual const char *DisplayAppGetName() override
    {
        return "Performance";
    }

    SimpleMovingAverage<30> mCPUUsage;

    PerformanceApp(IDisplay &d, MusicalStateTask *pMusicalStateTask, Clarinoid2ControlMapper *controls, Metronome *pm)
        : SettingsMenuApp(d), mpMusicalStateTask(pMusicalStateTask), mpControls(controls), mpMetronome(pm)
    {
    }

    MultiSubmenuSettingItem mTiming;

    LabelSettingItem mDelay = {
        Property<String>{[](void *cap) {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "TS Delay: ";
                             // ret += String((uint32_t)cap);
                             ret += (int)pThis->mTaskManager->mPreviousTimeSliceDelayTime.ElapsedMicros();
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    LabelSettingItem mTimesliceLen = {
        Property<String>{[](void *cap) {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "TS Len: ";
                             ret += (int)pThis->mTaskManager->mTimesliceDuration.ElapsedMicros();
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    LabelSettingItem mCPU = {
        Property<String>{[](void *cap) {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "CPU usage: ";
                             float p = (float)pThis->mTaskManager->mPreviousTimeSliceDelayTime.ElapsedMicros();
                             p /= pThis->mTaskManager->mTimesliceDuration.ElapsedMicros();
                             p = 1.0f - p;
                             pThis->mCPUUsage.Update(p);
                             p = pThis->mCPUUsage.GetValue() * 100;
                             ret += p;
                             ret += "%";
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    LabelSettingItem mInput = {Property<String>{[](void *cap) {
                                                    PerformanceApp *pThis = (PerformanceApp *)cap;
                                                    String ret = "M->Input:";
                                                    ret += (int)pThis->mpMusicalStateTask->mInputTiming.GetValue();
                                                    return ret;
                                                },
                                                this},
                               AlwaysEnabled};

    LabelSettingItem mMusicalState = {
        Property<String>{[](void *cap) {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "M->Music:";
                             ret += (int)pThis->mpMusicalStateTask->mMusicalStateTiming.GetValue();
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    // LabelSettingItem mSynthState = {
    //     Property<String>{[](void *cap) {
    //                          PerformanceApp *pThis = (PerformanceApp *)cap;
    //                          String ret = "M->Synth:";
    //                          ret += (int)pThis->mpMusicalStateTask->mSynthStateTiming.GetValue();
    //                          return ret;
    //                      },
    //                      this},
    //     AlwaysEnabled};

    void Init(TaskPlanner *tm)
    {
        mTaskManager = tm;

        mTiming.Init(
            [](void *cap) { // get item count
                PerformanceApp *pThis = (PerformanceApp *)cap;
                return pThis->mTaskManager->mTasks.size();
            },
            [](void *cap, size_t i) // get name
            {
                PerformanceApp *pThis = (PerformanceApp *)cap;
                auto &t = pThis->mTaskManager->mTasks[i];
                String ret = String(t.mInfo.mName) + ":" + (int)t.mInfo.mExecutionTimeMicros.GetValue();
                // Serial.println(ret);
                return ret;
            },
            [](void *cap, size_t i) // get submenu
            {
                PerformanceApp *pThis = (PerformanceApp *)cap;
                pThis->mSelectedTaskID = i;
                return &pThis->mSubitemList;
            },
            [](void *cap, size_t i) { return true; },
            this);
    }

    LabelSettingItem mSubitemTmp = {Property<String>{[](void *cap) {
                                                         // PerformanceApp* pThis = (PerformanceApp*)cap;
                                                         return String("tmp.");
                                                     },
                                                     this},
                                    AlwaysEnabled};

    ISettingItem *mSubitemArray[1] = {
        &mSubitemTmp,
    };
    SettingsList mSubitemList = {mSubitemArray};

    ISettingItem *mArray[6] = {
        &mDelay,        // ok
        &mTimesliceLen, // ok
        &mCPU,          //
        &mTiming,       // ok
        &mInput,        //
        &mMusicalState, //
        //&mSynthState,   //
    };
    SettingsList mRootList = {mArray};
    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    PeakMeterUtility<2000, 300> mPeakMeter;

    virtual void RenderFrontPage()
    {
        auto &perf = mAppSettings->GetCurrentPerformancePatch();

        // line 1
        mDisplay.setTextWrap(false);
        mDisplay.println(String("A:") + mAppSettings->GetSynthPatchName(perf.mSynthPresetA));

        mDisplay.println(String("Enc: ") + mpControls->mEncoder.CurrentValue());
        mDisplay.println(String("Back: ") + mpControls->mBack.CurrentValue());
        mDisplay.println(String("OK: ") + mpControls->mOK.CurrentValue());

        float peak, heldPeak;
        mPeakMeter.Update(peak, heldPeak);
        auto y = mDisplay.getCursorY();
        mDisplay.println(String("Peak ") + heldPeak);
        mDisplay.fillRoundRect(0, y, peak * mDisplay.width(), 5, 2, SSD1306_INVERSE);
        mDisplay.drawFastVLine(heldPeak * mDisplay.width(), y, 8, SSD1306_INVERSE);

        y = mDisplay.getCursorY();
        float val = mpMusicalStateTask->mMusicalState.mCurrentPitchN11.GetValue();
        static const int pbwidth = 5;
        if (val >= 0)
        {
            int pbextent = std::max(1, (int)(val * mDisplay.GetClientHeight() / 2));
            mDisplay.fillRect(mDisplay.width() - pbwidth - 1,     // x
                                       (mDisplay.GetClientHeight() / 2) - pbextent, // y
                                       5,                                           // width
                                       pbextent,                                    // height
                                       SSD1306_INVERSE);
        }
        else
        {
            int pbextent = -val * mDisplay.GetClientHeight() / 2;
            mDisplay.fillRect(mDisplay.width() - pbwidth - 1, // x
                                       (mDisplay.GetClientHeight() / 2),        // y
                                       5,                                       // width
                                       pbextent,                                // height
                                       SSD1306_INVERSE);
        }

        // metronome
        y = mDisplay.getCursorY();
        float beatFrac = mpMetronome->GetBeatFrac();
        mDisplay.println(String("") + perf.mBPM + " bpm " + mpMetronome->GetBeatInt());
        mDisplay.fillRoundRect(0, y, beatFrac * mDisplay.width(), 5, 2, SSD1306_INVERSE);

        static const int R = 24;
        static const int P = 16;
        static const float THRESH = 0.1f;
        mDisplay.fillCircle(
            mDisplay.width() - P, P, Clamp(THRESH - beatFrac, 0.0f, 1.0f) * R / THRESH, SSD1306_INVERSE);

        // state
        {
            float p = (float)mTaskManager->mPreviousTimeSliceDelayTime.ElapsedMicros();
            p /= mTaskManager->mTimesliceDuration.ElapsedMicros();
            p = 1.0f - p;
            mCPUUsage.Update(p);
            p = mCPUUsage.GetValue() * 100;
            mDisplay.println(String("v:NA") + "/" +
                                      MAX_SYNTH_VOICES + " a:" + (int)std::ceil(AudioProcessorUsage()) +
                                      "% tm:" + (int)std::ceil(p) + "%");
        }

        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
