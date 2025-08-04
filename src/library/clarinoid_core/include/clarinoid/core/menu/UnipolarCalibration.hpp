#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "Plotter.hpp"

// cycle through params similar to normal setting items, so this is something like a sub-menu.
// but because "back" is the only way to leave this editor, then there's no way to cancel. changes
// are always committed.

namespace clarinoid {

struct UnipolarCalibrationEditor : ISettingItemEditor
{
  enum class SelectedParam
  {
    RangeMin,
    RangeMax,
    CurveP,
    CurveS,
    COUNT
  };

  enum class EditState
  {
    SelectParam,
    EditParam,
  };

  SelectedParam mSelectedParam = SelectedParam::RangeMin;
  EditState mEditState = EditState::SelectParam;
  ISettingItemEditorActions* mpApi;

  Property<UnipolarMapping> mCalibBinding;

  cc::function<float(void*)>::ptr_t mRawValueGetter;
  void* mCapture = nullptr;

  Plotter<MAX_DISPLAY_WIDTH> mRawValuePlotter;
  Plotter<MAX_DISPLAY_WIDTH> mAdjValuePlotter;

  PulseWave mBlinker;

  UnipolarCalibrationEditor(const Property<UnipolarMapping>& calibBinding,
                            cc::function<float(void*)>::ptr_t rawValueGetter,
                            void* capture)
    : mCalibBinding(calibBinding)
    , mRawValueGetter(rawValueGetter)
    , mCapture(capture)
  {
    mBlinker.SetFrequencyAndDutyCycle01(3, .3);
  }

  void SetupEditing(ISettingItemEditorActions* papi, int x, int y)
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
    if (mEditState == EditState::SelectParam) {
      if (backWasPressed) {
        mpApi->CommitEditing();
        return;
      }
      if (okWasPressed) {
        mEditState = EditState::EditParam;
        return;
      }

      mSelectedParam =
        (SelectedParam)AddConstrained((int)mSelectedParam, encIntDelta, 0, ((int)SelectedParam::COUNT) - 1);
      return;
    }

    // editing param...

    if (backWasPressed || okWasPressed) {
      mEditState = EditState::SelectParam;
      return;
    }

    // OK, no state changes. edit a param.
    auto val = mCalibBinding.GetValue();
    switch (mSelectedParam) {
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
      case SelectedParam::COUNT: // -Wswitch
        CCASSERT(false);
        break;
    }
  }

  // render the range editing screen. 2 columns showing in | out.
  void RenderRange(const String& caption)
  {
    IAppDisplay* d = mpApi->GetDisplay();
    auto lineHeight = d->GetLineHeight();
    bool blinkOn = mBlinker.GetValue01Int(micros());
    bool captionOn = (mEditState != EditState::SelectParam) || blinkOn;
    bool guidelineOn = (mEditState != EditState::EditParam) || blinkOn;

    SizeI screenSize = d->ScreenSize(); // { 0, 0, d->width(), d->GetClientHeight() };

    // calculate left & right areas
    RectI rcDisplay = { { 0, 0 }, screenSize };

    RectI rcLeft = rcDisplay.LeftHalf().WithOffsetTop(lineHeight);

    RectI rcRight = rcLeft.WithLeft(rcLeft.Right());

    // draw caption & chrome
    d->SetCursor({ 10, 0 });
    d->PrintLine(caption);
    if (captionOn) {
      d->SetCursor({ 0, 0 });
      d->Print(">");
    }
    d->DrawHLine(0,
                 lineHeight,
                 rcDisplay.Width(),
                 SSD1306_WHITE); // virtual void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    d->DrawVLine(rcRight.Left(),
                 rcRight.Top(),
                 rcRight.Height(),
                 SSD1306_WHITE); // virtual void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

    // update plotter & render them.
    UnipolarMapping v = mCalibBinding.GetValue();
    float f = mRawValueGetter(mCapture);
    mRawValuePlotter.Plot(f);
    mAdjValuePlotter.Plot(v.PerformMapping(f));
    mRawValuePlotter.Render(*d, rcLeft);
    mAdjValuePlotter.Render(*d, rcRight);

