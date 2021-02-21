#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "Plotter.hpp"

// cycle through params similar to normal setting items, so this is something like a sub-menu.
// but because "back" is the only way to leave this editor, then there's no way to cancel. changes
// are always committed.

namespace clarinoid
{

    struct UnipolarCalibrationEditor : ISettingItemEditor
    {
        enum class SelectedParam
        {
            RangeMin,
            RangeMax,
            CurveP,
            CurveS,
            NoteOnThreshold,
            COUNT
        };

        enum class EditState
        {
            SelectParam,
            EditParam,
        };

        SelectedParam mSelectedParam = SelectedParam::RangeMin;
        EditState mEditState = EditState::SelectParam;
        ISettingItemEditorActions *mpApi;

        Property<UnipolarMapping> mCalibBinding;
        Property<float> mNoteOnThreshBinding;
        bool mSupportsNoteOnThresh = false;

        cc::function<float(void *)>::ptr_t mRawValueGetter;
        void *mCapture = nullptr;

        Plotter<MAX_DISPLAY_WIDTH> mRawValuePlotter;
        Plotter<MAX_DISPLAY_WIDTH> mAdjValuePlotter;

        PulseWave mBlinker;

        UnipolarCalibrationEditor(const Property<UnipolarMapping> &calibBinding, const Property<float> *optionalNoteOnThreshBinding, cc::function<float(void *)>::ptr_t rawValueGetter, void *capture) : mCalibBinding(calibBinding),
                                                                                                                                                                                                         mSupportsNoteOnThresh(optionalNoteOnThreshBinding != nullptr),
                                                                                                                                                                                                         mRawValueGetter(rawValueGetter),
                                                                                                                                                                                                         mCapture(capture)
        {
            if (mSupportsNoteOnThresh)
            {
                mNoteOnThreshBinding = *optionalNoteOnThreshBinding;
            }
            mBlinker.SetFrequencyAndDutyCycle01(3, .3);
        }

        void SetupEditing(ISettingItemEditorActions *papi, int x, int y)
        {
            mpApi = papi;
            CCASSERT(!!papi);
            auto d = papi->GetDisplay();
            CCASSERT(!!d);

            mEditState = EditState::SelectParam;

            mRawValuePlotter.Init();
            mAdjValuePlotter.Init();
        }

        float ModifyFloat01Val(float f, int encIntDelta)
        {
            f += ((float)encIntDelta) / 100;
            f = Clamp(f, 0.0f, 1.0f);
            return f;
        }

        float ModifyFloatSlopeVal(float f, int encIntDelta)
        {
            f += ((float)encIntDelta) / 100;
            f = Clamp(f, -0.99f, 0.99f);
            return f;
        }

        virtual void Update(bool backWasPressed, bool okWasPressed, int encIntDelta)
        {
            if (mEditState == EditState::SelectParam)
            {
                if (backWasPressed)
                {
                    mpApi->CommitEditing();
                    return;
                }
                if (okWasPressed)
                {
                    mEditState = EditState::EditParam;
                    return;
                }

                mSelectedParam = (SelectedParam)AddConstrained((int)mSelectedParam, encIntDelta, 0, ((int)SelectedParam::COUNT) - 1);
                //mSelectedParam = (SelectedParam)(((int)mSelectedParam + encIntDelta) % (int)SelectedParam::COUNT);
                if (!mSupportsNoteOnThresh && mSelectedParam == SelectedParam::NoteOnThreshold)
                {
                    mSelectedParam = (SelectedParam)0;
                }
                return;
            }

            // editing param...

            if (backWasPressed || okWasPressed)
            {
                mEditState = EditState::SelectParam;
                return;
            }

            // OK, no state changes. edit a param.
            auto val = mCalibBinding.GetValue();
            switch (mSelectedParam)
            {
            case SelectedParam::RangeMin:
                val.mSrcMin = ModifyFloat01Val(val.mSrcMin, encIntDelta);
                mCalibBinding.SetValue(val);
                break;
            case SelectedParam::RangeMax:
                val.mSrcMax = ModifyFloat01Val(val.mSrcMax, encIntDelta);
                mCalibBinding.SetValue(val);
                break;
            case SelectedParam::CurveP:
                val.mCurveP = ModifyFloat01Val(val.mCurveP, encIntDelta);
                mCalibBinding.SetValue(val);
                break;
            case SelectedParam::CurveS:
                val.mCurveS = ModifyFloatSlopeVal(val.mCurveS, encIntDelta);
                mCalibBinding.SetValue(val);
                break;
            case SelectedParam::NoteOnThreshold:
            {
                float th = mNoteOnThreshBinding.GetValue();
                th = ModifyFloat01Val(th, encIntDelta);
                mNoteOnThreshBinding.SetValue(th);
                break;
            }
            case SelectedParam::COUNT: // -Wswitch
                CCASSERT(false);
                break;
            }
        }

