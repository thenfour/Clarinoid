
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
#include <clarinoid/application/ControlMapper.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include <clarinoid/menu/MenuAppSystemSettings.hpp>
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/midi/midi.hpp>
#include <clarinoid/application/MusicalState.hpp>

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

    {&FreeMono12pt7b, "FreeMono12pt7b"},
    {&FreeMono18pt7b, "FreeMono18pt7b"},
    {&FreeMono24pt7b, "FreeMono24pt7b"},
    {&FreeMono9pt7b, "FreeMono9pt7b"},
    {&FreeMonoBold12pt7b, "FreeMonoBold12pt7b"},
    {&FreeMonoBold18pt7b, "FreeMonoBold18pt7b"},
    {&FreeMonoBold24pt7b, "FreeMonoBold24pt7b"},
    {&FreeMonoBold9pt7b, "FreeMonoBold9pt7b"},
    {&FreeMonoBoldOblique12pt7b, "FreeMonoBoldOblique12pt7b"},
    {&FreeMonoBoldOblique18pt7b, "FreeMonoBoldOblique18pt7b"},
    {&FreeMonoBoldOblique24pt7b, "FreeMonoBoldOblique24pt7b"},
    {&FreeMonoBoldOblique9pt7b, "FreeMonoBoldOblique9pt7b"},
    {&FreeMonoOblique12pt7b, "FreeMonoOblique12pt7b"},
    {&FreeMonoOblique18pt7b, "FreeMonoOblique18pt7b"},
    {&FreeMonoOblique24pt7b, "FreeMonoOblique24pt7b"},
    {&FreeMonoOblique9pt7b, "FreeMonoOblique9pt7b"},
    {&FreeSans12pt7b, "FreeSans12pt7b"},
    {&FreeSans18pt7b, "FreeSans18pt7b"},
    {&FreeSans24pt7b, "FreeSans24pt7b"},
    {&FreeSans9pt7b, "FreeSans9pt7b"},
    {&FreeSansBold12pt7b, "FreeSansBold12pt7b"},
    {&FreeSansBold18pt7b, "FreeSansBold18pt7b"},
    {&FreeSansBold24pt7b, "FreeSansBold24pt7b"},
    {&FreeSansBold9pt7b, "FreeSansBold9pt7b"},
    {&FreeSansBoldOblique12pt7b, "FreeSansBoldOblique12pt7b"},
    {&FreeSansBoldOblique18pt7b, "FreeSansBoldOblique18pt7b"},
    {&FreeSansBoldOblique24pt7b, "FreeSansBoldOblique24pt7b"},
    {&FreeSansBoldOblique9pt7b, "FreeSansBoldOblique9pt7b"},
    {&FreeSansOblique12pt7b, "FreeSansOblique12pt7b"},
    {&FreeSansOblique18pt7b, "FreeSansOblique18pt7b"},
    {&FreeSansOblique24pt7b, "FreeSansOblique24pt7b"},
    {&FreeSansOblique9pt7b, "FreeSansOblique9pt7b"},
    {&FreeSerif12pt7b, "FreeSerif12pt7b"},
    {&FreeSerif18pt7b, "FreeSerif18pt7b"},
    {&FreeSerif24pt7b, "FreeSerif24pt7b"},
    {&FreeSerif9pt7b, "FreeSerif9pt7b"},
    {&FreeSerifBold12pt7b, "FreeSerifBold12pt7b"},
    {&FreeSerifBold18pt7b, "FreeSerifBold18pt7b"},
    {&FreeSerifBold24pt7b, "FreeSerifBold24pt7b"},
    {&FreeSerifBold9pt7b, "FreeSerifBold9pt7b"},
    {&FreeSerifBoldItalic12pt7b, "FreeSerifBoldItalic12pt7b"},
    {&FreeSerifBoldItalic18pt7b, "FreeSerifBoldItalic18pt7b"},
    {&FreeSerifBoldItalic24pt7b, "FreeSerifBoldItalic24pt7b"},
    {&FreeSerifBoldItalic9pt7b, "FreeSerifBoldItalic9pt7b"},
    {&FreeSerifItalic12pt7b, "FreeSerifItalic12pt7b"},
    {&FreeSerifItalic18pt7b, "FreeSerifItalic18pt7b"},
    {&FreeSerifItalic24pt7b, "FreeSerifItalic24pt7b"},
    {&FreeSerifItalic9pt7b, "FreeSerifItalic9pt7b"},
    {&Org_01, "Org_01"},
    {&Picopixel, "Picopixel"},
    {&Tiny3x3a2pt7b, "Tiny3x3a2pt7b"},
    {&TomThumb, "TomThumb"},

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
    "........10........20........30........40........50",
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

struct TestDeviceApp
{
    InputDelegator mInputDelegator;
    Clarinoid2ControlMapper mControlMapper;
    CCDisplay mDisplay;
    AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;

    PerformanceApp mPerformanceApp;
    PerformancePatchSettingsApp mPerfPatchApp;
    SynthPatchMenuApp mSynthPatchApp;
    MetronomeSettingsApp mMetronomeSettingsApp;
    HarmSettingsApp mHarmVoiceSettingsApp;
    HarmPatchSettingsApp mHarmPatchApp;

    TestDeviceApp()
        : mDisplay(128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL),
          mMusicalStateTask(&mDisplay, &mAppSettings, &mInputDelegator, &mControlMapper),
          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper, &mMusicalStateTask.mMetronome),
          mPerfPatchApp(mDisplay), mSynthPatchApp(mDisplay),
          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, &mAppSettings, mDisplay),
          mHarmVoiceSettingsApp(mDisplay), mHarmPatchApp(mDisplay)
    {
    }

    void Main()
    {
        mControlMapper.Init(&mDisplay);

        FontTesterApp mFontTesterApp(mDisplay);

        IDisplayApp *allApps[] = {
            &mPerformanceApp, // nice to have this as front page to know if things are running healthy.
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
            ControlMapping::TypicalEncoderMapping(PhysicalControl::Enc, ControlMapping::Function::MenuScrollA);

        mDisplay.Init(&mAppSettings, &mInputDelegator, allApps);
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

        // the "Musical state" is the most critical. So let's run it periodically, spread through the whole time slice.
        // display tasks are also very heavy. Display1 is update/state, Display2 is SPI render.
        // musical state = about <2400 microseconds
        // display1 = about <1500 microseconds
        // display2 = about <2000 microseconds
        // LED tasks tend to be almost instantaneous (~<10 microseconds) so they can all live in the same slot.
        NopTask nopTask;

        TaskPlanner tp{
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(0), &mDisplayTask1, "Display1"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(1), &mMusicalStateTask, "MusS0"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(2), &mDisplayTask2, "Display2"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(3), &mMusicalStateTask, "MusS1"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(4), &nopTask, "Nop"},
        };

        mPerformanceApp.Init(&tp);

        tp.Main();
    }
};

} // namespace clarinoid
