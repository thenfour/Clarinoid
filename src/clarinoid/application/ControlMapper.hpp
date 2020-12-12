#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "ControlSource.hpp"
#include "FunctionHandler.hpp"

namespace clarinoid
{

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

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
      mpAppSettings = appSettings;
      mpSrc = psrc;

      RegisterFunction(ControlMapping::Function::MenuScrollA, &mMenuScrollA);

      RegisterFunction(ControlMapping::Function::Breath, &mBreath);

      RegisterFunction(ControlMapping::Function::MenuBack, &mMenuBack);
      RegisterFunction(ControlMapping::Function::MenuOK, &mMenuOK);
      RegisterFunction(ControlMapping::Function::Oct1, &mKeyOct1);
      RegisterFunction(ControlMapping::Function::Oct2, &mKeyOct2);
      RegisterFunction(ControlMapping::Function::Oct3, &mKeyOct3);

      RegisterFunction(ControlMapping::Function::LH1, &mKeyLH1);
      RegisterFunction(ControlMapping::Function::LH2, &mKeyLH2);
      RegisterFunction(ControlMapping::Function::LH3, &mKeyLH3);
      RegisterFunction(ControlMapping::Function::LH4, &mKeyLH4);

      RegisterFunction(ControlMapping::Function::RH1, &mKeyRH1);
      RegisterFunction(ControlMapping::Function::RH2, &mKeyRH2);
      RegisterFunction(ControlMapping::Function::RH3, &mKeyRH3);
      RegisterFunction(ControlMapping::Function::RH4, &mKeyRH4);

      mpSrc->InputSource_Init(this);
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

        CCASSERT((size_t)mapping.mFunction < SizeofStaticArray(mHandlers));

        FunctionHandler *dest = mHandlers[(size_t)mapping.mFunction]; // get the function this is mapped to
        auto src = mpSrc->InputSource_GetControl(mapping.mSource); // and the source control providing the value to map.
        mapping.UpdateValue(src.p);
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
