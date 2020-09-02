// nevermind messy code; still in prototyping mindset.

#ifndef CCTXLHRHTOMAIN_H
#define CCTXLHRHTOMAIN_H

#include <EasyTransfer.h>
#include <FastCRC.h>

#include "Shared_CCUtil.h"

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
struct CapTouchKeyData
{
  bool IsPressed : 8;
};

////////////////////////////////////////////////////////////////////////////////
struct LHRHChecksummablePayload
{
  CapTouchKeyData keys[6];
  CapTouchKeyData octaveKeys[4];

  int8_t focusedKey; // index into gKeyDesc
  uint32_t focusedTouchReadMicros;
  uint32_t focusedTouchReadValue;
  uint32_t focusedTouchReadUntouchedMicros;
  uint32_t focusedTouchReadThresholdMicros;
  
  bool button1;
  bool button2;

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

inline String ToString(const LHRHPayload& p)
{
  char format[800];
  sprintf(format, "s#:[%d] chk:[%04x] d:[k%c%c%c%c%c%c o%c%c%c%c b%c%c p:%d p:%d]",
    (int)p.serial,
    (int)p.dataChecksum,
//    p.data.keys[0] ? '0' : '-',
//    p.data.keys[1] ? '1' : '-',
//    p.data.keys[2] ? '2' : '-',
//    p.data.keys[3] ? '3' : '-',
//    p.data.keys[4] ? '4' : '-',
//    p.data.keys[5] ? '5' : '-',
    p.data.keys[0].IsPressed ? '0' : '-',
    p.data.keys[1].IsPressed ? '1' : '-',
    p.data.keys[2].IsPressed ? '2' : '-',
    p.data.keys[3].IsPressed ? '3' : '-',
    p.data.keys[4].IsPressed ? '4' : '-',
    p.data.keys[5].IsPressed ? '5' : '-',

    p.data.octaveKeys[0].IsPressed ? '0' : '-',
    p.data.octaveKeys[1].IsPressed ? '1' : '-',
    p.data.octaveKeys[2].IsPressed ? '2' : '-',
    p.data.octaveKeys[3].IsPressed ? '3' : '-',

    p.data.button1 ? '1' : '-',
    p.data.button2 ? '2' : '-',

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



#endif
