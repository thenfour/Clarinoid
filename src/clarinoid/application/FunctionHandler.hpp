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
    size_t mValuesRead = 0;
    virtual void BeginUpdate()
    {
      mValuesRead = 0;
    }
    virtual void Update(ControlMapping &mapping, const ControlInfo &src)
    {
      ControlValue out;
      auto used = mapping.MapValue(src.p->GetControlValue(), out);
      if (!used)
        return;

      if (mValuesRead == 0)
      {
        mAggregateValue = out;
        mValuesRead = 1;
        return;
      }

      mAggregateValue = ControlMapping::ApplyValue(mAggregateValue, out, mapping.mOperator);// FunctionHandler_ApplyValue(mAggregateValue, out, mapping.mOperator);
    }
    virtual void EndUpdate()
    {
      FunctionHandler_Update(mAggregateValue);
    }

    // implement in child controls to accept the final value of an input update round.
    //virtual ControlValue FunctionHandler_ApplyValue(const ControlValue &a, const ControlValue &b, ControlMapping::Operator op) = 0;
    virtual void FunctionHandler_Update(const ControlValue &) = 0;
  };

  // for controls like OK and Back, it's good if these land into a virtual button.
  // they *could* be actions, but the code is already written with them as buttons so...
  // for keyrhn, octave, i think also good to be a virtual. they are not actions, they are states.
  // for enabling / disabling settings, it's fine if they're functions.
  struct VirtualSwitch : ISwitch, FunctionHandler
  {
    bool mValue = false;
    virtual bool CurrentValue() const override
    {
      return mValue;
    }
    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      mValue = v.AsBool();
    }
  };

  struct VirtualAxis : IAnalogAxis, FunctionHandler
  {
    float mValue;
    virtual float CurrentValue01() const override
    {
      return mValue;
    }
    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      mValue = v.AsFloat01();
    }
  };

  struct VirtualEncoder : IEncoder, FunctionHandler
  {
    float mValue;
    virtual float CurrentValue() const override
    {
      return mValue;
    }
    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
      mValue = v.AsFloat01();
    }
  };

} // namespace clarinoid
