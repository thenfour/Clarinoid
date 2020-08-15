
#ifndef CCTRANSMIT_H
#define CCTRANSMIT_H

#include <EasyTransfer.h>
#include <FastCRC.h>

#include "CCUtil.h"

#define CCEWI_BAUD 500000 // https://www.pjrc.com/teensy/td_uart.html // 

// important to keep this well under HardwareSerial.cpp:RX_BUFFER_SIZE/2.
// We want there ALWAYS to be a complete packet in the buffer, not split up.
// and consider that the buffer could have like
// 2. [whole packet]
// 3. [99% of a packet]
// so it must be able to hold 2 packets. space is therefore a premium.
#define RX_BUFFER_SIZE 64
struct ChecksummablePayload
{
  bool key1 : 1;
  bool key2 : 1;
  bool key3 : 1;
  bool key4 : 1;
  bool key5 : 1;
  bool key6 : 1;

  bool octave1 : 1;
  bool octave2 : 1;
  bool octave3 : 1;
  bool octave4 : 1;
  bool button1 : 1;
  bool button2 : 1;

  uint16_t pressure1; // raw analog readings
  uint16_t pressure2; // raw analog readings
};


// payload is the same between both LH/RH modules.
struct LHRHPayload
{
  ChecksummablePayload data = {0};
  
  uint16_t framerate = 0;
  uint16_t serial = 0;
  uint16_t dataChecksum = 0;
};

// 4 bytes is what EasyTransfer adds to the payload.
static_assert((sizeof(LHRHPayload) + 4) < (RX_BUFFER_SIZE / 2), "");

inline String ToString(const LHRHPayload& p)
{
  char format[800];
  sprintf(format, "s#:[%d] chk:[%04x] d:[k%c%c%c%c%c%c o%c%c%c%c b%c%c p:%d p:%d]",
    (int)p.serial,
    (int)p.dataChecksum,
    p.data.key1 ? '1' : '-',
    p.data.key2 ? '2' : '-',
    p.data.key3 ? '3' : '-',
    p.data.key4 ? '4' : '-',
    p.data.key5 ? '5' : '-',
    p.data.key6 ? '6' : '-',

    p.data.octave1 ? '1' : '-',
    p.data.octave2 ? '2' : '-',
    p.data.octave3 ? '3' : '-',
    p.data.octave4 ? '4' : '-',

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


uint16_t gTXSerial = 0;

class CCTransmit : IUpdateObject
{
  framerateCalculator mFramerate;
  HardwareSerial& mSerial;
  EasyTransfer mET;
  LHRHPayload mTxPayload;
  
public:
  explicit CCTransmit(HardwareSerial& serial) :
    mSerial(serial)
  {
  }

  virtual void setup() {
    mSerial.begin(CCEWI_BAUD);
    mET.begin(details(mTxPayload), &mSerial);
  }

  virtual void loop() {
    mFramerate.onFrame();
  }

  void Send(LHRHPayload& payload) {
    payload.dataChecksum = CalcChecksum(payload);
    payload.framerate = mFramerate.getFPS();
    payload.serial = gTXSerial;
    //mSerial.write((uint8_t*)&payload, sizeof(payload));
    mTxPayload = payload;
    mET.sendData();
    gTXSerial ++;
  }
};

#endif
