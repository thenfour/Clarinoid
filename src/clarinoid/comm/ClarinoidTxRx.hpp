// nevermind messy code; still in prototyping mindset.

#pragma once


#include "../basic/Basic.hpp"

#define CCEWI_BAUD 500000 // https://www.pjrc.com/teensy/td_uart.html // 

#ifndef RX_BUFFER_SIZE
# define RX_BUFFER_SIZE 64
#endif

// some things are here that should probably not be ...
// but basically the arduino is such a horrible primitive IDE that it feels more natural and fluent to do stuff like this than to properly structure things.
enum class LHRHLEDMode : uint8_t {
  Debug = 0,
  Minimal = 1,
  MainControlled = 2,
  Off = 3
};

// stagger groups are bitfields of 6 bits. each bit describes when to sample the key.
// so B000000 will never sample.
// B001001 will sample once every 3rd frame (no, no, yes, no, no, yes)
struct KeyDesc {
  KeyDesc(const char *name, bool lh, uint8_t staggerGroup) :
    mName(name),
    mLH(lh),
    mStaggerGroup(staggerGroup)
  {
  }
  const char *mName;
  bool mLH;// lh/rh
  bool mStaggerGroup;
};

KeyDesc gKeyDesc[16] = {
  { "LH1", true, B101010 },// 0
  { "LH2", true, B010101 },// 1
  { "LH3", true, B101010 },
  { "LH4", true, B010101 },
  { "RH1", false, B101010 },// 4
  { "RH2", false, B010101 },
  { "RH3", false, B101010 },
  { "RH4", false, B010101 },
  { "O1", true, B010101 }, // 8
  { "O2", true, B101010 },
  { "O3", true, B010101 },
  { "O4", true, B101010 },
  { "LHx1", true, B001001 }, // 12
  { "LHx2", true, B010010 },
  { "RHx1", false, B001001 }, // 14
  { "RHx2", false, B010010 },
};
#define KEY_LH1 0
#define KEY_LH2 1
#define KEY_LH3 2
#define KEY_LH4 3
#define KEY_RH1 4
#define KEY_RH2 5
#define KEY_RH3 6
#define KEY_RH4 7
#define KEY_O1 8
#define KEY_O2 9
#define KEY_O3 10
#define KEY_O4 11
#define KEY_LHX1 12
#define KEY_LHX2 13
#define KEY_RHX1 14
#define KEY_RHX2 15


// important to keep this well under HardwareSerial.cpp:RX_BUFFER_SIZE/2.
// We want there ALWAYS to be a complete packet in the buffer, not split up.
// and consider that the buffer could have like
// 2. [whole packet]
// 3. [99% of a packet]
// so it must be able to hold 2 packets. space is therefore a premium.

////////////////////////////////////////////////////////////////////////////////
// struct CapTouchKeyData
// {
//   bool IsPressed : 8;
// };

struct BigKeyData
{
  uint8_t keys[6] = {0}; // 0 or 1
  uint8_t oct[6] = {0}; // 0 or 1; 4 octave keys, b1 and b2

  bool GetButton1() const { return oct[4];}
  void SetButton1(bool b) { oct[4] = (b ? 1 : 0);}

  bool GetButton2() const { return oct[5];}
  void SetButton2(bool b) { oct[5] = (b ? 1 : 0);}

  bool GetOctave(size_t n) const { return oct[n];}
  void SetOctave(size_t n, bool b) { oct[n] = (b ? 1 : 0);}

  bool GetKey(size_t n) const { return keys[n];}
  void SetKey(size_t n, bool b) { keys[n] = (b ? 1 : 0);}

  uint16_t Serialize() {
    uint16_t b1 = keys[0] | (keys[1] << 1) | (keys[2] << 2) | (keys[3] << 3) | (keys[4] << 4) | (keys[5] << 5);
    uint16_t b2 = oct[0] | (oct[1] << 1) | (oct[2] << 2) | (oct[3] << 3) | (oct[4] << 4) | (oct[5] << 5);
    return (b1 << 8) | b2;
  }
  static BigKeyData Deserialize(uint16_t d) {
    BigKeyData ret;
    uint16_t b1 = d >> 8;
    ret.keys[0] = b1 & 1;
    ret.keys[1] = b1 & 2;
    ret.keys[2] = b1 & 4;
    ret.keys[3] = b1 & 8;
    ret.keys[4] = b1 & 16;
    ret.keys[5] = b1 & 32;
    uint16_t b2 = d & 0xff;
    ret.oct[0] = b2 & 1;
    ret.oct[1] = b2 & 2;
    ret.oct[2] = b2 & 4;
    ret.oct[3] = b2 & 8;
    ret.oct[4] = b2 & 16;
    ret.oct[5] = b2 & 32;
    return ret;
  }
};