    // draw lines. (min, max)
    if (mSelectedParam != SelectedParam::RangeMin || guidelineOn) {
      int y = (int)((1.0f - v.mSrcMin) * rcLeft.Height());
      d->DrawDottedHLine(rcLeft.Left(), rcLeft.Width(), y + rcLeft.Top(), SSD1306_WHITE);
    }
    if (mSelectedParam != SelectedParam::RangeMax || guidelineOn) {
      int y = (int)((1.0f - v.mSrcMax) * rcLeft.Height());
      d->DrawDottedHLine(rcLeft.Left(), rcLeft.Width(), y + rcLeft.Top(), SSD1306_WHITE);
    }
  }

  // 3 columns with 1 row caption
  void RenderCurve(const String& caption)
  {
    IAppDisplay* d = mpApi->GetDisplay();
    auto lineHeight = d->GetLineHeight();
    bool blinkOn = mBlinker.GetValue01Int(micros());
    bool captionOn = (mEditState != EditState::SelectParam) || blinkOn;
    bool guidelineOn = (mEditState != EditState::EditParam) || blinkOn;

    RectI rcDisplay = { 0, 0, d->ScreenSize().width, d->GetClientHeight() };

    // calculate left & right areas
    RectI rcLeft =
      rcDisplay.WithOffsetTop(lineHeight).Cell(3, 1, 0, 0); // leave a line at the top, take 1st of 3 columns.
    RectI rcMiddle = rcLeft.Cell(1, 1, 1, 0);               // shifts right 1 column
    RectI rcRight = rcMiddle.Cell(1, 1, 1, 0);

    // caption & chrome
    d->SetCursor({ 10, 0 });
    d->Print(caption);
    if (captionOn) {
      d->SetCursor({ 0, 0 });
      d->Print(">");
    }
    d->DrawHLine(0,
                 lineHeight,
                 rcDisplay.Width(),
                 SSD1306_WHITE); // virtual void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    d->DrawVLine(rcMiddle.Left(),
                 rcMiddle.Top(),
                 rcMiddle.Height(),
                 SSD1306_WHITE); // virtual void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    d->DrawVLine(rcRight.Left(),
                 rcRight.Top(),
                 rcRight.Height(),
                 SSD1306_WHITE); // virtual void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

    // update plotter & render them.
    UnipolarMapping v = mCalibBinding.GetValue();
    float f = mRawValueGetter(mCapture);
    mRawValuePlotter.Plot(f);
    mAdjValuePlotter.Plot(v.PerformMapping(f));
    mRawValuePlotter.Render(*d, rcMiddle);
    mAdjValuePlotter.Render(*d, rcRight);

    // curve render with point
    for (int x = 0; x < rcLeft.Width(); ++x) {
      float fy =
        1.0f - Curve2::Eval((float)x / rcLeft.Width(), v.mCurveP, v.mCurveS); // 1-val for screen vertical flipping
      int y = fy * rcLeft.Height();
      // or is it void drawPixel(int16_t x, int16_t y, uint16_t color); ?
      d->SetPixel({ x, rcLeft.Top() + y },
                  SSD1306_WHITE); // virtual void writePixel(int16_t x, int16_t y, uint16_t color);
    }

    // render the little handle
    if (guidelineOn) {
      constexpr int HandleSize = 2;
      float fhy = 1.0f - Curve2::Eval(v.mCurveP, v.mCurveP, v.mCurveS); // 1-val for screen vertical flipping
      int hx = (int)(v.mCurveP * rcLeft.Width());
      int hy = fhy * rcLeft.Height();
      d->DrawDottedRect(rcLeft.Left() + hx - HandleSize,
                        rcLeft.Top() + hy - HandleSize,
                        HandleSize * 2 + 1,
                        HandleSize * 2 + 1,
                        SSD1306_WHITE);
    }
  }

  virtual void Render()
  {
    IAppDisplay* d = mpApi->GetDisplay();
    d->ClearState();
    d->FillScreen(SSD1306_BLACK);

    switch (mSelectedParam) {
      case SelectedParam::RangeMin:
        RenderRange(String("Range min: ") + mCalibBinding.GetValue().mSrcMin);
        break;
      case SelectedParam::RangeMax:
        RenderRange(String("Range max: ") + mCalibBinding.GetValue().mSrcMax);
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

struct UnipolarCalibrationSettingItem : public ISettingItem
{
  const char* mName;
  UnipolarCalibrationEditor mEditor;
  Property<UnipolarMapping> mCalibBinding;

  UnipolarCalibrationSettingItem(const char* name,
                                 const Property<UnipolarMapping>& calibBinding,
                                 cc::function<float(void*)>::ptr_t rawValueGetter,
                                 void* capture)
    : mName(name)
    , mEditor(calibBinding, rawValueGetter, capture)
    , mCalibBinding(calibBinding)
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
  virtual ISettingItemEditor* GetEditor(size_t multiIndex) { return &mEditor; }
};

} // namespace clarinoid
