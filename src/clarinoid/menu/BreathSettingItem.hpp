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

struct BreathCalibrationEditor : ISettingItemEditor
{
    enum class SelectedParam
    {
        RangeMin = 0,
        RangeMax = 1,
        Curve = 2,
        //NoteOnThreshold = 3,
        COUNT = 3
    };
    enum class EditState
    {
        SelectParam,
        EditParam,
    };

    SelectedParam mSelectedParam = SelectedParam::RangeMin;
    EditState mEditState = EditState::SelectParam;
    ISettingItemEditorActions* mpApi;

    Property<BreathCalibrationSettings> mBinding;
    cc::function<float(void*)>::ptr_t mRawValueGetter;
    void* mCapture = nullptr;

    Plotter<1> mRawValuePlotter;
    Plotter<1> mAdjValuePlotter;

    PulseWave mBlinker;

    BreathCalibrationEditor(const Property<BreathCalibrationSettings>& binding, cc::function<float(void*)>::ptr_t rawValueGetter, void* capture) :
        mBinding(binding),
        mRawValueGetter(rawValueGetter),
        mCapture(capture)
    {
        mBlinker.SetFrequencyAndDutyCycle01(4, .2);
    }

    void SetupEditing(ISettingItemEditorActions* papi, int x, int y)
    {
        mpApi = papi;
        CCASSERT(!!papi);
        auto d = papi->GetDisplay();
        CCASSERT(!!d);
        RectI rd = { 0, 0, d->mDisplay.width(), d->mDisplay.height() };

        mEditState = EditState::SelectParam;

        mRawValuePlotter.Init(d, RectI { 0, 0, rd.width / 2, rd.height });
        mAdjValuePlotter.Init(d, RectI { rd.width / 2, 0, rd.width / 2, rd.height });
    }

    float ModifyFloat01Val(float f, int encIntDelta)
    {
        f += ((float)encIntDelta) / 75;
        if (f < 0) f = 0;
        if (f > 1) f = 1;
        return f;
    }

    virtual void Update(bool backWasPressed, bool okWasPressed, int encIntDelta)
    {
        if (backWasPressed)
        {
            Serial.println("back was pressed.");
        }
        if (mEditState == EditState::SelectParam)
        {
            if (backWasPressed) {
                mpApi->CommitEditing();
                return;
            }
            if (okWasPressed) {
                Serial.println("switching to EditParam.");
                mEditState = EditState::EditParam;
                return;
            }
            mSelectedParam = (SelectedParam)(((int)mSelectedParam + encIntDelta) % (int)SelectedParam::COUNT);
            return;
        }

        // mEditState == EditState::EditParam...
        if (backWasPressed || okWasPressed) {
            Serial.println("switching to SelectParam.");
            mEditState = EditState::SelectParam;
            return;
        }

        // OK, no state changes. edit a param.
        auto val = mBinding.GetValue();
        switch (mSelectedParam)
        {
            case SelectedParam::RangeMin:
                val.mRangeMin = ModifyFloat01Val(val.mRangeMin, encIntDelta);
                //Serial.println(String("mRangeMin change to ") + val.mRangeMin);
                break;
            case SelectedParam::RangeMax:
                val.mRangeMax = ModifyFloat01Val(val.mRangeMax, encIntDelta);
                //Serial.println(String("mRangeMax change to ") + val.mRangeMax);
                break;
            // case SelectedParam::NoteOnThreshold:
            //     val.mNoteOnThreshold = ModifyFloat01Val(val.mNoteOnThreshold, encIntDelta);
            //     //Serial.println(String("mNoteOnThreshold change to ") + val.mNoteOnThreshold);
            //     break;
            case SelectedParam::Curve:
                val.mCurve = val.mCurve + ((float)encIntDelta / 50);
                //Serial.println(String("mCurve change to ") + val.mCurve);
                break;
            case SelectedParam::COUNT: // -Wswitch
                CCASSERT(false);
                break;
        }
        mBinding.SetValue(val);
    }

    virtual void Render()
    {
        CCDisplay* d = mpApi->GetDisplay();
        d->mDisplay.clearDisplay();
        d->mDisplay.setCursor(0, 0);
        //d->mDisplay.println("breath setting item.");

        auto v = mBinding.GetValue();
        float f = mRawValueGetter(mCapture);
         mRawValuePlotter.Plot(f);
         mAdjValuePlotter.Plot(v.TranfsormValue01(f));

         mRawValuePlotter.Render();
         mAdjValuePlotter.Render();

        RectI rd = { 0, 0, d->mDisplay.width(), d->mDisplay.height() };
        d->mDisplay.setCursor(rd.width / 4, 0);
        d->mDisplay.print("Raw");
        d->mDisplay.setCursor(rd.width * 4 / 3, 0);
        d->mDisplay.print("Adj");
        int16_t y;

        bool blinkOn = mBlinker.GetValue01Int(micros());

        y = (int16_t)((float)rd.height * (1.0f-v.mRangeMin));
        d->mDisplay.setCursor(0, y-4);
        if (mSelectedParam != SelectedParam::RangeMin || blinkOn) {
            d->mDisplay.print(String("Min:") + v.mRangeMin);
            d->mDisplay.DrawDottedHLine<2>(0, rd.width / 2, y, SSD1306_WHITE);
        }

        y = (int16_t)((float)rd.height * (1.0f-v.mRangeMax));
        d->mDisplay.setCursor(0, y-4);
        if (mSelectedParam != SelectedParam::RangeMax || blinkOn) {
            d->mDisplay.print(String("Max:") + v.mRangeMax);
            d->mDisplay.DrawDottedHLine<2>(0, rd.width / 2, y, SSD1306_WHITE);
        }

        // y = (int16_t)((float)rd.height * (1.0f-v.mNoteOnThreshold));
        // d->mDisplay.setCursor(rd.width / 2, y-4);
        // if (mSelectedParam != SelectedParam::NoteOnThreshold || blinkOn) {
        //     d->mDisplay.print(String("Thr:") + v.mNoteOnThreshold);
        //     d->mDisplay.DrawDottedHLine<3>(rd.width / 2, rd.width / 2, y, SSD1306_WHITE);
        // }

        d->mDisplay.setCursor(rd.width / 2, rd.height - 8);
        if (mSelectedParam != SelectedParam::Curve || blinkOn) {
            d->mDisplay.print(String("Curve:") + v.mCurve);
        }
    }
};


struct BreathCalibrationSettingItem : public ISettingItem
{
  BreathCalibrationEditor mEditor;
  Property<BreathCalibrationSettings> mBinding;
  
  BreathCalibrationSettingItem(const Property<BreathCalibrationSettings>& binding, cc::function<float(void*)>::ptr_t rawValueGetter, void* capture) :
    mEditor(binding, rawValueGetter, capture),
    mBinding(binding)
  {
  }

  virtual String GetName(size_t multiIndex) { return "Breath"; }
    virtual String GetValueString(size_t multiIndex)
    {
        auto v = mBinding.GetValue();
        return String("") + v.mRangeMin + "-" + v.mRangeMax + " c" + v.mCurve/* + " t" + v.mNoteOnThreshold*/; // 0.01-0.96 c0.13 t1
    }

  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return true; }
  virtual ISettingItemEditor* GetEditor(size_t multiIndex) {
    return &mEditor;
  }
};





} // namespace clarinoid
