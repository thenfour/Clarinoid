#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

  struct ControlInfo
  {
    const char *Name = "(unnamed)";
    IControl* p;
  };

  // base class for accessing raw hardware input sources. inherited by BassoonoidControlMapper for example.
  struct IInputSource
  {
    virtual void InputSource_Init(struct InputDelegator *) = 0;
    virtual size_t InputSource_GetControlCount() = 0;
    virtual ControlInfo InputSource_GetControl(PhysicalControl index) = 0;
  };

} // namespace clarinoid
