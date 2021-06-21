#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "ControlSource.hpp"

namespace clarinoid
{
  // this basically aggregates values in the case that multiple keys are mapped to the same thing.
  struct FunctionHandler
  {
    ControlValue mAggregateValue;

    virtual void BeginUpdate()
    {
      mAggregateValue = FunctionHandler_GetCurrentValue();
    }
    virtual void Update(ControlMapping &mapping, const ControlInfo &src)
    {
      ControlValue out;
      auto used = mapping.UpdateAndMapValue(src.p, out);
      if (!used) {
        return;
      }
      mAggregateValue = ControlMapping::ApplyValue(&mAggregateValue, out, mapping.mOperator);// FunctionHandler_ApplyValue(mAggregateValue, out, mapping.mOperator);
    }
    virtual void EndUpdate()
    {
      FunctionHandler_Update(mAggregateValue);
    }

    // implement in child controls to accept the final value of an input update round.
    virtual ControlValue FunctionHandler_GetCurrentValue() const = 0;
    virtual void FunctionHandler_Update(const ControlValue &) = 0;
  };

  // for controls like OK and Back, it's good if these land into a virtual button.
  // they *could* be actions, but the code is already written with them as buttons so...
  // for keyrhn, octave, i think also good to be a virtual. they are not actions, they are states.
  // for enabling / disabling settings, it's fine if they're functions.
  struct VirtualSwitch : ISwitch, FunctionHandler
  {
    ControlValue mValue;
    virtual bool CurrentValue() const override
    {
      return mValue.AsBool();
    }
    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      mValue = v;
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override { return mValue; }
  };

  struct VirtualAxis : IAnalogAxis, FunctionHandler
  {
    float mValue = 0.0f;
    virtual float CurrentValue01() const override
    {
      return Clamp(mValue, 0.0f, 1.0f);
    }
    float CurrentValueN11() const // pitch bend.
    {
      return Clamp(mValue, -1.0f, 1.0f);
    }
    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      mValue = v.AsFloat01();
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override { return ControlValue::FloatValue(mValue); }
  };

  struct VirtualEncoder : IEncoder, FunctionHandler
  {
    float mValue = 0.0f;
    virtual float CurrentValue() const override
    {
      return mValue;
    }
    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      mValue = v.AsFloat01();
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override { return ControlValue::FloatValue(mValue); }
  };

} // namespace clarinoid