//static constexpr size_t gsaou = sizeof(BigKeyData);

////////////////////////////////////////////////////////////////////////////////
struct LHRHChecksummablePayload
{
  uint16_t packedKeys; // serialized version of BigKeyData

  int8_t focusedKey; // index into gKeyDesc
  uint16_t focusedTouchReadMicros;
  uint16_t focusedTouchReadValue;
  uint16_t focusedTouchReadUntouchedMicros;
  uint16_t focusedTouchReadThresholdMicros;

  uint16_t pressure1; // raw analog readings
  uint16_t pressure2; // raw analog readings
};

// payload is the same between both LH/RH modules.
struct LHRHPayload
{
  LHRHChecksummablePayload data;
  
  uint16_t framerate;
  uint16_t serial;
  uint16_t dataChecksum;
};

static constexpr size_t LHRHChecksummablePayloadsize =  sizeof(LHRHChecksummablePayload);
static constexpr size_t LHRHPayloadsize =  sizeof(LHRHPayload);


inline String ToString(const LHRHPayload& p)
{
  char format[800];
  BigKeyData keys = BigKeyData::Deserialize(p.data.packedKeys);
  sprintf(format, "s#:[%d] chk:[%04x] d:[k%c%c%c%c%c%c o%c%c%c%c b%c%c p:%d p:%d]",
    (int)p.serial,
    (int)p.dataChecksum,
    keys.keys[0] ? '0' : '-',
    keys.keys[1] ? '1' : '-',
    keys.keys[2] ? '2' : '-',
    keys.keys[3] ? '3' : '-',
    keys.keys[4] ? '4' : '-',
    keys.keys[5] ? '5' : '-',

    keys.oct[0] ? '0' : '-',
    keys.oct[1] ? '1' : '-',
    keys.oct[2] ? '2' : '-',
    keys.oct[3] ? '3' : '-',

    keys.oct[4] ? '1' : '-',
    keys.oct[5] ? '2' : '-',

    p.data.pressure1,
    p.data.pressure2
    );
  return format;
}

uint16_t CalcChecksum(const LHRHPayload& p)
{
  static FastCRC16 CRC16;
  return CRC16.x25((const uint8_t*)&p.data, sizeof(p.data));
}

////////////////////////////////////////////////////////////////////////////////
enum class CommandFromMain : uint8_t
{
  None = 0,
  ResetTouchKeys = 1,
  EnableOrangeLED = 2,
  DisableOrangeLED = 3,
  SetTouchMaxFactor = 4,
};

struct MainChecksummablePayload
{
  CommandFromMain cmd;
  float cmdFloatParam1;
  LHRHLEDMode ledMode;
  uint8_t leds[10][3];
  int8_t focusedTouchKey; // index into gKeyDesc
};

// payload is the same between both LH/RH modules.
struct MainPayload
{
  MainChecksummablePayload data;
  uint16_t framerate; // not really needed but it's simpler to keep it just like LHRHPayload
  uint16_t serial;
  uint16_t dataChecksum;
};

inline String ToString(const MainPayload& p)
{
  char format[800];
  sprintf(format, "s#:[%d] chk:[%04x]",
    (int)p.serial,
    (int)p.dataChecksum
    );
  return format;
}

uint16_t CalcChecksum(const MainPayload& p)
{
  static FastCRC16 CRC16;
  return CRC16.x25((const uint8_t*)&p.data, sizeof(p.data));
}

////////////////////////////////////////////////////////////////////////////////

// needed because EasyTransfer wants the same datatype for rx / tx
union BothPayloads
{
  LHRHPayload lhrhPayload;
  MainPayload mainPayload;
};

// 4 bytes is what EasyTransfer adds to the payload.
// In theory this maximizes success of transfers. In practice it's just not necessary
//static_assert((sizeof(BothPayloads) + 4) < (RX_BUFFER_SIZE / 2), "");


