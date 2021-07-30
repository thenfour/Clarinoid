
#pragma once

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN // as opposed to some submodules like LH / RH

#include "testDeviceBaseSystemSettings.hpp"

#include <clarinoid/basic/Basic.hpp>

#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Leds.hpp>

#include <clarinoid/components/Encoder.hpp>
#include <clarinoid/components/Potentiometer.hpp>
#include <clarinoid/components/HoneywellABPI2C.hpp>
#include <clarinoid/components/MCP23017.hpp>
#include <clarinoid/components/CCMPR121.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/application/Display.hpp>
#include <clarinoid/application/DefaultHud.hpp>
#include <clarinoid/application/ControlMapper.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include <clarinoid/menu/MenuAppSystemSettings.hpp>
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/midi/midi.hpp>
#include <clarinoid/application/MusicalState.hpp>
#include <clarinoid/Gui/GuiApp.hpp>

#include <clarinoid/synth/Synth.hpp>

#include "testDeviceControlMapper.hpp"
#include "testDeviceMusicalStateTask.hpp"
#include "testDeviceDebugDisplayApp.hpp"
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/menu/MenuAppMetronome.hpp>
#include <clarinoid/menu/MenuAppHarmonizerSettings.hpp>
#include <clarinoid/menu/MenuAppPerformanceSettings.hpp>

namespace clarinoid
{

struct FontInfo
{
    const GFXfont *mpFont;
    const char *mName;
};

const FontInfo gAllFonts[] = {

    {&pixChicago4pt7b, "pixChicago4pt7b"},

    {&MatchupPro8pt7b, "MatchupPro8pt7b"},

    {&TomThumb, "TomThumb"},

    {nullptr, "Default 6x8"},

};

struct FontTesterApp : SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "FontTesterApp";
    }

    FontTesterApp(CCDisplay &d) : SettingsMenuApp(d)
    {
    }

    size_t mSelectedFont = 0;

    MultiSubmenuSettingItem mFontList = {[](void *cap) { return SizeofStaticArray(gAllFonts); },
                                         [](void *cap, size_t mi) { // name
                                             // auto *pThis = (FontTesterApp *)cap;
                                             return String(gAllFonts[mi].mName);
                                         },                         // return string name
                                         [](void *cap, size_t mi) { // get submenu for item
                                             auto *pThis = (FontTesterApp *)cap;
                                             pThis->mSelectedFont = mi;
                                             return &pThis->mSubmenuList;
                                         },                                         // return submenu
                                         [](void *cap, size_t mi) { return true; }, // is enabled
                                         this};                                     // capture

    static const char *const gLoremIpsumLines[12];

    MultiLabelSettingItem mLoremIpsum = {[](void *cap) { return SizeofStaticArray(gLoremIpsumLines); }, // item count
                                         [](void *cap, size_t i) {
                                             auto *pThis = (FontTesterApp *)cap;
                                             pThis->mDisplay.mDisplay.setFont(gAllFonts[pThis->mSelectedFont].mpFont);
                                             return String(gLoremIpsumLines[i]);
                                         },                                        // text
                                         [](void *cap, size_t i) { return true; }, // is enabled
                                         this};

    ISettingItem *mSubmenuArray[1] = {
        &mLoremIpsum,
    };
    SettingsList mSubmenuList = {mSubmenuArray};

    ISettingItem *mArray[1] = {
        &mFontList,
    };
    SettingsList mRootList = {mArray};
    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.mDisplay.println("Font tester > ");
        SettingsMenuApp::RenderFrontPage();
    }
};

const char *const FontTesterApp::gLoremIpsumLines[12] = {
    "{......10}{......20}{......30}{......40}{......50}{......60}",
    "0123456789",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "NOPQRSTUVWXYZ",
    "abcdefghijklmnopqrstuvwxyz",
    "nopqrstuvwxyz",
    "Lorem ipsum dolor sit",
    "amet, consectetur adipiscing",
    "elit. Morbi eu lacinia ipsum",
    "ac luctus nisi. Duis at mi et",
    "sapien porttitor feugiat. Integer",
    "id nulla ut magna euismod lobortis.",
};

struct TestDeviceApp : ISysInfoProvider
{
    InputDelegator mInputDelegator;
    Clarinoid2ControlMapper mControlMapper;
    CCDisplay mDisplay;
    DefaultHud mHud;
    AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;

    PerformanceApp mPerformanceApp;
    PerformancePatchSettingsApp mPerfPatchApp;
    SynthPatchMenuApp mSynthPatchApp;
    MetronomeSettingsApp mMetronomeSettingsApp;
    HarmSettingsApp mHarmVoiceSettingsApp;
    HarmPatchSettingsApp mHarmPatchApp;

