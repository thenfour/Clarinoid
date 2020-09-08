#pragma once

#include <Shared_CCSwitch.h>
#include <Shared_CCUtil.h>

struct CCEWIPhysicalState
{
  CCVirtualSwitch key_lh1;
  CCVirtualSwitch key_lh2;
  CCVirtualSwitch key_lh3;
  CCVirtualSwitch key_lh4;

  CCVirtualSwitch key_rh1;
  CCVirtualSwitch key_rh2;
  CCVirtualSwitch key_rh3;
  CCVirtualSwitch key_rh4;

  CCVirtualSwitch key_octave1;
  CCVirtualSwitch key_octave2;
  CCVirtualSwitch key_octave3;
  CCVirtualSwitch key_octave4;

  CCVirtualSwitch key_lhExtra1;
  CCVirtualSwitch key_lhExtra2;
  CCVirtualSwitch key_rhExtra1;
  CCVirtualSwitch key_rhExtra2;

  // match the order of gKeyDesc
  CCVirtualSwitch* mOrderedKeys[16] = {
    &key_lh1,
    &key_lh2,
    &key_lh3,
    &key_lh4,

    &key_rh1,
    &key_rh2,
    &key_rh3,
    &key_rh4,

    &key_octave1,
    &key_octave2,
    &key_octave3,
    &key_octave4,

    &key_lhExtra1,
    &key_lhExtra2,
    &key_rhExtra1,
    &key_rhExtra2,
  };

  float breath01;
  float bite01;
  float pitchDown01;

  // yea these are sorta oddballs; not really sure they should be lumped in here
  // but it's convenient to do so because they come from the LHRH payloads.
  CCVirtualSwitch key_back;
  Tristate key_triState; // same.
  bool key_triState_is_dirty = false;
  
  void Update(const LHRHPayload& lh, const LHRHPayload& rh)
  {
    this->key_lh1.Update(lh.data.keys[1].IsPressed);
    this->key_lh2.Update(lh.data.keys[2].IsPressed);
    this->key_lh3.Update(lh.data.keys[3].IsPressed);
    this->key_lh4.Update(lh.data.keys[4].IsPressed);

    this->key_octave1.Update(lh.data.octaveKeys[0].IsPressed);
    this->key_octave2.Update(lh.data.octaveKeys[1].IsPressed);
    this->key_octave3.Update(lh.data.octaveKeys[2].IsPressed);
    this->key_octave4.Update(lh.data.octaveKeys[3].IsPressed);

    this->key_lhExtra1.Update(lh.data.keys[0].IsPressed);
    this->key_lhExtra2.Update(lh.data.keys[5].IsPressed);
    
    this->key_rh1.Update(rh.data.keys[1].IsPressed);
    this->key_rh2.Update(rh.data.keys[2].IsPressed);
    this->key_rh3.Update(rh.data.keys[3].IsPressed);
    this->key_rh4.Update(rh.data.keys[4].IsPressed);

    this->key_rhExtra1.Update(rh.data.keys[0].IsPressed);
    this->key_rhExtra2.Update(rh.data.keys[5].IsPressed);

    this->breath01 = (float)lh.data.pressure1 / 1024;
    this->bite01 = (float)lh.data.pressure2 / 1024;
    this->pitchDown01 = (float)rh.data.pressure1 / 1024;

    this->key_back.Update(lh.data.button1);

    Tristate oldState = key_triState;
    if (rh.data.button1) {
      this->key_triState = Tristate::Position1;
    } else if (rh.data.button2) {
      this->key_triState = Tristate::Position3;
    } else {
      this->key_triState = Tristate::Position2;
    }
    key_triState_is_dirty = oldState != key_triState;
  }
};
