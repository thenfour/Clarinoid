#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "ControlSource.hpp"

namespace clarinoid
{

  struct IEncoderDestination
  {
    virtual void BeginUpdate() = 0;
    virtual void Update(EncoderMapping *pConfig, const EncoderInfo &rawInput) = 0;
    virtual void EndUpdate() = 0;
  };


  struct CompositeEncoderDestination : IEncoder, IEncoderDestination
  {
    virtual void BeginUpdate() override {}
    virtual void Update(EncoderMapping *pConfig, const EncoderInfo &rawInput) override {}
    virtual void EndUpdate() override {}

    virtual EncoderState CurrentValue() const override { return {}; }
  };
} // namespace clarinoid
