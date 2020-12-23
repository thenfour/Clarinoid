#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "ControlSource.hpp"
#include "FunctionHandler.hpp"

namespace clarinoid
{
  struct SynthPresetMappableFunction : FunctionHandler
  {
    IInputSource *mInputSrc;
    AppSettings *mAppSettings;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
      mInputSrc = psrc;
      mAppSettings = appSettings;
    }

    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      int nv = v.AsRoundedInt();
      nv = RotateIntoRange(nv, SYNTH_PRESET_COUNT);
      int old = mAppSettings->mGlobalSynthPreset;
      if (old != nv)
      {
        mInputSrc->InputSource_ShowToast(String("Synth preset: ") + nv + " (" + (nv - old) + ")");
        mAppSettings->mGlobalSynthPreset = nv;
      }
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override
    {
      int v = mAppSettings->mGlobalSynthPreset;
      return ControlValue::IntValue(v);
    }
  };

  struct TransposeMappableFunction : FunctionHandler
  {
    IInputSource *mInputSrc;
    AppSettings *mAppSettings;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
      mInputSrc = psrc;
      mAppSettings = appSettings;
    }

    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      int nv = v.AsRoundedInt();
      if (nv < -48)
      {
        mInputSrc->InputSource_ShowToast(String("Transposition\r\ntoo low"));
        return;
      }
      if (nv > 48)
      {
        mInputSrc->InputSource_ShowToast(String("Transposition\r\ntoo high"));
        return;
      }
      int old = mAppSettings->mTranspose;
      if (old != nv)
      {
        mInputSrc->InputSource_ShowToast(String("Transpose: ") + nv + " (" + (nv - old) + ")");
        mAppSettings->mTranspose = nv;
      }
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override
    {
      return ControlValue::IntValue(mAppSettings->mTranspose);
    }
  };

  struct InputDelegator
  {
    AppSettings *mpAppSettings = nullptr;
    IInputSource *mpSrc = nullptr;

    FunctionHandler *mHandlers[(size_t)ControlMapping::Function::COUNT] = {nullptr};

    // for convenience, we can put very-common composite inputs right here. avoids having to pass around stuff
    // everywhere in the app like breath.
    VirtualSwitch mMenuBack;
    VirtualSwitch mMenuOK;
    VirtualEncoder mMenuScrollA;

    VirtualSwitch mKeyLH1;
    VirtualSwitch mKeyLH2;
    VirtualSwitch mKeyLH3;
    VirtualSwitch mKeyLH4;

    VirtualSwitch mKeyRH1;
    VirtualSwitch mKeyRH2;
    VirtualSwitch mKeyRH3;
    VirtualSwitch mKeyRH4;

    VirtualSwitch mKeyOct1;
    VirtualSwitch mKeyOct2;
    VirtualSwitch mKeyOct3;

    VirtualAxis mBreath;
    VirtualAxis mPitchBend;

    VirtualSwitch mModifierFine;
    VirtualSwitch mModifierCourse;
    VirtualSwitch mModifierShift;
    VirtualSwitch mModifierCtrl;

    SynthPresetMappableFunction mSynthPresetFn;
    TransposeMappableFunction mTransposeFn;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
      mpAppSettings = appSettings;
      mpSrc = psrc;

      mSynthPresetFn.Init(appSettings, psrc);
      mTransposeFn.Init(appSettings, psrc);

      RegisterFunction(ControlMapping::Function::Nop, &mMenuBack); // anything works; it's never called.

      RegisterFunction(ControlMapping::Function::ModifierCourse, &mModifierCourse);
      RegisterFunction(ControlMapping::Function::ModifierFine, &mModifierFine);
      RegisterFunction(ControlMapping::Function::ModifierShift, &mModifierShift);
      RegisterFunction(ControlMapping::Function::ModifierCtrl, &mModifierCtrl);

      RegisterFunction(ControlMapping::Function::MenuBack, &mMenuBack);
      RegisterFunction(ControlMapping::Function::MenuOK, &mMenuOK);
      RegisterFunction(ControlMapping::Function::MenuScrollA, &mMenuScrollA);

      RegisterFunction(ControlMapping::Function::LH1, &mKeyLH1);
      RegisterFunction(ControlMapping::Function::LH2, &mKeyLH2);
      RegisterFunction(ControlMapping::Function::LH3, &mKeyLH3);
      RegisterFunction(ControlMapping::Function::LH4, &mKeyLH4);

      RegisterFunction(ControlMapping::Function::RH1, &mKeyRH1);
      RegisterFunction(ControlMapping::Function::RH2, &mKeyRH2);
      RegisterFunction(ControlMapping::Function::RH3, &mKeyRH3);
      RegisterFunction(ControlMapping::Function::RH4, &mKeyRH4);

      RegisterFunction(ControlMapping::Function::Oct1, &mKeyOct1);
      RegisterFunction(ControlMapping::Function::Oct2, &mKeyOct2);
      RegisterFunction(ControlMapping::Function::Oct3, &mKeyOct3);

      RegisterFunction(ControlMapping::Function::Breath, &mBreath);
      RegisterFunction(ControlMapping::Function::PitchBend, &mPitchBend);

      RegisterFunction(ControlMapping::Function::SynthPreset, &mSynthPresetFn);
      RegisterFunction(ControlMapping::Function::Transpose, &mTransposeFn);

      mpSrc->InputSource_Init(this);
    }

    template <typename Tval, typename TEnum>
    static bool HasFlag(Tval val, TEnum e)
    {
      auto ival = (typename std::underlying_type<TEnum>::type)val;
      auto ie = (typename std::underlying_type<TEnum>::type)e;
      return (ival & ie) == ie;
    }

    bool MatchesModifierKeys(const ControlMapping &m)
    {
      return true;

      // todo: this needs to be fully thought-out. we have modifier keys but,
      // criteria for matching them is not super basic, so it should be done right.
      // maybe also, mod keys mapping should be handled specially

      // if (m.mModifier == ModifierKey::Any)
      //   return true;
      // bool course = mModifierCourse.CurrentValue();
      // if (HasFlag(m.mModifier, ModifierKey::Course) && !course) // requires course but course not set.
      //   return false;
      // bool fine = mModifierFine.CurrentValue();
      // if (HasFlag(m.mModifier, ModifierKey::Fine) && !fine)
      //   return false;
      // bool shift = mModifierShift.CurrentValue();
      // if (HasFlag(m.mModifier, ModifierKey::Shift) && !shift)
      //   return false;
      // bool ctrl = mModifierCtrl.CurrentValue();
      // if (HasFlag(m.mModifier, ModifierKey::Ctrl) && !ctrl)
      //   return false;
      // if ((m.mModifier == ModifierKey::None) && (course || fine || shift || ctrl))
      //   return false;
      // return true;
    }

    // for test code.
    void ResetModifiers()
    {
      mModifierCourse.mValue = false;
      mModifierFine.mValue = false;
      mModifierShift.mValue = false;
      mModifierCtrl.mValue = false;
    }

    // process all input state and delegate to handlers.
    void Update()
    {
      CCASSERT(!!mpSrc);
      CCASSERT(!!this->mpAppSettings);

      // one control may be mapped to many functions.
      // one function may be controlled by many controls.
      // this aggregation is done by the FunctionHandler.
      size_t functionCount = (size_t)ControlMapping::Function::COUNT;
      for (size_t i = 0; i < functionCount; ++i)
      {
        CCASSERT(mHandlers[i] != nullptr);
        mHandlers[i]->BeginUpdate();
      }

      for (size_t i = 0; i < SizeofStaticArray(mpAppSettings->mControlMappings); ++i)
      {
        auto &mapping = mpAppSettings->mControlMappings[i];
        if (mapping.mFunction == ControlMapping::Function::Nop)
          continue;
        if (!MatchesModifierKeys(mapping))
          continue;

        CCASSERT((size_t)mapping.mFunction < SizeofStaticArray(mHandlers));

        FunctionHandler *dest = mHandlers[(size_t)mapping.mFunction]; // get the function this is mapped to
        auto src = mpSrc->InputSource_GetControl(mapping.mSource);    // and the source control providing the value to map.
        dest->Update(mapping, src);
      }

      // tell all destinations we completed.
      for (size_t i = 0; i < functionCount; ++i)
      {
        mHandlers[i]->EndUpdate();
      }
    }

    void RegisterFunction(ControlMapping::Function f, FunctionHandler *handler)
    {
      CCASSERT((size_t)f < SizeofStaticArray(mHandlers)); // bounds check
      CCASSERT(mHandlers[(size_t)f] == nullptr);          // do not register a handler more than once.
      mHandlers[(size_t)f] = handler;
    }
  };

} // namespace clarinoid
