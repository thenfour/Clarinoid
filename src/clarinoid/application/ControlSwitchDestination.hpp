#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "ControlSource.hpp"

namespace clarinoid
{

  struct IButtonDestination
  {
    virtual void BeginUpdate() = 0;
    virtual void Update(ButtonMapping *pConfig, const SwitchInfo &rawInput) = 0;
    virtual void EndUpdate() = 0;
  };

  // for controls like OK and Back, it's good if these land into a virtual button.
  // they *could* be actions, but the code is already written with them as buttons so...
  // for keyrhn, octave, i think also good to be a virtual. they are not actions, they are states.
  // for enabling / disabling settings, it's fine if they're functions.
  struct CompositeSwitchDestination : ISwitch, IButtonDestination
  {
    bool tempState;
    virtual void BeginUpdate() override { tempState = false; }
    virtual void Update(ButtonMapping *pConfig, const SwitchInfo &rawInput) override
    {
      
    }
    virtual void EndUpdate() override {}

    virtual bool CurrentValue() const override
    {
      return false;
    }
  };

  // // run a function with a switch result
  // struct FunctionSwitchDestination : ISwitch, IButtonDestination
  // {
  //   virtual void BeginUpdate() override {}
  //   virtual void Update(ButtonMapping *pConfig, const SwitchInfo &rawInput) override
  //   {
  //     //
  //   }
  //   virtual void EndUpdate() = 0;

  //   virtual bool CurrentValue() const override
  //   {
  //     return false;
  //   }
  // };

} // namespace clarinoid
