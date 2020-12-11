#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

  // Info that a hardware control mapper can provide about controls.
  struct SwitchInfo
  {
    const char *Name = "(unnamed)";
    ISwitch *p;
  };
  struct AxisInfo
  {
    const char *Name = "(unnamed)";
    IAnalogAxis *p;
  };
  struct EncoderInfo
  {
    const char *Name = "(unnamed)";
    IEncoder *p;
  };

  // base class for accessing raw hardware input sources. inherited by BassoonoidControlMapper for example.
  struct IInputSource
  {
    virtual void InputSource_Init(struct InputDelegator *) = 0;
    virtual size_t InputSource_GetSwitchCount() = 0;
    virtual SwitchInfo InputSource_GetSwitch(PhysicalSwitch index) = 0;
    virtual size_t InputSource_GetAxisCount() = 0;
    virtual AxisInfo InputSource_GetAxis(PhysicalAxis index) = 0;
    virtual size_t InputSource_GetEncoderCount() = 0;
    virtual EncoderInfo InputSource_GetEncoder(PhysicalEncoder index) = 0;
  };

} // namespace clarinoid
