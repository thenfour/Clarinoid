
#ifndef CCRECEIVE_H
#define CCRECEIVE_H

#include <FastCRC.h>
#include "CCTransmit.h"

class CCReceiver : IUpdateObject
{
  FastCRC16 CRC16;
  HardwareSerial& mSerial;

public:

  // we don't buffer data. we use the latest stuff, skipping old. within 1 frame, it's just not helpful to process
  // intermediate data. it is meant to be stateless, because
  // most of all: we expect to miss some data occasionally. so we are required for this to be stateless.
  bool mHaveNewData;
  LHRHPayload mLatestData;
  
  // stats
  uint32_t mRxSuccess = 0;
  uint32_t mSkippedPayloads = 0;
  uint32_t mSizeErrors = 0;
  uint32_t mChecksumErrors = 0;
 
  explicit CCReceiver(HardwareSerial& serial) :
    mSerial(serial)
  {
  }

  virtual void setup() {
    mSerial.begin(CCEWI_BAUD);
  }

  virtual void loop() {
    char fmt[100];
    mHaveNewData = false;
    
    // some things make this more complex than it seems like it should be:
    // you can only peek() one byte at a time. so we can't rummage through the whole
    // buffer looking for stuff without actually taking the whole buffer.
    // we cannot take the whole buffer, because we will chop off partial
    // transmissions.
    // we cannot chop off partial transmissions because we expect the buffer
    // to overflow between frames sometimes. we need to tolerate when packets get
    // sliced up and FIFO overflows. I mean, the FIFO should handle this well automatically.
    //
    // so, for this to work, our payload size MUST be 1/2 the buffer size in order to
    // guarantee that it can always hold a complete payload (once it's filled). From there,
    // it's our job to not remove too much data from the rx buffer.
    //
    // in the case that our payload is very small, we also want to make sure we just
    // skip to the last complete payload transmission.
    
    LHRHPayload model; // initializes with marker.
    LHRHPayload rxPayload;
    int markerSize = sizeof(model.beginMarker);
    uint8_t* pmodel = (uint8_t*)&rxPayload;
    uint8_t* prx = (uint8_t*)&rxPayload;
    int irx = 0; // byte pos in rxPayload

    if (!mSerial.available())
      return;
      
    Serial.println(String("{ rx, available = ") + mSerial.available() + "; payloadSize = " + sizeof(LHRHPayload));

    // So first, skip to the last theoretical complete payload transmission.
    // worst case: [complete payload][payload minus 1 byte]|
    while((uint32_t)mSerial.available() > (sizeof(LHRHPayload) * 2))
    {
      mSerial.read(); // throw away old stateless data.
    }

    Serial.println(String("  after throwing away data, available = ") + mSerial.available());

    // now carefully search the buffer for the beginning marker.
    // if it's not found in the complete payload area of the buffer, just abandon ship; this is some kind of error.
    // if it's found, this is the happy flow
    while(true)
    {
      if ((uint32_t)mSerial.available() < sizeof(LHRHPayload)) {
        // don't touch this data! it's still incoming...
        Serial.println(String("} nothing found, available = ") + mSerial.available());
        return;
      }
      uint8_t c = mSerial.read();
      if (c == pmodel[irx]) {
        Serial.println(String("  found marker byte ") + irx);
        // this byte matches the marker byte we're currently searching for.
        irx ++; // now we can search for the next byte.
        if (irx == markerSize) {
          // marker matched. now we're clear to proceed reading the rest of the payload.
          for (/* irx is already pointing to the next byte */; irx < (int)sizeof(LHRHPayload); ++ irx) {
            prx[irx] = mSerial.read();
          }

          Serial.println(String("rx: ") + ToString(rxPayload));
          
          // we have successfully read. verify data
          auto checksum = CalcChecksum(rxPayload);
          sprintf(fmt, "%08x", checksum);
          Serial.println(String("---> checksum i calculated: ") + fmt);
          bool success = true;
          if (checksum != rxPayload.dataChecksum) {
            mChecksumErrors ++;
            success = false;
          }
          if (rxPayload.payloadSize != model.payloadSize) {
            mSizeErrors ++;
            success = false;
          }
          if (!success) {
            Serial.println(String("} returning error"));
            return;
          }
          
          mHaveNewData = true;
          mLatestData = rxPayload;
          mRxSuccess ++;
          Serial.println(String("} returning success"));
          return;
        }
      } else {
        sprintf(fmt, "[%d] = %02x", irx, (int)c);
        Serial.println(String("  not a marker byte: ") + fmt);
        irx = 0; // reset our search; byte didn't match
      }
    }
  }

};

#endif
