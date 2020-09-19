#pragma once

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
    BigKeyData lhKeyData = BigKeyData::Deserialize(lh.data.packedKeys);
    BigKeyData rhKeyData = BigKeyData::Deserialize(rh.data.packedKeys);
    this->key_lh1.Update(lhKeyData.keys[1]);
    this->key_lh2.Update(lhKeyData.keys[2]);
    this->key_lh3.Update(lhKeyData.keys[3]);
    this->key_lh4.Update(lhKeyData.keys[4]);

    this->key_octave1.Update(lhKeyData.oct[0]);
    this->key_octave2.Update(lhKeyData.oct[1]);
    this->key_octave3.Update(lhKeyData.oct[2]);
    this->key_octave4.Update(lhKeyData.oct[3]);

    this->key_lhExtra1.Update(lhKeyData.keys[0]);
    this->key_lhExtra2.Update(lhKeyData.keys[5]);
    
    this->key_rh1.Update(rhKeyData.keys[1]);
    this->key_rh2.Update(rhKeyData.keys[2]);
    this->key_rh3.Update(rhKeyData.keys[3]);
    this->key_rh4.Update(rhKeyData.keys[4]);

    this->key_rhExtra1.Update(rhKeyData.keys[0]);
    this->key_rhExtra2.Update(rhKeyData.keys[5]);

    this->breath01 = (float)lh.data.pressure1 / 1024;
    this->bite01 = (float)lh.data.pressure2 / 1024;
    this->pitchDown01 = (float)rh.data.pressure1 / 1024;

    this->key_back.Update(lhKeyData.GetButton1());

    Tristate oldState = key_triState;
    if (rhKeyData.GetButton1()) {
      this->key_triState = Tristate::Position1;
    } else if (rhKeyData.GetButton2()) {
      this->key_triState = Tristate::Position3;
    } else {
      this->key_triState = Tristate::Position2;
    }
    key_triState_is_dirty = oldState != key_triState;
  }
};
