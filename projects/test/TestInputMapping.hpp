

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Encoder.hpp>
#include <clarinoid/components/Potentiometer.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/application/ControlMapper.hpp>

namespace clarinoid
{
  struct TestButton : ISwitch
  {
    bool mValue = false;
    virtual bool CurrentValue() const override { return mValue; }
  };
  struct TestAxis : IAnalogAxis
  {
    float mValue = 0;
    virtual float CurrentValue01() const override { return mValue; }
  };
  struct TestEncoder : IEncoder
  {
    float mValue = 0;
    virtual float CurrentValue() const override { return mValue; }
  };

  struct TestControlMapper :
    IInputSource,
    ITask
  {
    virtual void TaskRun() override
    {
    }

    virtual void InputSource_Init(struct InputDelegator* p) override
    {
      mControlInfo[(size_t)PhysicalControl::Encoder1] = ControlInfo{ "Encoder1", &mEncoder1 };
      mControlInfo[(size_t)PhysicalControl::Encoder2] = ControlInfo{ "Encoder2", &mEncoder2 };
      mControlInfo[(size_t)PhysicalControl::Axis1] = ControlInfo{ "Axis1", &mAxis1 };
      mControlInfo[(size_t)PhysicalControl::Axis2] = ControlInfo{ "Axis2", &mAxis2 };
      mControlInfo[(size_t)PhysicalControl::Button1] = ControlInfo{ "Button1", &mButton1 };
      mControlInfo[(size_t)PhysicalControl::Button2] = ControlInfo{ "Button2", &mButton2 };
    }

    virtual size_t InputSource_GetControlCount() override { return SizeofStaticArray(mControlInfo); }
    virtual ControlInfo InputSource_GetControl(PhysicalControl index) override
    {
      CCASSERT((size_t)index < SizeofStaticArray(mControlInfo));
      return mControlInfo[(size_t)index];
    }

    virtual void InputSource_ShowToast(const String& s) override
    {
      log("TOAST : %s", s.mStr.str().c_str());
    }

    TestEncoder mEncoder1;
    TestEncoder mEncoder2;
    TestAxis mAxis1;
    TestAxis mAxis2;
    TestButton mButton1;
    TestButton mButton2;

    ControlInfo mControlInfo[(size_t)PhysicalControl::COUNT];
  };



  void TestInputDelegator()
  {
    {
      InputDelegator id;
      TestControlMapper tcm;
      AppSettings as;

      id.Init(&as, &tcm);

      // we want to test the following overarching scenarios, 
      // and then test detailed scenarios like multiple mapping, mixing datatypes etc.

      // octave key test with trigger down test
      as.mControlMappings[0].mSource = PhysicalControl::Button1;
      as.mControlMappings[0].mFunction = ControlMapping::Function::Oct1;
      as.mControlMappings[0].mModifier = ModifierKey::Any;
      as.mControlMappings[0].mOperator = ControlMapping::Operator::Set;
      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::Passthrough;

      TestEq(id.mKeyOct1.CurrentValue(), false); // ensure default state
      tcm.mButton1.mValue = true;
      id.Update();
      TestEq(id.mKeyOct1.CurrentValue(), true); // because the mapping is "passthrough", the value will be true without neediing to be "triggered".

      // but if we use Trigger style mapping, then it actually needs to be *triggered* to set.
      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::TriggerDownValue;
      as.mControlMappings[0].mValueArray[0] = 0.0f;
      as.mControlMappings[0].mTriggerBelowValue = 0.5f;

      id.mKeyOct1.mValue = false; // let's artificially change this value to false, and watch how even though the button is pressed, it won't affect because trigger condition is not met.
      TestEq(id.mKeyOct1.CurrentValue(), false); // double check
      TestEq(tcm.mButton1.mValue, true);
      id.Update(); // updating now should NOT affect 
      TestEq(id.mKeyOct1.CurrentValue(), false); // and check that no trigger condition happened.

      // but now releasing the button will trigger down.
      tcm.mButton1.mValue = false;
      id.Update();
      TestEq(id.mKeyOct1.CurrentValue(), false); // and check that a trigger condition happened.

      // modifier key test.
      as.mControlMappings[0] = {};
      as.mControlMappings[0].mSource = PhysicalControl::Button1;
      as.mControlMappings[0].mFunction = ControlMapping::Function::ModifierShift;
      as.mControlMappings[0].mModifier = ModifierKey::Any; // this is kinda important
      as.mControlMappings[0].mOperator = ControlMapping::Operator::Set;
      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::Passthrough;

      tcm.mButton1.mValue = true; // holding shift now.
      id.Update();
      TestEq(id.mModifierShift.CurrentValue(), true);

      // add a mapping with modifier
      as.mControlMappings[1] = {};
      as.mControlMappings[1].mSource = PhysicalControl::Button2;
      as.mControlMappings[1].mFunction = ControlMapping::Function::Oct1;
      as.mControlMappings[1].mModifier = ModifierKey::None;
      as.mControlMappings[1].mOperator = ControlMapping::Operator::Set;
      as.mControlMappings[1].mStyle = ControlMapping::MapStyle::Passthrough;

      tcm.mButton2.mValue = true; // holding OCT1 now. but because the modifier is NONE, it won't work.

      id.Update();
      TestEq(id.mKeyOct1.CurrentValue(), false);

      as.mControlMappings[1].mModifier = ModifierKey::Shift;
      id.Update();
      TestEq(id.mModifierShift.CurrentValue(), true);
      TestEq(id.mKeyOct1.CurrentValue(), true);

      // encoder. let's have multiple encoders scrolling the menu
      id.mMenuScrollA.mValue = 0.0f;

      as.mControlMappings[0] = {};
      as.mControlMappings[0].mSource = PhysicalControl::Encoder1;
      as.mControlMappings[0].mDeltaScale = 1.0f;
      as.mControlMappings[0].mFunction = ControlMapping::Function::MenuScrollA;
      as.mControlMappings[0].mModifier = ModifierKey::None;
      as.mControlMappings[0].mOperator = ControlMapping::Operator::Add;
      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::DeltaWithScale;

      as.mControlMappings[1] = {};

      id.ResetModifiers();

      tcm.mEncoder1.mValue = 0.0f; // in order to get a delta value, start with 0.
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 0.0f));