    TaskPlanner *mTaskPlanner = nullptr; // set after initializing it, late in the startup process.

    TestDeviceApp()
        : mDisplay(128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL), mHud(mDisplay, this),
          mMusicalStateTask(&mDisplay, &mAppSettings, &mInputDelegator, &mControlMapper),
          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper, &mMusicalStateTask.mMetronome),
          mPerfPatchApp(mDisplay), mSynthPatchApp(mDisplay),
          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, &mAppSettings, mDisplay),
          mHarmVoiceSettingsApp(mDisplay), mHarmPatchApp(mDisplay)
    {
    }

    SimpleMovingAverage<30> mCPUUsage;
    PeakMeterUtility<2000, 300> mPeakMeter;

    virtual uint8_t ISysInfoProvider_GetPolyphony() override
    {
        return 0;
    }
    virtual float ISysInfoProvider_GetAudioCPUUsage() override
    {
        return AudioProcessorUsage();
    }
    virtual float ISysInfoProvider_GetTaskManagerCPUUsage() override
    {
        if (!mTaskPlanner)
            return 0.0f;
        float p = (float)mTaskPlanner->mPreviousTimeSliceDelayTime.ElapsedMicros();
        p /= mTaskPlanner->mTimesliceDuration.ElapsedMicros();
        p = 1.0f - p;
        return p* 100.0f;
    }
    virtual float ISysInfoProvider_GetPeak() override
    {
        float peak, heldPeak;
        mPeakMeter.Update(peak, heldPeak);
        return heldPeak;
    }
    virtual MidiNote ISysInfoProvider_GetNote() override
    {
        return MidiNote((uint8_t)mMusicalStateTask.mMusicalState.mLastPlayedNote);
    }
    virtual float ISysInfoProvider_GetTempo() override
    {
        return mMusicalStateTask.mMetronome.mBPM;
    }
    virtual Metronome *ISysInfoProvider_GetMetronome() override
    {
        return &mMusicalStateTask.mMetronome;
    }
    virtual float ISysInfoProvider_GetPitchBendN11() override
    {
        return 0;
    }
    virtual float ISysInfoProvider_GetBreath01() override
    {
        return 0;
    }
    virtual AppSettings *ISysInfoProvider_GetSettings() override
    {
        return &mAppSettings;
    }

    void Main()
    {
        mControlMapper.Init(&mDisplay);

        FontTesterApp mFontTesterApp(mDisplay);
        GuiPerformanceApp mGuiPerformanceApp(mDisplay);

        IDisplayApp *allApps[] = {
            &mPerformanceApp, // nice to have this as front page to know if things are running healthy.
            &mGuiPerformanceApp,
            &mFontTesterApp,

            &mPerfPatchApp,
            &mHarmPatchApp,
            &mSynthPatchApp,

            &mHarmVoiceSettingsApp,

            &mMetronomeSettingsApp,
        };

        mInputDelegator.Init(&mAppSettings, &mControlMapper);

        size_t im = 0;

        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::Ok, ControlMapping::Function::MenuOK);
        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::Back, ControlMapping::Function::MenuBack);

        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::Button3, ControlMapping::Function::DisplayFontToggle);

        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::EncButton, ControlMapping::Function::DisplayFontToggle);

        mAppSettings.mControlMappings[im++] =
            ControlMapping::TypicalEncoderMapping(PhysicalControl::Enc, ControlMapping::Function::MenuScrollA);

        mDisplay.Init(&mAppSettings, &mInputDelegator, &mHud, allApps);
        mMusicalStateTask.Init();

        Wire1.setClock(400000); // use high speed mode. default speed = 100k

        FunctionTask mDisplayTask1{this, [](void *cap) {
                                       TestDeviceApp *pThis = (TestDeviceApp *)cap;
                                       pThis->mDisplay.UpdateAndRenderTask();
                                   }};

        FunctionTask mDisplayTask2{this, [](void *cap) {
                                       TestDeviceApp *pThis = (TestDeviceApp *)cap;
                                       pThis->mDisplay.DisplayTask();
                                   }};

        NopTask nopTask;

        // Important that a state task runs before display tasks, to ensure there's not a single
        // uninitialized glitch frame at startup
        TaskPlanner tp{
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(0), &mMusicalStateTask, "MusS0"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(8000), &mDisplayTask1, "Display1"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(16000), &mMusicalStateTask, "MusS1"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(23000), &mDisplayTask2, "Display2"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(32000), &nopTask, "Nop"},
        };

        mTaskPlanner = &tp;
        mPerformanceApp.Init(&tp);

        tp.Main();
    }
};

} // namespace clarinoid
