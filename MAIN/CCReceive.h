
#ifndef CCRECEIVE_H
#define CCRECEIVE_H

#include <EasyTransfer.h>
#include "CCTransmit.h"

class CCReceiver : IUpdateObject
{
  EasyTransfer mET;
  HardwareSerial& mSerial;

public:

  // we don't buffer data. we use the latest stuff, skipping old. within 1 frame, it's just not helpful to process
  // intermediate data. it is meant to be stateless, because
  // most of all: we expect to miss some data occasionally. so we are required for this to be stateless.
  bool mHaveNewData = false;
  bool mErrorsDirty = false;
  LHRHPayload mData;
  LHRHPayload mInboundData;
  
  // stats
  uint32_t mRxSuccess = 0;
  uint32_t mSkippedPayloads = 0;
  uint32_t mChecksumErrors = 0;
 
  explicit CCReceiver(HardwareSerial& serial) :
    mSerial(serial)
  {
  }

  virtual void setup() {
    mSerial.begin(CCEWI_BAUD);
    mET.begin(details(mInboundData), &mSerial);
  }
  
  virtual void loop() {
    mErrorsDirty = false;
    if (mET.receiveData())
    {
      LHRHPayload model; // initializes with marker.
      auto checksum = CalcChecksum(mInboundData);
      
      if (checksum != mInboundData.dataChecksum) {
        mChecksumErrors ++;
        mErrorsDirty = true;
        return;
      }
      
      mHaveNewData = true;
      if (mRxSuccess > 0 && (mInboundData.serial > mData.serial)) { // account for first successful rx and overflow. in case of overflow just don't count the skips. lazy.
        mSkippedPayloads += mInboundData.serial - mData.serial - 1; // account for this one...
      }
      mData = mInboundData;
      mRxSuccess ++;
    }
  }

};

#endif