      tcm.mEncoder1.mValue = 1.5f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 1.5f));

      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 1.5f)); // test that delta 0 doesn't change the val.

      tcm.mEncoder1.mValue = 0.0f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 0.0f)); // test that negative delta works.

      // test delta scale.
      as.mControlMappings[0].mDeltaScale = .25f;
      tcm.mEncoder1.mValue = 1.0f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 0.25f));

      tcm.mEncoder1.mValue = 2.0f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 0.5f));

      tcm.mEncoder1.mValue = 3.0f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 0.75f));

      tcm.mEncoder1.mValue = 4.0f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 1.0f));

      // negative delta scale
      as.mControlMappings[0].mReader.Reset();
      as.mControlMappings[0].mDeltaScale = -2.0f;
      tcm.mEncoder1.mValue = 0.0f;
      id.Update();
      tcm.mEncoder1.mValue = 0.5f;
      id.mMenuScrollA.mValue = 0.0f; // reset scroll
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, -1.0f));


      // multiple encoders assigned to same function. this proves a "course" / "fine" modifiers can work with an encoder.
      as.mControlMappings[0] = {};
      as.mControlMappings[0].mSource = PhysicalControl::Encoder1;
      as.mControlMappings[0].mFunction = ControlMapping::Function::MenuScrollA;
      as.mControlMappings[0].mModifier = ModifierKey::None;
      as.mControlMappings[0].mOperator = ControlMapping::Operator::Add;
      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::DeltaWithScale;

      as.mControlMappings[1] = {};
      as.mControlMappings[1].mSource = PhysicalControl::Encoder2;
      as.mControlMappings[1].mFunction = ControlMapping::Function::MenuScrollA;
      as.mControlMappings[1].mModifier = ModifierKey::None;
      as.mControlMappings[1].mOperator = ControlMapping::Operator::Add;
      as.mControlMappings[1].mStyle = ControlMapping::MapStyle::DeltaWithScale;

      tcm.mEncoder1.mValue = 0.0f; // reset encoder 1 state so next value is delta
      as.mControlMappings[0].mReader.Reset();
      as.mControlMappings[0].mDeltaScale = 2.0f;

      tcm.mEncoder2.mValue = 0.0f; // reset encoder 1 state so next value is delta
      as.mControlMappings[1].mReader.Reset();
      as.mControlMappings[1].mDeltaScale = -.5f;

      id.mMenuScrollA.mValue = 0.0f; // reset scroll
      id.Update(); // establish the 0 state.
      Test(FloatEquals(id.mMenuScrollA.mValue, 0.0f));

      // move encoder1
      tcm.mEncoder1.mValue = 11.5f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 23.0f));

      tcm.mEncoder2.mValue = -3.0f;
      id.Update();
      Test(FloatEquals(id.mMenuScrollA.mValue, 24.5f));


      // unipolar axis test (breath, curve)
      as.mControlMappings[0] = {};
      as.mControlMappings[1] = {};

      as.mControlMappings[0].mSource = PhysicalControl::Axis1;
      as.mControlMappings[0].mFunction = ControlMapping::Function::Breath;
      as.mControlMappings[0].mModifier = ModifierKey::Any;
      as.mControlMappings[0].mOperator = ControlMapping::Operator::Set; // if you had 2 breath controllers
      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::Passthrough;

      // start at 0.
      id.mBreath.mValue = 0.0f;
      tcm.mAxis1.mValue = 0.0f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 0.0f));

      tcm.mAxis1.mValue = 0.22f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 0.22f));

      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::RemapUnipolar;
      as.mControlMappings[0].mNPolarMapping.mNegative = UnipolarMapping{ 0.2f, 0.8f, 2.0f, 5.0f, .5f, 0.0f };

      tcm.mAxis1.mValue = 0.0f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 2.0f));

      tcm.mAxis1.mValue = 0.2f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 2.0f));

      tcm.mAxis1.mValue = 0.21f;
      id.Update();
      Test(id.mBreath.mValue > 2.0f);

      tcm.mAxis1.mValue = 1.0f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 5.0f));

      tcm.mAxis1.mValue = 0.8f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 5.0f));

      tcm.mAxis1.mValue = 0.79f;
      id.Update();
      Test(id.mBreath.mValue < 5.0f);

      // add a bipolar axis to add to breath. now this is getting pretty extreme.
      // the "operator" concept is a compromise for the sake of simplicity... in an ideal perfect system
      // you'd have a complete formula to configure like FUNCTION = set(prev_value, (axis1 + axis2));
      // but yea ... so it will be FUNCTION = set(prev_value, axis1) + axis2. bceause of the way it's calculated.
      as.mControlMappings[1].mSource = PhysicalControl::Axis2;
      as.mControlMappings[1].mFunction = ControlMapping::Function::PitchBend;
      as.mControlMappings[1].mModifier = ModifierKey::Any;
      as.mControlMappings[1].mOperator = ControlMapping::Operator::Set;
      as.mControlMappings[1].mStyle = ControlMapping::MapStyle::RemapBipolar;

      as.mControlMappings[1].mNPolarMapping.mNegative = UnipolarMapping{ 0.1f, 0.4f, -1.0f, 0.0f, .5f, 0.0f };
      as.mControlMappings[1].mNPolarMapping.mPositive = UnipolarMapping{ 0.6f, 0.9f, 0.0f, 1.0f, .5f, 0.0f };

      // first just see if this works by assigning it to pitch bend.
      tcm.mAxis2.mValue = 0.0f;
      id.Update();
      Test(FloatEquals(id.mPitchBend.mValue, -1.0f));

      tcm.mAxis2.mValue = 0.1f;
      id.Update();
      Test(FloatEquals(id.mPitchBend.mValue, -1.0f));

      tcm.mAxis2.mValue = 0.4f;
      id.Update();
      Test(FloatEquals(id.mPitchBend.mValue, 0.0f));

      tcm.mAxis2.mValue = 0.5f;
      id.Update();
      Test(FloatEquals(id.mPitchBend.mValue, 0.0f));

      tcm.mAxis2.mValue = 0.6f;
      id.Update();
      Test(FloatEquals(id.mPitchBend.mValue, 0.0f));

      tcm.mAxis2.mValue = 0.9f;
      id.Update();
      Test(FloatEquals(id.mPitchBend.mValue, 1.0f));

      tcm.mAxis2.mValue = 1.0f;
      id.Update();
      Test(FloatEquals(id.mPitchBend.mValue, 1.0f));

      // now map it to interact with the breath controller to check this behavior.
      as.mControlMappings[1].mFunction = ControlMapping::Function::Breath;
      as.mControlMappings[1].mOperator = ControlMapping::Operator::Add;

      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::RemapUnipolar;
      as.mControlMappings[0].mNPolarMapping.mNegative = UnipolarMapping{ 0.1f, 0.9f, 0.0f, 1.0f, .5f, 0.0f };

      tcm.mAxis1.mValue = 0.5f;
      tcm.mAxis2.mValue = 0.5f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 0.5f));

      tcm.mAxis1.mValue = 0.5f;
      tcm.mAxis2.mValue = 0.0f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, -.5f));

      tcm.mAxis1.mValue = 0.5f;
      tcm.mAxis2.mValue = 1.0f;
      id.Update();
      Test(FloatEquals(id.mBreath.mValue, 1.5f));

      // test multiple buttons mapped to "menu back". the point is that 
      // triggers will play nicely together resulting in a kind of boolean "or" behavior
      // or a refcounting sorta idea.
      as.mControlMappings[0] = {};
      as.mControlMappings[1] = {};

      as.mControlMappings[0].mSource = PhysicalControl::Button1;
      as.mControlMappings[0].mFunction = ControlMapping::Function::MenuBack;
      as.mControlMappings[0].mModifier = ModifierKey::Any;
      as.mControlMappings[0].mOperator = ControlMapping::Operator::Add;
      as.mControlMappings[0].mStyle = ControlMapping::MapStyle::TriggerUpDownValue;
      as.mControlMappings[0].mValueArray[0] = 1.0f;
      as.mControlMappings[0].mValueArray[1] = -1.0f;

      as.mControlMappings[1].mSource = PhysicalControl::Button2;
      as.mControlMappings[1].mFunction = ControlMapping::Function::MenuBack;
      as.mControlMappings[1].mModifier = ModifierKey::Any;
      as.mControlMappings[1].mOperator = ControlMapping::Operator::Add;
      as.mControlMappings[1].mStyle = ControlMapping::MapStyle::TriggerUpDownValue;
      as.mControlMappings[1].mValueArray[0] = 1.0f;
      as.mControlMappings[1].mValueArray[1] = -1.0f;

      // start unpressed
      tcm.mButton1.mValue = false;
      tcm.mButton2.mValue = false;
      SwitchControlReader backReader;
      id.Update();
      backReader.Update(&id.mMenuBack);
      Test(!id.mMenuBack.CurrentValue());
      Test(!backReader.IsNewlyPressed());
      Test(!backReader.IsNewlyUnpressed());

      // user presses button1, invoking trigger
      tcm.mButton1.mValue = true;
      id.Update();
      backReader.Update(&id.mMenuBack);
      Test(id.mMenuBack.CurrentValue());
      Test(backReader.IsNewlyPressed());
      Test(!backReader.IsNewlyUnpressed());

      // continues holding... no trigger but the value remains in pressed state.
      tcm.mButton1.mValue = true;
      id.Update();
      backReader.Update(&id.mMenuBack);
      Test(id.mMenuBack.CurrentValue());
      Test(!backReader.IsNewlyPressed());
      Test(!backReader.IsNewlyUnpressed());

      // releases button1, it's no longer pressed state.
      tcm.mButton1.mValue = false;
      id.Update();
      backReader.Update(&id.mMenuBack);
      Test(!id.mMenuBack.CurrentValue());
      Test(!backReader.IsNewlyPressed());
      Test(backReader.IsNewlyUnpressed());

      // continues released. remain unpressed.
      tcm.mButton1.mValue = false;
      id.Update();
      backReader.Update(&id.mMenuBack);
      Test(!id.mMenuBack.CurrentValue());
      Test(!backReader.IsNewlyPressed());
      Test(!backReader.IsNewlyUnpressed());

      // user presses button1 again
      tcm.mButton1.mValue = true;
      id.Update(); // it's now newly pressed.
      id.Update();// remains pressed.
      Test(id.mMenuBack.CurrentValue());
      backReader.Update(&id.mMenuBack);

      // user now adds button2.
      tcm.mButton2.mValue = true;
      id.Update();
      backReader.Update(&id.mMenuBack);
      Test(id.mMenuBack.CurrentValue());
      Test(!backReader.IsNewlyPressed());
      Test(!backReader.IsNewlyUnpressed());

      // user now releases button2. it should remain pressed because of the 1st button!
      tcm.mButton2.mValue = false;
      id.Update();
      backReader.Update(&id.mMenuBack);
      Test(id.mMenuBack.CurrentValue());
      Test(!backReader.IsNewlyPressed());
      Test(!backReader.IsNewlyUnpressed());
    }

    // test "transpose" style mapping.
    {
      InputDelegator id;
      TestControlMapper tcm;
      AppSettings as;

      id.Init(&as, &tcm);

      as.mControlMappings[0] = ControlMapping::ButtonIncrementMapping(PhysicalControl::Button1, ControlMapping::Function::SynthPreset, 1.0f);

      tcm.mButton1.mValue = false;
      as.mGlobalSynthPreset = 0; // ensure initial state
      id.Update();
      TestEq(as.mGlobalSynthPreset, 0); // not triggered yet.

      tcm.mButton1.mValue = true;
      id.Update();
      TestEq(as.mGlobalSynthPreset, 1); // triggered

      tcm.mButton1.mValue = true;
      id.Update();
      TestEq(as.mGlobalSynthPreset, 1); // not triggered 

      tcm.mButton1.mValue = false;
      id.Update();
      TestEq(as.mGlobalSynthPreset, 1); // not triggered

      tcm.mButton1.mValue = true;
      id.Update();
      TestEq(as.mGlobalSynthPreset, 2); // triggered
    }
  }

} // namespace clarinoid




