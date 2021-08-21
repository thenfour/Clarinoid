#pragma once

#include <clarinoid/menu/MenuSettings.hpp>
#include <clarinoid/menu/Plotter.hpp>

namespace clarinoid
{

static String IndexToChar(int i)
{
    char r[2] = {0};
    if (i < 10)
    { // 0-9
        r[0] = '0' + i;
    }
    else if (i < 37)
    { // 10-36
        r[0] = 'A' + (i - 10);
    }
    else
    {
        r[0] = '!';
    }
    return String(r);
}

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
        Property<String>{[](void *cap) FLASHMEM {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "TS Delay: ";
                             // ret += String((uint32_t)cap);
                             ret += (int)pThis->mTaskManager->mPreviousTimeSliceDelayTime.ElapsedMicros();
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    LabelSettingItem mTimesliceLen = {
        Property<String>{[](void *cap) FLASHMEM {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "TS Len: ";
                             ret += (int)pThis->mTaskManager->mTimesliceDuration.ElapsedMicros();
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    LabelSettingItem mCPU = {
        Property<String>{[](void *cap) FLASHMEM {
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

    LabelSettingItem mInput = {Property<String>{[](void *cap) FLASHMEM {
                                                    PerformanceApp *pThis = (PerformanceApp *)cap;
                                                    String ret = "M->Input:";
                                                    ret += (int)pThis->mpMusicalStateTask->mInputTiming.GetValue();
                                                    return ret;
                                                },
                                                this},
                               AlwaysEnabled};

    LabelSettingItem mMusicalState = {
        Property<String>{[](void *cap) FLASHMEM {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "M->Music:";
                             ret += (int)pThis->mpMusicalStateTask->mMusicalStateTiming.GetValue();
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    LabelSettingItem mSynthState = {
        Property<String>{[](void *cap) FLASHMEM {
                             PerformanceApp *pThis = (PerformanceApp *)cap;
                             String ret = "M->Synth:";
                             ret += (int)pThis->mpMusicalStateTask->mSynthStateTiming.GetValue();
                             return ret;
                         },
                         this},
        AlwaysEnabled};

    void Init(TaskPlanner *tm)
    {
        mTaskManager = tm;

        mTiming.Init(
            [](void *cap) FLASHMEM { // get item count
                PerformanceApp *pThis = (PerformanceApp *)cap;
                return pThis->mTaskManager->mTasks.mSize;
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

    LabelSettingItem mSubitemTmp = {Property<String>{[](void *cap) FLASHMEM {
                                                         // PerformanceApp* pThis = (PerformanceApp*)cap;
                                                         return String("tmp.");
                                                     },
                                                     this},
                                    AlwaysEnabled};

    ISettingItem *mSubitemArray[1] = {
        &mSubitemTmp,
    };
    SettingsList mSubitemList = {mSubitemArray};

    ISettingItem *mArray[7] = {
        &mDelay,        // ok
        &mTimesliceLen, // ok
        &mCPU,          //
        &mTiming,       // ok
        &mInput,        //
        &mMusicalState, //
        &mSynthState,   //
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

        // line 0
        mDisplay.setTextWrap(false);
        mDisplay.println(String("P:") + perf.mName);

        // line 1
        mDisplay.setTextWrap(false);
        mDisplay.println(String("A:") + mAppSettings->GetSynthPatchName(perf.mSynthPresetA));

        // line 2
        mDisplay.setTextWrap(false);
        mDisplay.println(String("B:") + mAppSettings->GetSynthPatchName(perf.mSynthPresetB));

        // line 3
        mDisplay.println(perf.mHarmEnabled ? String("H:") + mAppSettings->GetHarmPatchName(perf.mHarmPreset)
                                           : (String("H:off:") + mAppSettings->GetHarmPatchName(perf.mHarmPreset)));

        float peak, heldPeak;
        mPeakMeter.Update(peak, heldPeak);
        auto y = mDisplay.getCursorY();
        mDisplay.println(String("Peak ") + heldPeak);
        mDisplay.fillRoundRect(0, y, peak * mDisplay.width(), 5, 2, SSD1306_INVERSE);
        mDisplay.drawFastVLine(heldPeak * mDisplay.width(), y, 8, SSD1306_INVERSE);

        // y = mDisplay.mDisplay.getCursorY();
        // float val = mpMusicalStateTask->mMusicalState.mCurrentBreath01.GetValue();
        // mDisplay.mDisplay.println(String("breath ") + val);
        // mDisplay.mDisplay.fillRoundRect(0, y, val * mDisplay.mDisplay.width(), 5, 2, SSD1306_INVERSE);

        y = mDisplay.getCursorY();
        float val = mpMusicalStateTask->mMusicalState.mCurrentPitchN11.GetValue();
        // mDisplay.mDisplay.println(String("pitch ") + val);
        static const int pbwidth = 5;
        if (val >= 0)
        {
            int pbextent = std::max(1, val * mDisplay.height() / 2);
            mDisplay.fillRect(mDisplay.width() - pbwidth - 1,     // x
                              (mDisplay.height() / 2) - pbextent, // y
                              5,                                  // width
                              pbextent,                           // height
                              SSD1306_INVERSE);
        }
        else
        {
            int pbextent = -val * mDisplay.height() / 2;
            mDisplay.fillRect(mDisplay.width() - pbwidth - 1, // x
                              (mDisplay.height() / 2),        // y
                              5,                              // width
                              pbextent,                       // height
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
            mDisplay.println(String("v:") + mpMusicalStateTask->mSynth.mCurrentPolyphony + "/" + MAX_SYNTH_VOICES +
                             " a:" + (int)std::ceil(AudioProcessorUsage()) + "% tm:" + (int)std::ceil(p) + "%");
        }

        SettingsMenuApp::RenderFrontPage();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DebugDisplayApp : SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "DebugDisplayApp";
    }

    Clarinoid2ControlMapper &mControls;
    MusicalStateTask &mMusicalStateTask;

    DebugDisplayApp(IDisplay &d, Clarinoid2ControlMapper &c, MusicalStateTask &mst)
        : SettingsMenuApp(d), mControls(c), mMusicalStateTask(mst)
    {
    }

    LabelSettingItem mBreath = {
        Property<String>{[](void *cap) FLASHMEM {
                             DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                             return (String)(
                                 (String("Breath: ") + int(pThis->mControls.mBreath.CurrentValue01() * 1000)));
                         },
                         this},
        AlwaysEnabled};

    LabelSettingItem mLHK = {
        Property<String>{
            [](void *cap) FLASHMEM {
                DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                String ret = "LHK:";
                ret += pThis->mMusicalStateTask.mpInput->mKeyLH1.CurrentValue() ? (String("1")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyLH2.CurrentValue() ? (String("2")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyLH3.CurrentValue() ? (String("3")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyLH4.CurrentValue() ? (String("4")) : String(" ");
                return ret;
            },
            this},
        AlwaysEnabled};

    LabelSettingItem mOct = {
        Property<String>{
            [](void *cap) FLASHMEM {
                DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                String ret = "Oct:";
                ret += pThis->mMusicalStateTask.mpInput->mKeyOct1.CurrentValue() ? (String("1")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyOct2.CurrentValue() ? (String("2")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyOct3.CurrentValue() ? (String("3")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyOct4.CurrentValue() ? (String("4")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyOct5.CurrentValue() ? (String("5")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyOct6.CurrentValue() ? (String("6")) : String(" ");
                return ret;
            },
            this},
        AlwaysEnabled};

    LabelSettingItem mRHK = {
        Property<String>{
            [](void *cap) FLASHMEM {
                DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                String ret = "RHK:";
                ret += pThis->mMusicalStateTask.mpInput->mKeyRH1.CurrentValue() ? (String("1")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyRH2.CurrentValue() ? (String("2")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyRH3.CurrentValue() ? (String("3")) : String(" ");
                ret += pThis->mMusicalStateTask.mpInput->mKeyRH4.CurrentValue() ? (String("4")) : String(" ");
                return ret;
            },
            this},
        AlwaysEnabled};

    LabelSettingItem mMCPA = {Property<String>{[](void *cap) FLASHMEM {
                                                   DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                   String ret = "MCPA:";
                                                   for (size_t i = 0; i < 8; ++i)
                                                   {
                                                       ret += pThis->mControls.mMCP.mButtons[i].CurrentValue()
                                                                  ? (String("") + i)
                                                                  : String(" ");
                                                   }
                                                   return ret;
                                               },
                                               this},
                              AlwaysEnabled};

    LabelSettingItem mMCPB = {Property<String>{[](void *cap) FLASHMEM {
                                                   DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                   String ret = "MCPB:";
                                                   for (size_t i = 8; i < 16; ++i)
                                                   {
                                                       ret += pThis->mControls.mMCP.mButtons[i].CurrentValue()
                                                                  ? (String("") + i)
                                                                  : String(" ");
                                                   }
                                                   return ret;
                                               },
                                               this},
                              AlwaysEnabled};

    LabelSettingItem mLHMPR121 = {Property<String>{[](void *cap) FLASHMEM {
                                                       DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                       String ret = "L:";
                                                       for (size_t i = 0; i < 12; ++i)
                                                       {
                                                           ret += pThis->mControls.mLHMPR.mButtons[i].CurrentValue()
                                                                      ? IndexToChar(i)
                                                                      : String("-");
                                                       }
                                                       return ret;
                                                   },
                                                   this},
                                  AlwaysEnabled};

    LabelSettingItem mRHMPR121 = {Property<String>{[](void *cap) FLASHMEM {
                                                       DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                       String ret = "R:";
                                                       for (size_t i = 0; i < 12; ++i)
                                                       {
                                                           ret += pThis->mControls.mRHMPR.mButtons[i].CurrentValue()
                                                                      ? IndexToChar(i)
                                                                      : String("-");
                                                       }
                                                       return ret;
                                                   },
                                                   this},
                                  AlwaysEnabled};

    LabelSettingItem mAudioProcessorUsage = {Property<String>{[](void *cap) FLASHMEM {
                                                                  // DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                                  String ret =
                                                                      String("Audio CPU %:") + AudioProcessorUsage();
                                                                  return ret;
                                                              },
                                                              this},
                                             AlwaysEnabled};

    LabelSettingItem mAudioProcessorUsageMax = {Property<String>{[](void *cap) FLASHMEM {
                                                                     // DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                                     String ret = String("Audio max CPU %:") +
                                                                                  AudioProcessorUsageMax();
                                                                     return ret;
                                                                 },
                                                                 this},
                                                AlwaysEnabled};

    LabelSettingItem mAudioMemoryUsage = {Property<String>{[](void *cap) FLASHMEM {
                                                               // DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                               String ret = String("Audio mem:") + AudioMemoryUsage();
                                                               return ret;
                                                           },
                                                           this},
                                          AlwaysEnabled};

    LabelSettingItem mAudioMemoryUsageMax = {Property<String>{[](void *cap) FLASHMEM {
                                                                  // DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                                  String ret =
                                                                      String("Audio mem max:") + AudioMemoryUsageMax();
                                                                  return ret;
                                                              },
                                                              this},
                                             AlwaysEnabled};

    ISettingItem *mArray[12] = {
        &mBreath,
        &mLHK,
        &mOct,
        &mRHK,
        &mLHMPR121,
        &mRHMPR121,
        &mMCPA,
        &mMCPB,
        &mAudioProcessorUsage,
        &mAudioProcessorUsageMax,
        &mAudioMemoryUsage,
        &mAudioMemoryUsageMax,
    };
    SettingsList mRootList = {mArray};

    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.println(String("Debug info ->"));
        SettingsMenuApp::RenderFrontPage();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct AudioMonitorApp : DisplayApp
{
    Plotter<MAX_DISPLAY_WIDTH> mPlotter;

    AudioMonitorApp(IDisplay &d) : DisplayApp(d)
    {
        mPlotter.Plot(0);
    }

    virtual const char *DisplayAppGetName() override
    {
        return "AudioMonitor";
    }

    virtual void UpdateApp() override
    {
        if (mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }
    virtual void RenderApp() override
    {
    }
    virtual void RenderFrontPage() override
    {
        mDisplay.println(String("Peak"));
        float peak = CCSynth::GetPeakLevel();
        mPlotter.Plot(peak);
        RectI rcDisplay = {0, 0, this->mDisplay.width(), this->mDisplay.height()};
        mPlotter.Render(this->mDisplay, rcDisplay);
    }

    virtual void DisplayAppUpdate() override
    {
        DisplayApp::DisplayAppUpdate(); // update input
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <size_t LHelectrodeCount, size_t RHelectrodeCount>
struct MPR121ConfigApp : SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "MPR121ConfigApp";
    }

    Clarinoid2ControlMapper &mControls;
    MusicalStateTask &mMusicalStateTask;

    MPR121ConfigApp(IDisplay &d, Clarinoid2ControlMapper &c, MusicalStateTask &mst)
        : SettingsMenuApp(d), mControls(c), mMusicalStateTask(mst)
    {
    }

    BoolSettingItem mTrackingEnabled = {
        "Tracking Enable",
        "ON",
        "OFF",
        Property<bool>{[](void *cap) FLASHMEM {
                           auto *pThis = (MPR121ConfigApp *)cap;
                           return pThis->mControls.mLHMPR.mMpr121.IsBaselineTrackingEnabled() &&
                                  pThis->mControls.mRHMPR.mMpr121.IsBaselineTrackingEnabled();
                       },
                       [](void *cap, const bool &v) {
                           auto *pThis = (MPR121ConfigApp *)cap;
                           pThis->mControls.mLHMPR.mMpr121.SetBaselineTrackingEnabled(v);
                           pThis->mControls.mRHMPR.mMpr121.SetBaselineTrackingEnabled(v);
                       },
                       this},
        AlwaysEnabled};

    TriggerSettingItem mTameOutliers = {
        String("Tame outliers"),
        [](void *cap) FLASHMEM {
            auto *pThis = (MPR121ConfigApp *)cap;
            // ASSUME that there's outliers.
            // get all baselines, determine the biggest gap, and everything above the
            // gap bring to the highest val below the gap.
            pThis->InitElectrodeData();

            // find the biggest gap, track the highest value below the gap.
            electrodeData *modelElectrode = nullptr;
            int biggestGap = 0;
            int16_t baselineAtGapTop = 0;
            for (size_t i = 0; i < SizeofStaticArray(pThis->mElectrodeData) - 1; ++i)
            {
                electrodeData &a = pThis->mElectrodeData[i];
                electrodeData &b = pThis->mElectrodeData[i + 1];
                auto gap = abs(a.mBaselineValue10bit - b.mBaselineValue10bit);
                if (gap >= biggestGap)
                {
                    biggestGap = gap;
                    if (a.mBaselineValue10bit < b.mBaselineValue10bit)
                    {
                        baselineAtGapTop =
                            a.mBaselineValue10bit + gap / 2; // because baseline is live-tracking, don't just use b's
                                                             // baseline val. you have to use something in the middle.
                        modelElectrode = &a;
                    }
                    else
                    {
                        baselineAtGapTop = b.mBaselineValue10bit + gap / 2;
                        modelElectrode = &b;
                    }
                }
            }

            // all electrodes which are above the gap, tame them.
            for (auto &e : pThis->mElectrodeData)
            {
                if (e.mBaselineValue10bit < baselineAtGapTop)
                    continue;
                e.mDevice->SetBaseline(e.mElectrodeIndex, modelElectrode->mBaselineValue10bit);
            }
        },
        this,
        []() { return true; }};

    TriggerSettingItem mCreateOutliers = {String("Create outlir"),
                                          [](void *cap) FLASHMEM {
                                              auto *pThis = (MPR121ConfigApp *)cap;
                                              pThis->mControls.mLHMPR.mMpr121.SetBaseline(1, 880);
                                              pThis->mControls.mRHMPR.mMpr121.SetBaseline(2, 880);
                                              pThis->mControls.mRHMPR.mMpr121.SetBaseline(3, 1000);
                                          },
                                          this,
                                          []() { return true; }};

    TriggerSettingItem mSoftReset = {String("Soft reset"),
                                     [](void *cap) FLASHMEM {
                                         auto *pThis = (MPR121ConfigApp *)cap;
                                         pThis->mControls.mLHMPR.mMpr121.SoftReset();
                                         pThis->mControls.mRHMPR.mMpr121.SoftReset();
                                     },
                                     this,
                                     []() { return true; }};

    BoolSettingItem mAutoconfigEnable = {
        "Autoconf Enable",
        "ON",
        "OFF",
        Property<bool>{[](void *cap) FLASHMEM {
                           auto *pThis = (MPR121ConfigApp *)cap;
                           return pThis->mControls.mLHMPR.mMpr121.IsAutoconfigEnabled() &&
                                  pThis->mControls.mRHMPR.mMpr121.IsAutoconfigEnabled();
                       },
                       [](void *cap, const bool &v) {
                           auto *pThis = (MPR121ConfigApp *)cap;
                           pThis->mControls.mLHMPR.mMpr121.SetAutoconfigEnabled(v);
                           pThis->mControls.mRHMPR.mMpr121.SetAutoconfigEnabled(v);
                       },
                       this},
        AlwaysEnabled};

    ISettingItem *mArray[5] = {
        &mTrackingEnabled, // does'nt work
        &mCreateOutliers,
        &mTameOutliers,     // doesn't work
        &mSoftReset,        // works well
        &mAutoconfigEnable, // it seems autoconfig is actually the best.
    };
    SettingsList mRootList = {mArray};

    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    struct electrodeData
    {
        MPR121::MPR121Device *mDevice = nullptr;
        uint8_t mElectrodeIndex = 0;
        int16_t mBaselineValue10bit = 0;
        int16_t mFilteredData10bit = 0;
    };

    electrodeData mElectrodeData[LHelectrodeCount + RHelectrodeCount];

    void InitElectrodeData()
    {
        // LH
        for (uint8_t i = 0; i < LHelectrodeCount; ++i)
        {
            mElectrodeData[i].mDevice = &mControls.mLHMPR.mMpr121;
            mElectrodeData[i].mElectrodeIndex = i;
            mElectrodeData[i].mBaselineValue10bit = mControls.mLHMPR.mMpr121.GetBaselineData(i);
            mElectrodeData[i].mFilteredData10bit = mControls.mLHMPR.mMpr121.filteredData(i);
        }

        // RH
        for (uint8_t i = 0; i < RHelectrodeCount; ++i)
        {
            mElectrodeData[LHelectrodeCount + i].mDevice = &mControls.mRHMPR.mMpr121;
            mElectrodeData[LHelectrodeCount + i].mElectrodeIndex = i;
            mElectrodeData[LHelectrodeCount + i].mBaselineValue10bit = mControls.mRHMPR.mMpr121.GetBaselineData(i);
            mElectrodeData[LHelectrodeCount + i].mFilteredData10bit = mControls.mRHMPR.mMpr121.filteredData(i);
        }
    }

    virtual void RenderFrontPage()
    {
        InitElectrodeData();
        size_t mKeyCount = SizeofStaticArray(mElectrodeData);

        for (size_t i = 0; i < mKeyCount; ++i)
        {
            int filteredVal = mElectrodeData[i].mFilteredData10bit; // mDevice.mMpr121.filteredData(i + mKeyIndexBegin);
            int baselineVal =
                mElectrodeData[i].mBaselineValue10bit; // mDevice.mMpr121.GetBaselineData(i + mKeyIndexBegin);

            int x = mDisplay.width() * i / mKeyCount;        // 128 * 5 / 10 =
            int x2 = mDisplay.width() * (i + 1) / mKeyCount; // 128 * 5 / 10 =
            int width = x2 - x;
            mDisplay.setCursor(x, 0);
            mDisplay.print(IndexToChar(i));

            mDisplay.drawFastVLine(x, 0, 4, WHITE);
            int filteredY = (int)mDisplay.height() * filteredVal / 1024; // 10 bit val scaled to height.
            int baselineY = (int)mDisplay.height() * baselineVal / 1024; // 10 bit val scaled to height.

            mDisplay.SetTextSolid(false);
            mDisplay.drawFastHLine(x, baselineY, width, WHITE);
            mDisplay.SetTextSolid(true);
            mDisplay.drawFastHLine(x, filteredY, width, WHITE);
        }

        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