        // render the range editing screen. 2 columns showing in | out.
        void RenderRange(const String &caption)
        {
            CCDisplay *d = mpApi->GetDisplay();
            auto lineHeight = d->mDisplay.GetLineHeight();
            bool blinkOn = mBlinker.GetValue01Int(micros());
            bool captionOn = (mEditState != EditState::SelectParam) || blinkOn;
            bool guidelineOn = (mEditState != EditState::EditParam) || blinkOn;

            RectI rcDisplay = {0, 0, d->mDisplay.width(), d->mDisplay.height()};

            // calculate left & right areas
            RectI rcLeft = rcDisplay;
            rcLeft.y = lineHeight;
            rcLeft.height -= lineHeight;
            rcLeft.width /= 2;
            RectI rcRight = rcLeft;
            rcRight.x = rcLeft.width;

            // draw caption & chrome
            d->mDisplay.setCursor(10, 0);
            d->mDisplay.println(caption);
            if (captionOn)
            {
                d->mDisplay.setCursor(0, 0);
                d->mDisplay.print(">");
            }
            d->mDisplay.writeFastHLine(0, lineHeight, rcDisplay.width, SSD1306_WHITE);       // virtual void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
            d->mDisplay.writeFastVLine(rcRight.x, rcRight.y, rcRight.height, SSD1306_WHITE); // virtual void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

            // update plotter & render them.
            UnipolarMapping v = mCalibBinding.GetValue();
            float f = mRawValueGetter(mCapture);
            mRawValuePlotter.Plot(f);
            mAdjValuePlotter.Plot(v.PerformMapping(f));
            mRawValuePlotter.Render(*d, rcLeft);
            mAdjValuePlotter.Render(*d, rcRight);

            // draw lines. (min, max, thresh)
            if (mSelectedParam != SelectedParam::RangeMin || guidelineOn)
            {
                int y = (int)((1.0f - v.mSrcMin) * rcLeft.height);
                d->mDisplay.DrawDottedHLine(rcLeft.x, rcLeft.width, y + rcLeft.y, SSD1306_WHITE);
            }
            if (mSelectedParam != SelectedParam::RangeMax || guidelineOn)
            {
                int y = (int)((1.0f - v.mSrcMax) * rcLeft.height);
                d->mDisplay.DrawDottedHLine(rcLeft.x, rcLeft.width, y + rcLeft.y, SSD1306_WHITE);
            }
            if (mSupportsNoteOnThresh)
            {
                if (mSelectedParam != SelectedParam::NoteOnThreshold || guidelineOn)
                {
                    float thr = mNoteOnThreshBinding.GetValue();
                    int y = (int)(thr * rcRight.height);
                    d->mDisplay.DrawDottedHLine(rcRight.x, rcRight.width, y + rcRight.height, SSD1306_WHITE);
                }
            }
        }

