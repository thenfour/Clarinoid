#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "ControlSource.hpp"
#include "ControlSwitchDestination.hpp"
#include "ControlAxisDestination.hpp"
#include "ControlEncoderDestination.hpp"

namespace clarinoid
{
  struct InputDelegator
  {
    AppSettings *mpAppSettings = nullptr;
    IInputSource *mpSrc = nullptr;

    IButtonDestination *mButtonDestinations[(size_t)ButtonMapping::Destination::COUNT] = {nullptr};
    IAxisDestination *mAxisDestinations[(size_t)AxisMapping::Destination::COUNT] = {nullptr};
    IEncoderDestination *mEncoderDestinations[(size_t)EncoderMapping::Destination::COUNT] = {nullptr};

    // CompositeSwitchDestination mNopSwitch;
    // CompositeAxisDestination mNopAxis;
    // CompositeEncoderDestination mNopEncoder;

    // for convenience, we can put very-common composite inputs right here. avoids having to pass around stuff
    // everywhere in the app like breath.
    CompositeSwitchDestination mMenuBack;
    CompositeSwitchDestination mMenuOK;
    CompositeEncoderDestination mMenuScrollA;

    CompositeSwitchDestination mKeyLH1;
    CompositeSwitchDestination mKeyLH2;
    CompositeSwitchDestination mKeyLH3;
    CompositeSwitchDestination mKeyLH4;

    CompositeSwitchDestination mKeyRH1;
    CompositeSwitchDestination mKeyRH2;
    CompositeSwitchDestination mKeyRH3;
    CompositeSwitchDestination mKeyRH4;

    CompositeSwitchDestination mKeyOct1;
    CompositeSwitchDestination mKeyOct2;
    CompositeSwitchDestination mKeyOct3;

    CompositeAxisDestination mBreath;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
      mpAppSettings = appSettings;
      mpSrc = psrc;

      //RegisterDestination(EncoderMapping::Destination::Nop, &mNopEncoder);
      RegisterDestination(EncoderMapping::Destination::MenuScroll, &mMenuScrollA);
      //RegisterDestination(AxisMapping::Destination::Nop, &mNopAxis);
      RegisterDestination(AxisMapping::Destination::Breath, &mBreath);

      //RegisterDestination(ButtonMapping::Destination::Nop, &mNopSwitch);
      RegisterDestination(ButtonMapping::Destination::MenuBack, &mMenuBack);
      RegisterDestination(ButtonMapping::Destination::MenuOK, &mMenuOK);
      RegisterDestination(ButtonMapping::Destination::Oct1, &mKeyOct1);
      RegisterDestination(ButtonMapping::Destination::Oct2, &mKeyOct2);
      RegisterDestination(ButtonMapping::Destination::Oct3, &mKeyOct3);

      RegisterDestination(ButtonMapping::Destination::LH1, &mKeyLH1);
      RegisterDestination(ButtonMapping::Destination::LH2, &mKeyLH2);
      RegisterDestination(ButtonMapping::Destination::LH3, &mKeyLH3);
      RegisterDestination(ButtonMapping::Destination::LH4, &mKeyLH4);

      RegisterDestination(ButtonMapping::Destination::RH1, &mKeyRH1);
      RegisterDestination(ButtonMapping::Destination::RH2, &mKeyRH2);
      RegisterDestination(ButtonMapping::Destination::RH3, &mKeyRH3);
      RegisterDestination(ButtonMapping::Destination::RH4, &mKeyRH4);

      mpSrc->InputSource_Init(this);
    }

    void UpdateButtons()
    {
      size_t hardwareCount = mpSrc->InputSource_GetSwitchCount();
      for (size_t i = 0; i < hardwareCount; ++i)
      {
        CCASSERT(mButtonDestinations[i] != nullptr);
        mButtonDestinations[i]->BeginUpdate();
      }

      for (size_t i = 0; i < SizeofStaticArray(mpAppSettings->mButtonMappings); ++i)
      {
        auto &mapping = mpAppSettings->mButtonMappings[i];
        if (mapping.mSourceBehavior == ButtonMapping::Behavior::Nop || mapping.mDestination == ButtonMapping::Destination::Nop)
          continue;

        CCASSERT((size_t)mapping.mDestination < SizeofStaticArray(mButtonDestinations));

        auto src = mpSrc->InputSource_GetSwitch(mapping.mSource);

        IButtonDestination *dest = mButtonDestinations[(size_t)mapping.mDestination];

        dest->Update(&mapping, src);
      }

      // tell all destinations we completed.
      for (size_t i = 0; i < hardwareCount; ++i)
      {
        mButtonDestinations[i]->EndUpdate();
      }
    }