////////////////////////////////////////////////////////////////////////////////

// the LHRH serial handler
class CCLHRHTxRx: UpdateObjectT<ProfileObjectType::TxRx>
{
  // { for TX
  framerateCalculator mFramerate;
  HardwareSerial& mSerial;
  EasyTransfer mET;
  BothPayloads mLivePayload;
  uint16_t mTXSerial = 0;
  // for TX }
  
public:

  // { for rx
  bool mHaveNewData = false;
  bool mErrorsDirty = false;
  MainPayload mReceivedData;
  
  // stats
  uint32_t mRxSuccess = 0;
  uint32_t mSkippedPayloads = 0;
  uint32_t mChecksumErrors = 0;
  // for rx }

  explicit CCLHRHTxRx(HardwareSerial& serial) :
    mSerial(serial)
  {
  }

  virtual void setup() {
    mSerial.begin(CCEWI_BAUD);
    mET.begin(details(mLivePayload), &mSerial);
  }

  virtual void loop() {
    mFramerate.onFrame();

    mErrorsDirty = false;
    mHaveNewData = false;
    if (mET.receiveData())
    {
      auto checksum = CalcChecksum(mLivePayload.mainPayload);
      if (checksum != mLivePayload.mainPayload.dataChecksum) {
        mChecksumErrors ++;
        mErrorsDirty = true;
        return;
      }
      
      mHaveNewData = true;
      if (mRxSuccess > 0 && (mLivePayload.mainPayload.serial > mReceivedData.serial)) { // account for first successful rx and overflow. in case of overflow just don't count the skips. lazy.
        mSkippedPayloads += mLivePayload.mainPayload.serial - mReceivedData.serial - 1; // account for this one...
      }
      mReceivedData = mLivePayload.mainPayload;
      mRxSuccess ++;
    }
  }

  void Send(LHRHPayload& payload) {
    payload.dataChecksum = CalcChecksum(payload);
    payload.framerate = mFramerate.getFPS();
    payload.serial = mTXSerial;
    mLivePayload.lhrhPayload = payload;
    mET.sendData();
    mTXSerial ++;
  }
};

// the MAIN serial handler
class CCMainTxRx: UpdateObjectT<ProfileObjectType::TxRx>
{
public:
  // { for TX
  framerateCalculator mFramerate;
  HardwareSerial& mSerial;
  EasyTransfer mET;
  BothPayloads mLivePayload;
  uint16_t mTXSerial = 0;
  // for TX }

  // { for rx
  bool mHaveNewData = false;
  bool mErrorsDirty = false;
  LHRHPayload mReceivedData;
  framerateCalculator mRxRate;
  
  // stats
  uint32_t mRxSuccess = 0;
  uint32_t mSkippedPayloads = 0;
  uint32_t mChecksumErrors = 0;
  // for rx }

  explicit CCMainTxRx(HardwareSerial& serial) :
    mSerial(serial)
  {
  }

  virtual void setup() {
    mSerial.begin(CCEWI_BAUD);
    mET.begin(details(mLivePayload), &mSerial);
  }

  virtual void loop() {
    mFramerate.onFrame();

    mErrorsDirty = false;
    mHaveNewData = false;
    if (mET.receiveData())
    {
      auto checksum = CalcChecksum(mLivePayload.lhrhPayload);
      if (checksum != mLivePayload.lhrhPayload.dataChecksum) {
        mChecksumErrors ++;
        mErrorsDirty = true;
        return;
      }
      
      mHaveNewData = true;
      if (mRxSuccess > 0 && (mLivePayload.lhrhPayload.serial > mReceivedData.serial)) { // account for first successful rx and overflow. in case of overflow just don't count the skips. lazy.
        mSkippedPayloads += mLivePayload.lhrhPayload.serial - mReceivedData.serial - 1; // account for this one...
      }
      mReceivedData = mLivePayload.lhrhPayload;
      mRxSuccess ++;
      mRxRate.onFrame();
    }
  }

  void Send(MainPayload& payload) {
    payload.dataChecksum = CalcChecksum(payload);
    payload.framerate = mFramerate.getFPS();
    payload.serial = mTXSerial;
    mLivePayload.mainPayload = payload;
    mET.sendData();
    mTXSerial ++;
  }
};


