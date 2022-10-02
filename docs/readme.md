
# Clarinoid

## Links to other docs

* [Development notes, including how to reduce static memory bloat](development.md)
* [Modulation specification](modulation.md)

## Projects

### Clarinoid2
This is for Clarinoid2, not the original prototype #1. I believe that was arduino only, and had separate LH and RH projects.

### Bassoonoid


### Test Device
I don't think this is actually used, but should be now that the test device is working.

### Bommanoid

## Control / input mapping

So i wrote this a while ago (2020) and forgot how it works, several times. Therefore lets document it a bit? I'll use Bassoonoid as an example.

The point is to go from hardware input to something that can be used deep in code. Buttons can perform functions. Pressure sensors and potentiometers will map float value to float value.

One of the reasons the design is more complex than you might think is because I want to support user-editable control mappings. So mappings are unified in a generic way, theoretically configurable at runtime. This is not possible yet but maybe in the future?

### Base system settings / enum PhysicalControl

```c++
    BaseSystemSettings.hpp / 
    enum class PhysicalControl : uint8_t
    {
        CPBack,
        CPOk,
        Breath,
        Pitch,
        ...
        COUNT,
    }

```

### Control mapper

```c++
    struct BommanoidControlMapper : IInputSource, ITask

    virtual void TaskRun() override
    {
        NoInterrupts _ni;
        mEncoder.Update();

    virtual void InputSource_Init(struct InputDelegator *p) override
    {
        mControlInfo[(size_t)PhysicalControl::Enc] = ControlInfo{"ENC", &mEncoder};
```
### IInputSource

```c++
    struct IInputSource
    {
        virtual void InputSource_Init(struct InputDelegator *) = 0;
        virtual size_t InputSource_GetControlCount() = 0;
        virtual ControlInfo InputSource_GetControl(PhysicalControl index) = 0;
        virtual void InputSource_ShowToast(
            const String &s) = 0; // this is a bit odd but allows the input delegator to show toasts to the GUI.
    };
```

### ControlInfo

### App init

Create default control mappings:

```c++
        mInputDelegator.Init(&mAppSettings, &mControlMapper);

        mAppSettings.mControlMappings[0] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHOk, ControlMapping::Function::MenuOK);
        mAppSettings.mControlMappings[1] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHBack, ControlMapping::Function::MenuBack);
```

### InputDelegator

The `InputDelegator` is a generic class which performs mapped actions. While controlmapper associates hardware controls with control info, input function mappings are in app settings. `InputDelegator` will bring all this together. It looks at control mappings in your settings, and applies the functions / param mappings.

Seems simple, but it means there are a set number of fixed "virtual controls" that real controls map to, or other control destinations. All those virtual controls derive from `FunctionHandler` and must implement `Update` and `GetCurrentValue`.

**Note** that this is not an `ITask`; it is not part of the TaskManager cycle. It's updated from `MusicalStateTask`. Why? Not sure really.

```c++
struct InputDelegator
{
    AppSettings *mpAppSettings = nullptr;
    IInputSource *mpSrc = nullptr; // <-- the app-specific control mapper

    VirtualSwitch mMenuOK;
    VirtualEncoder mMenuScrollA;
    VirtualAxis mBreath;
    SynthPresetAMappableFunction mSynthPresetAFn;
    ///...

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
        RegisterFunction(ControlMapping::Function::Nop,
                         &mMenuBack); // anything works; it's never called.

        RegisterFunction(ControlMapping::Function::MenuOK, &mMenuOK);
        RegisterFunction(ControlMapping::Function::MenuScrollA, &mMenuScrollA);
        RegisterFunction(ControlMapping::Function::Breath, &mBreath);
        RegisterFunction(ControlMapping::Function::SynthPresetA, &mSynthPresetAFn);
        ///...
    }

    void Update()
    {
        /// for each input mapping, finds the FunctionHandler for it, and calls fn->Update(mapping, controlmapper)
    }
};

```

### FunctionHandler

```c++
    virtual ControlValue FunctionHandler_GetCurrentValue() const = 0;
    virtual void FunctionHandler_Update(const ControlValue &) = 0;
```


## what happens when you press a button, turn an encoder, or apply breath pressure?

1. `BommanoidControlMapper::TaskRun()` updates the internal value of the control. The internal value is not unified; it's basically managed by the driver itself. So a simple: `mEncoder.Update();`.
1. 