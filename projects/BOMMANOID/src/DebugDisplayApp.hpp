#pragma once

#include <clarinoid/menu/MenuSettings.hpp>
#include <clarinoid/menu/Plotter.hpp>
#include "bsControlMapper.hpp"
#include "MusicalStateTask.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct PerformanceApp : SettingsMenuApp
{
    size_t mSelectedTaskID = 0;
    TaskPlanner *mTaskManager = nullptr;
    MusicalStateTask *mpMusicalStateTask = nullptr;
    BommanoidControlMapper *mpControls = nullptr;
    virtual const char *DisplayAppGetName() override
    {
        return "Performance";
    }

    SimpleMovingAverage<30> mCPUUsage;

    PerformanceApp(IDisplay &d, MusicalStateTask *pMusicalStateTask, BommanoidControlMapper *controls)
        : SettingsMenuApp(d), mpMusicalStateTask(pMusicalStateTask), mpControls(controls)
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
        &mCPU,          // definitely bugged
        &mTiming,       // ok
        &mInput,        // bugged
        &mMusicalState, // bugged
        &mSynthState,   // bugged
    };
    SettingsList mRootList = {mArray};
    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.println(String("PERFormance >"));
        // mDisplay.println(String(" McpL: ") + (int)mpControls->mTimingMcpL.ElapsedMicros());
        // mDisplay.println(String(" McpR: ") + (int)mpControls->mTimingMcpR.ElapsedMicros());
        // mDisplay.println(String(" Breath: ") + (int)mpControls->mTimingBreath.ElapsedMicros());
        // mDisplay.println(String(" Encoders: ") + (int)mpControls->mTimingEncoders.ElapsedMicros());
        // mDisplay.println(String(" Analog: ") + (int)mpControls->mTimingAnalog.ElapsedMicros());
        // mDisplay.println(String(" Digital: ") + (int)mpControls->mTimingDigital.ElapsedMicros());

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

    BommanoidControlMapper &mControls;
    MusicalStateTask &mMusicalStateTask;

    DebugDisplayApp(IDisplay &d, BommanoidControlMapper &c, MusicalStateTask &mst)
        : SettingsMenuApp(d), mControls(c), mMusicalStateTask(mst)
    {
    }

    LabelSettingItem mMidiNote = {Property<String>{[](void *cap) FLASHMEM {
                                                       DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                       String ret =
                                                           String("MidiNote: ") +
                                                           pThis->mMusicalStateTask.mMusicalState.mLiveVoice.mMidiNote;
                                                       return ret;
                                                   },
                                                   this},
                                  AlwaysEnabled};

    LabelSettingItem mEnc = {Property<String>{[](void *cap) FLASHMEM {
                                                    DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                    String ret = String("RH Encoder raw:") +
                                                                 pThis->mControls.mEncoder.RawValue();
                                                    return ret;
                                                },
                                                this},
                               AlwaysEnabled};

    LabelSettingItem mSynthPoly = {
        Property<String>{[](void *cap) FLASHMEM {
                             DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                             String ret = String("Synth poly:") + (pThis->mMusicalStateTask.mSynth.mCurrentPolyphony);
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

    ISettingItem *mArray[7] = {
        &mMidiNote,
        //&mFilterFreq,
        &mEnc,
        &mSynthPoly,
        &mAudioProcessorUsage,
        &mAudioProcessorUsageMax,
        &mAudioMemoryUsage,
        &mAudioMemoryUsageMax,
        // joyx
        // joyy
        // pitch
        // volume
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
        // float rms01 = gpSynthGraph->rms1.read();
        // gpSynthGraph->peak1.readPeakToPeak();
    }
    virtual void RenderApp() override
    {
        // mDisplay.mDisplay.setTextSize(1);
        // mDisplay.mDisplay.setCursor(0, 0);
        // mDisplay.mDisplay.setTextWrap(false);
        // mDisplay.mDisplay.println(String("woa"));
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

} // namespace clarinoid
