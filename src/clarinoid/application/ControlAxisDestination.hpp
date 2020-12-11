#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "ControlSource.hpp"

namespace clarinoid
{

  struct IAxisDestination
  {
    virtual void BeginUpdate() = 0;
    virtual void Update(AxisMapping *pConfig, const AxisInfo &rawInput) = 0;
    virtual void EndUpdate() = 0;
  };

  struct CompositeAxisDestination : IAnalogAxis, IAxisDestination
  {
    virtual void BeginUpdate() override {}
    virtual void Update(AxisMapping *pConfig, const AxisInfo &rawInput) override {}
    virtual void EndUpdate() override {}

    virtual float CurrentValue01() const override { return 1.0f; }
  };
} // namespace clarinoid
