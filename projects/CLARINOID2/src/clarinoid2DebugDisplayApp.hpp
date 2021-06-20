#pragma once

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
        virtual const char *DisplayAppGetName() override { return "Performance"; }

        SimpleMovingAverage<30> mCPUUsage;

        PerformanceApp(CCDisplay &d, MusicalStateTask *pMusicalStateTask, Clarinoid2ControlMapper *controls) : SettingsMenuApp(d),
                                                                                                               mpMusicalStateTask(pMusicalStateTask),
                                                                                                               mpControls(controls)
        {
        }

        MultiSubmenuSettingItem mTiming;

        LabelSettingItem mDelay = {Property<String>{[](void *cap) {
                                                        PerformanceApp *pThis = (PerformanceApp *)cap;
                                                        String ret = "TS Delay: ";
                                                        //ret += String((uint32_t)cap);
                                                        ret += (int)pThis->mTaskManager->mPreviousTimeSliceDelayTime.ElapsedMicros();
                                                        return ret;
                                                    },
                                                    this},
                                   AlwaysEnabled};

        LabelSettingItem mTimesliceLen = {Property<String>{[](void *cap) {
                                                               PerformanceApp *pThis = (PerformanceApp *)cap;
                                                               String ret = "TS Len: ";
                                                               ret += (int)pThis->mTaskManager->mTimesliceDuration.ElapsedMicros();
                                                               return ret;
                                                           },
                                                           this},
                                          AlwaysEnabled};

        LabelSettingItem mCPU = {Property<String>{[](void *cap) {
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

        LabelSettingItem mMusicalState = {Property<String>{[](void *cap) {
                                                               PerformanceApp *pThis = (PerformanceApp *)cap;
                                                               String ret = "M->Music:";
                                                               ret += (int)pThis->mpMusicalStateTask->mMusicalStateTiming.GetValue();
                                                               return ret;
                                                           },
                                                           this},
                                          AlwaysEnabled};

        LabelSettingItem mSynthState = {Property<String>{[](void *cap) {
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
                [](void *cap) { // get item count
                    PerformanceApp *pThis = (PerformanceApp *)cap;
                    return pThis->mTaskManager->mTasks.size();
                },
                [](void *cap, size_t i) // get name
                {
                    PerformanceApp *pThis = (PerformanceApp *)cap;
                    auto &t = pThis->mTaskManager->mTasks[i];
                    String ret = String(t.mInfo.mName) + ":" + (int)t.mInfo.mExecutionTimeMicros.GetValue();
                    //Serial.println(ret);
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
                                                             //PerformanceApp* pThis = (PerformanceApp*)cap;
                                                             return String("tmp.");
                                                         },
                                                         this},
                                        AlwaysEnabled};

        ISettingItem *mSubitemArray[1] =
            {
                &mSubitemTmp,
            };
        SettingsList mSubitemList = {mSubitemArray};

        ISettingItem *mArray[7] =
            {
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
            mDisplay.mDisplay.println(String("Task timings >"));

            SettingsMenuApp::RenderFrontPage();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct DebugDisplayApp : SettingsMenuApp
    {
        virtual const char *DisplayAppGetName() override { return "DebugDisplayApp"; }

        Clarinoid2ControlMapper &mControls;
        MusicalStateTask &mMusicalStateTask;

        DebugDisplayApp(CCDisplay &d, Clarinoid2ControlMapper &c, MusicalStateTask &mst) : SettingsMenuApp(d),
                                                                                           mControls(c),
                                                                                           mMusicalStateTask(mst)
        {
        }

        LabelSettingItem mBreath = {Property<String>{[](void *cap) {
                                                         DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                         return (String)((String("Breath: ") + int(pThis->mControls.mBreath.CurrentValue01() * 1000)));
                                                     },
                                                     this},
                                    AlwaysEnabled};


        LabelSettingItem mLHK = {Property<String>{[](void *cap) {
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

        LabelSettingItem mOct = {Property<String>{[](void *cap) {
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


        LabelSettingItem mRHK = {Property<String>{[](void *cap) {
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



        LabelSettingItem mMCPA = {Property<String>{[](void *cap) {
                                                      DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                      String ret = "MCPA:";
                                                      for (size_t i = 0; i < 8; ++i)
                                                      {
                                                          ret += pThis->mControls.mMCP.mButtons[i].CurrentValue() ? (String("") + i) : String(" ");
                                                      }
                                                      return ret;
                                                  },
                                                  this},
                                 AlwaysEnabled};

        LabelSettingItem mMCPB = {Property<String>{[](void *cap) {
                                                      DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                      String ret = "MCPB:";
                                                      for (size_t i = 8; i < 16; ++i)
                                                      {
                                                          ret += pThis->mControls.mMCP.mButtons[i].CurrentValue() ? (String("") + i) : String(" ");
                                                      }
                                                      return ret;
                                                  },
                                                  this},
                                 AlwaysEnabled};

        static String IndexToChar(int i) {
            char r[2] = {0};
            if (i < 10) { // 0-9
                r[0] = '0' + i;
            } else if (i < 37) { // 10-36
                r[0] = 'A' + (i-10);
            } else {
                r[0] = '!';
            }
            return String(r);
        }

        LabelSettingItem mLHMPR121 = {Property<String>{[](void *cap) {
                                                      DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                      String ret = "L:";
                                                      for (size_t i = 0; i < 12; ++i)
                                                      {
                                                          ret += pThis->mControls.mLHMPR.mButtons[i].CurrentValue() ? IndexToChar(i) : String("-");
                                                      }
                                                      return ret;
                                                  },
                                                  this},
                                 AlwaysEnabled};

        LabelSettingItem mRHMPR121 = {Property<String>{[](void *cap) {
                                                      DebugDisplayApp *pThis = (DebugDisplayApp *)cap;
                                                      String ret = "R:";
                                                      for (size_t i = 0; i < 12; ++i)
                                                      {
                                                          ret += pThis->mControls.mRHMPR.mButtons[i].CurrentValue() ? IndexToChar(i) : String("-");
                                                      }
                                                      return ret;
                                                  },
                                                  this},
                                 AlwaysEnabled};

        LabelSettingItem mAudioProcessorUsage = {Property<String>{[](void *cap) {
                                                                      //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                                      String ret = String("Audio CPU %:") + AudioProcessorUsage();
                                                                      return ret;
                                                                  },
                                                                  this},
                                                 AlwaysEnabled};

        LabelSettingItem mAudioProcessorUsageMax = {Property<String>{[](void *cap) {
                                                                         //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                                         String ret = String("Audio max CPU %:") + AudioProcessorUsageMax();
                                                                         return ret;
                                                                     },
                                                                     this},
                                                    AlwaysEnabled};

        LabelSettingItem mAudioMemoryUsage = {Property<String>{[](void *cap) {
                                                                   //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                                   String ret = String("Audio mem:") + AudioMemoryUsage();
                                                                   return ret;
                                                               },
                                                               this},
                                              AlwaysEnabled};

        LabelSettingItem mAudioMemoryUsageMax = {Property<String>{[](void *cap) {
                                                                      //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
                                                                      String ret = String("Audio mem max:") + AudioMemoryUsageMax();
                                                                      return ret;
                                                                  },
                                                                  this},
                                                 AlwaysEnabled};

        ISettingItem *mArray[12] =
            {
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
            mDisplay.mDisplay.println(String("Debug info ->"));
            SettingsMenuApp::RenderFrontPage();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct AudioMonitorApp : DisplayApp
    {
        Plotter<MAX_DISPLAY_WIDTH> mPlotter;

        AudioMonitorApp(CCDisplay &d) : DisplayApp(d)
        {
            mPlotter.Plot(0);
        }

        virtual const char *DisplayAppGetName() override { return "AudioMonitor"; }

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
            mDisplay.mDisplay.println(String("Peak"));
            float peak = CCSynth::GetPeakLevel();
            mPlotter.Plot(peak);
            RectI rcDisplay = {0, 0, this->mDisplay.mDisplay.width(), this->mDisplay.mDisplay.height()};
            mPlotter.Render(this->mDisplay, rcDisplay);
        }

        virtual void DisplayAppUpdate() override
        {
            DisplayApp::DisplayAppUpdate(); // update input
        }
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct TouchKeyMonitorApp : DisplayApp
    {
        CCMPR121& mDevice;
        const char *mDisplayName;
        size_t mKeyIndexBegin;
        size_t mKeyCount;

        TouchKeyMonitorApp(CCDisplay &d, CCMPR121& device, const char *displayName, size_t keyIndexBegin, size_t keyCount) :
            DisplayApp(d),
            mDevice(device), mDisplayName(displayName),
            mKeyIndexBegin(keyIndexBegin),
            mKeyCount(keyCount)
        {
        }

        virtual const char *DisplayAppGetName() override { return mDisplayName; }

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
            //String strBase(mDisplayName);

            for (size_t i = 0; i < mKeyCount; ++ i) {
                int filteredVal = mDevice.mMpr121.filteredData(i + mKeyIndexBegin);
                int baselineVal = mDevice.mMpr121.baselineData(i + mKeyIndexBegin);

                int x = mDisplay.mDisplay.width() * i / mKeyCount; // 128 * 5 / 10 = 
                int x2 = mDisplay.mDisplay.width() * (i + 1) / mKeyCount; // 128 * 5 / 10 = 
                int width = x2 - x;
                mDisplay.mDisplay.setCursor(x, 0);
                mDisplay.mDisplay.print(String("") + i + mKeyIndexBegin);

                mDisplay.mDisplay.drawFastVLine(x, 0, 4, WHITE);
                int filteredY = (int)mDisplay.mDisplay.height() * filteredVal / 1024; // 10 bit val scaled to height.
                int baselineY = (int)mDisplay.mDisplay.height() * baselineVal / 1024; // 10 bit val scaled to height.

                //strBase += String(" ") + baselineVal + ":" + filteredVal + "[" + baselineY + ":" + filteredY + "]";

                mDisplay.mDisplay.mSolidText = false;
                mDisplay.mDisplay.drawFastHLine(x, baselineY, width, WHITE);
                mDisplay.mDisplay.mSolidText = true;
                mDisplay.mDisplay.drawFastHLine(x, filteredY, width, WHITE);
            }
            //log(strBase);
        }

        virtual void DisplayAppUpdate() override
        {
            DisplayApp::DisplayAppUpdate(); // updates input
        }
    };



} // namespace clarinoid
