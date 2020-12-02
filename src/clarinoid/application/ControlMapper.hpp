#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{


// generic interface to access mapped controls. similar to "physical state" of yore.

struct IControlMapper
{
  virtual IEncoder* MenuEncoder() { CCASSERT(false); return nullptr; }
  virtual ISwitch* MenuBack() { CCASSERT(false); return nullptr; }
  virtual ISwitch* MenuOK() { CCASSERT(false); return nullptr; }

  virtual IAnalogControl* BreathSensor() { CCASSERT(false); return nullptr; }

  virtual ISwitch* KeyLH1() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyLH2() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyLH3() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyLH4() { CCASSERT(false); return nullptr; }

  virtual ISwitch* KeyRH1() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyRH2() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyRH3() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyRH4() { CCASSERT(false); return nullptr; }

  virtual ISwitch* KeyOct1() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyOct2() { CCASSERT(false); return nullptr; }
  virtual ISwitch* KeyOct3() { CCASSERT(false); return nullptr; }
};

} // namespace clarinoid