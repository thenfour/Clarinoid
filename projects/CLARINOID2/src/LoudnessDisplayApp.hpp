#pragma once

#include <clarinoid/application/DisplayDefs.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include <clarinoid/application/DefaultHud.hpp>
#include "clarinoid2MusicalStateTask.hpp"

namespace clarinoid
{
struct MusicalStateTask; // shut up vs code.

struct LoudnessDisplayApp : DisplayApp
{
    MusicalStateTask &mMusicalStateTask;
    ISysInfoProvider &mSysInfoProvider;

    LoudnessDisplayApp(IDisplay &display, MusicalStateTask &musicalStateTask, ISysInfoProvider &sysInfoProvider)
        : DisplayApp(display), mMusicalStateTask(musicalStateTask), mSysInfoProvider(sysInfoProvider)
    {
    }

    virtual void UpdateApp() override
    {
        if (mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }

    virtual void DisplayAppUpdate() override
    {
        DisplayApp::DisplayAppUpdate();
    }

    virtual const char *DisplayAppGetName() override
    {
        return "mesh demo";
    }

    virtual void RenderApp() override
    {
    }

    bool AllowOverlayIndicators() const override
    {
        return false;
    }

    void RenderChannel(int y, AudioAnalysisState state, const char *name)
    {
        static constexpr int barW = 81;
        static constexpr int barH = 8;
        VUMeterConfig vuConfig;
        vuConfig.rcDisplay = RectI::Construct(18, y, barW, barH);
        RenderVUMeterHoriz(mDisplay, state, vuConfig);
        if (state.isClipping)
        {
            mDisplay.setCursor(2, y);
            mDisplay.print("!");
        }

        mDisplay.setCursor(9, y);
        mDisplay.print(name);

        mDisplay.setCursor(100, y);
        if (state.isClipping)
        {
            mDisplay.print("CLIP");
        }
        else
        {
            mDisplay.print(GainToIntString(state.heldPeakLinear));
        }
    }

    virtual void RenderFrontPage() override
    {
        RenderChannel(1, gAnalysisStateL, "L");
        RenderChannel(12, gAnalysisStateR, "R");

        // render tables.
        static constexpr int labelRowY = 24;
        static constexpr int row1Y = 34;
        static constexpr int row2Y = 46;
        static constexpr int labelColX = 20;
        static constexpr int col1X = 33;
        static constexpr int col2X = 80;

        mDisplay.setCursor(col1X, labelRowY);
        mDisplay.print("RMS");
        mDisplay.setCursor(col2X, labelRowY);
        mDisplay.print("Peak");

        mDisplay.setCursor(labelColX, row1Y);
        mDisplay.print("L");
        mDisplay.setCursor(labelColX, row2Y);
        mDisplay.print("R");

        mDisplay.setCursor(col1X, row1Y);
        mDisplay.print(GainToIntString(gAnalysisStateL.rmsLinear));
        mDisplay.setCursor(col1X, row2Y);
        mDisplay.print(GainToIntString(gAnalysisStateR.rmsLinear));
        mDisplay.setCursor(col2X, row1Y);
        mDisplay.print(GainToIntString(gAnalysisStateL.peakLinear));
        mDisplay.setCursor(col2X, row2Y);
        mDisplay.print(GainToIntString(gAnalysisStateR.peakLinear));
    }
};

} // namespace clarinoid