        // 3 columns with 1 row caption
        void RenderCurve(const String &caption)
        {
            CCDisplay *d = mpApi->GetDisplay();
            auto lineHeight = d->mDisplay.GetLineHeight();
            bool blinkOn = mBlinker.GetValue01Int(micros());
            bool captionOn = (mEditState != EditState::SelectParam) || blinkOn;
            bool guidelineOn = (mEditState != EditState::EditParam) || blinkOn;

            RectI rcDisplay = {0, 0, d->mDisplay.width(), d->mDisplay.height()};

            // calculate left & right areas
            RectI rcLeft = rcDisplay;
            rcLeft.y = lineHeight;
            rcLeft.height -= lineHeight;
            rcLeft.width /= 3;

            RectI rcMiddle = rcLeft;
            rcMiddle.x += rcLeft.width;

            RectI rcRight = rcMiddle;
            rcRight.x += rcMiddle.width;

            // caption & chrome
            d->mDisplay.setCursor(10, 0);
            d->mDisplay.print(caption);
            if (captionOn)
            {
                d->mDisplay.setCursor(0, 0);
                d->mDisplay.print(">");
            }
            d->mDisplay.writeFastHLine(0, lineHeight, rcDisplay.width, SSD1306_WHITE);          // virtual void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
            d->mDisplay.writeFastVLine(rcMiddle.x, rcMiddle.y, rcMiddle.height, SSD1306_WHITE); // virtual void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
            d->mDisplay.writeFastVLine(rcRight.x, rcRight.y, rcRight.height, SSD1306_WHITE);    // virtual void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

            // update plotter & render them.
            UnipolarMapping v = mCalibBinding.GetValue();
            float f = mRawValueGetter(mCapture);
            mRawValuePlotter.Plot(f);
            mAdjValuePlotter.Plot(v.PerformMapping(f));
            mRawValuePlotter.Render(*d, rcMiddle);
            mAdjValuePlotter.Render(*d, rcRight);

            // curve render with point
            for (int x = 0; x < rcLeft.width; ++x)
            {
                float fy = 1.0f - Curve2::Eval((float)x / rcLeft.width, v.mCurveP, v.mCurveS); // 1-val for screen vertical flipping
                int y = fy * rcLeft.height;
                // or is it void drawPixel(int16_t x, int16_t y, uint16_t color); ?
                d->mDisplay.writePixel(x, rcLeft.y + y, SSD1306_WHITE); //virtual void writePixel(int16_t x, int16_t y, uint16_t color);
            }

            // render the little handle
            if (guidelineOn)
            {
                constexpr int HandleSize = 2;
                float fhy = 1.0f - Curve2::Eval(v.mCurveP, v.mCurveP, v.mCurveS); // 1-val for screen vertical flipping
                int hx = (int)(v.mCurveP * rcLeft.width);
                int hy = fhy * rcLeft.height;
                d->mDisplay.DrawDottedRect(rcLeft.x + hx - HandleSize, rcLeft.y + hy - HandleSize, HandleSize * 2 + 1, HandleSize * 2 + 1, SSD1306_WHITE);
            }
        }

        virtual void Render()
        {
            CCDisplay *d = mpApi->GetDisplay();
            d->mDisplay.clearDisplay();
            d->mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

            switch (mSelectedParam)
            {
            case SelectedParam::RangeMin:
                RenderRange(String("Range min: ") + mCalibBinding.GetValue().mSrcMin);
                break;
            case SelectedParam::RangeMax:
                RenderRange(String("Range max: ") + mCalibBinding.GetValue().mSrcMax);
                break;
            case SelectedParam::NoteOnThreshold:
                RenderRange(String("Note thresh: ") + mNoteOnThreshBinding.GetValue());
                break;
            case SelectedParam::CurveP:
                RenderCurve(String("Curve Pos: ") + mCalibBinding.GetValue().mCurveP);
                break;
            case SelectedParam::CurveS:
                RenderCurve(String("Curve Slope: ") + mCalibBinding.GetValue().mCurveS);
                break;
            case SelectedParam::COUNT: // -Wswitch
                CCASSERT(false);
                break;
            }
        }
    };

    struct BreathCalibrationSettingItem : public ISettingItem
    {
        const char *mName;
        UnipolarCalibrationEditor mEditor;
        Property<UnipolarMapping> mCalibBinding;

        BreathCalibrationSettingItem(const char *name, const Property<UnipolarMapping> &calibBinding,
                                     const Property<float> &noteOnThreshBinding,
                                     cc::function<float(void *)>::ptr_t rawValueGetter,
                                     void *capture) : mName(name),
                                                      mEditor(calibBinding, &noteOnThreshBinding, rawValueGetter, capture),
                                                      mCalibBinding(calibBinding)
        {
        }

        virtual String GetName(size_t multiIndex) { return mName; }
        virtual String GetValueString(size_t multiIndex)
        {
            auto v = mCalibBinding.GetValue();
            return String("") + v.mSrcMin + "-" + v.mSrcMax;
        }

        virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
        virtual bool IsEnabled(size_t multiIndex) const { return true; }
        virtual ISettingItemEditor *GetEditor(size_t multiIndex)
        {
            return &mEditor;
        }
    };

} // namespace clarinoid