    void UpdateAxes()
    {
      size_t hardwareCount = mpSrc->InputSource_GetAxisCount();
      for (size_t i = 0; i < hardwareCount; ++i)
      {
        CCASSERT(mAxisDestinations[i] != nullptr);
        mAxisDestinations[i]->BeginUpdate();
      }

      for (size_t i = 0; i < SizeofStaticArray(mpAppSettings->mAxisMappings); ++i)
      {
        auto &mapping = mpAppSettings->mAxisMappings[i];
        if (mapping.mSourceBehavior == AxisMapping::Behavior::Nop || mapping.mDestination == AxisMapping::Destination::Nop)
          continue;

        CCASSERT((size_t)mapping.mDestination < SizeofStaticArray(mAxisDestinations));

        auto src = mpSrc->InputSource_GetAxis(mapping.mSource);

        IAxisDestination *dest = mAxisDestinations[(size_t)mapping.mDestination];

        dest->Update(&mapping, src);
      }

      // tell all destinations we completed.
      for (size_t i = 0; i < hardwareCount; ++i)
      {
        mAxisDestinations[i]->EndUpdate();
      }
    }

    void UpdateEncoders()
    {
      size_t hardwareCount = mpSrc->InputSource_GetEncoderCount();
      for (size_t i = 0; i < hardwareCount; ++i)
      {
        CCASSERT(mEncoderDestinations[i] != nullptr);
        mEncoderDestinations[i]->BeginUpdate();
      }

      for (size_t i = 0; i < SizeofStaticArray(mpAppSettings->mEncoderMappings); ++i)
      {
        auto &mapping = mpAppSettings->mEncoderMappings[i];
        if (mapping.mDestination == EncoderMapping::Destination::Nop)
          continue;

        CCASSERT((size_t)mapping.mDestination < SizeofStaticArray(mEncoderDestinations));

        auto src = mpSrc->InputSource_GetEncoder(mapping.mSource);

        IEncoderDestination *dest = mEncoderDestinations[(size_t)mapping.mDestination];

        dest->Update(&mapping, src);
      }

      // tell all destinations we completed.
      for (size_t i = 0; i < hardwareCount; ++i)
      {
        mEncoderDestinations[i]->EndUpdate();
      }
    }

    // process all input state and delegate to handlers.
    void Update()
    {
      CCASSERT(!!mpSrc);
      CCASSERT(!!this->mpAppSettings);

      UpdateButtons();
      UpdateAxes();
      UpdateEncoders();
    }

    // subscribe to control updates. handlers are called is inputs are processed.
    void RegisterDestination(ButtonMapping::Destination d, IButtonDestination *handler)
    {
      CCASSERT((size_t)d < SizeofStaticArray(mButtonDestinations)); // bounds check
      CCASSERT(mButtonDestinations[(size_t)d] == nullptr);          // do not register a handler more than once.
    }
    void RegisterDestination(AxisMapping::Destination d, IAxisDestination *handler)
    {
      CCASSERT((size_t)d < SizeofStaticArray(mAxisDestinations)); // bounds check
      CCASSERT(mAxisDestinations[(size_t)d] == nullptr);          // do not register a handler more than once.
    }
    void RegisterDestination(EncoderMapping::Destination d, IEncoderDestination *handler)
    {
      CCASSERT((size_t)d < SizeofStaticArray(mEncoderDestinations)); // bounds check
      CCASSERT(mEncoderDestinations[(size_t)d] == nullptr);          // do not register a handler more than once.
    }
  };

} // namespace clarinoid
