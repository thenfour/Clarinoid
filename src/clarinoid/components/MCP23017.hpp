
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

// struct Adafruit_MCP23017 {

//   static constexpr int MCP23017_DEFAULT_ADDRESS = 0x20; //!< default MCP23017 Address

//   // registers
//   static constexpr int MCP23017_IODIRA = 0x00;   //!< I/O direction register A
//   static constexpr int MCP23017_IPOLA = 0x02;    //!< Input polarity port register A
//   static constexpr int MCP23017_GPINTENA = 0x04; //!< Interrupt-on-change pins A
//   static constexpr int MCP23017_DEFVALA = 0x06;  //!< Default value register A
//   static constexpr int MCP23017_INTCONA = 0x08;  //!< Interrupt-on-change control register A
//   static constexpr int MCP23017_IOCONA = 0x0A;   //!< I/O expander configuration register A
//   static constexpr int MCP23017_GPPUA = 0x0C;    //!< GPIO pull-up resistor register A
//   static constexpr int MCP23017_INTFA = 0x0E;    //!< Interrupt flag register A
//   static constexpr int MCP23017_INTCAPA = 0x10;  //!< Interrupt captured value for port register A
//   static constexpr int MCP23017_GPIOA = 0x12;    //!< General purpose I/O port register A
//   static constexpr int MCP23017_OLATA = 0x14;    //!< Output latch register 0 A

//   static constexpr int MCP23017_IODIRB = 0x01;   //!< I/O direction register B
//   static constexpr int MCP23017_IPOLB = 0x03;    //!< Input polarity port register B
//   static constexpr int MCP23017_GPINTENB = 0x05; //!< Interrupt-on-change pins B
//   static constexpr int MCP23017_DEFVALB = 0x07;  //!< Default value register B
//   static constexpr int MCP23017_INTCONB = 0x09;  //!< Interrupt-on-change control register B
//   static constexpr int MCP23017_IOCONB = 0x0B;   //!< I/O expander configuration register B
//   static constexpr int MCP23017_GPPUB = 0x0D;    //!< GPIO pull-up resistor register B
//   static constexpr int MCP23017_INTFB = 0x0F;    //!< Interrupt flag register B
//   static constexpr int MCP23017_INTCAPB = 0x11;  //!< Interrupt captured value for port register B
//   static constexpr int MCP23017_GPIOB = 0x13;    //!< General purpose I/O port register B
//   static constexpr int MCP23017_OLATB = 0x15;    //!< Output latch register 0 B

//   static constexpr int MCP23017_INT_ERR = 255; //!< Interrupt error

//   // minihelper to keep Arduino backward compatibility
//   static inline void wiresend(uint8_t x, TwoWire *theWire) {
//     theWire->write((uint8_t)x);
//   }

//   static inline uint8_t wirerecv(TwoWire *theWire) {
//     return theWire->read();
//   }

// /**
//  * Bit number associated to a give Pin
//  */
// static constexpr uint8_t bitForPin(uint8_t pin) { return pin % 8; }

// /**
//  * Register address, port dependent, for a given PIN
//  */
// static constexpr uint8_t regForPin(uint8_t pin, uint8_t portAaddr,
//                                      uint8_t portBaddr) {
//   return (pin < 8) ? portAaddr : portBaddr;
// }

// /**
//  * Reads a given register
//  */
// uint8_t readRegister(uint8_t addr) {
//   // read the current GPINTEN
//   _wire->beginTransmission(MCP23017_DEFAULT_ADDRESS | i2caddr);
//   wiresend(addr, _wire);
//   _wire->endTransmission();
//   _wire->requestFrom(MCP23017_DEFAULT_ADDRESS | i2caddr, 1);
//   return wirerecv(_wire);
// }

// /**
//  * Writes a given register
//  */
// void writeRegister(uint8_t regAddr, uint8_t regValue) {
//   // Write the register
//   _wire->beginTransmission(MCP23017_DEFAULT_ADDRESS | i2caddr);
//   wiresend(regAddr, _wire);
//   wiresend(regValue, _wire);
//   _wire->endTransmission();
// }

// /**
//  * Helper to update a single bit of an A/B register.
//  * - Reads the current register value
//  * - Writes the new register value
//  */
// void updateRegisterBit(uint8_t pin, uint8_t pValue,
//                                           uint8_t portAaddr,
//                                           uint8_t portBaddr) {
//   uint8_t regValue;
//   uint8_t regAddr = regForPin(pin, portAaddr, portBaddr);
//   uint8_t bit = bitForPin(pin);
//   regValue = readRegister(regAddr);

//   // set the value for the particular bit
//   bitWrite(regValue, bit, pValue);

//   writeRegister(regAddr, regValue);
// }

// ////////////////////////////////////////////////////////////////////////////////

// /*!
//  * Initializes the MCP23017 given its HW selected address, see datasheet for
//  * Address selection.
//  * @param addr Selected address
//  * @param theWire the I2C object to use, defaults to &Wire
//  */
// void begin(uint8_t addr, TwoWire *theWire) {
//   if (addr > 7) {
//     addr = 7;
//   }
//   i2caddr = addr;
//   _wire = theWire;

//   _wire->begin();

//   // set defaults!
//   // all inputs on port A and B
//   writeRegister(MCP23017_IODIRA, 0xff);
//   writeRegister(MCP23017_IODIRB, 0xff);
// }

// /**
//  * Initializes the default MCP23017, with 000 for the configurable part of the
//  * address
//  * @param theWire the I2C object to use, defaults to &Wire
//  */
// void begin(TwoWire *theWire) { begin(0, theWire); }

// /**
//  * Sets the pin mode to either INPUT or OUTPUT
//  * @param p Pin to set
//  * @param d Mode to set the pin
//  */
// void pinMode(uint8_t p, uint8_t d) {
//   updateRegisterBit(p, (d == INPUT), MCP23017_IODIRA, MCP23017_IODIRB);
// }

// /**
//  * Reads all 16 pins (port A and B) into a single 16 bits variable.
//  * @return Returns the 16 bit variable representing all 16 pins
//  */
// uint16_t readGPIOAB() {
//   uint16_t ba = 0;
//   uint8_t a;

//   // read the current GPIO output latches
//   _wire->beginTransmission(MCP23017_DEFAULT_ADDRESS | i2caddr);
//   wiresend(MCP23017_GPIOA, _wire);
//   _wire->endTransmission();

//   _wire->requestFrom(MCP23017_DEFAULT_ADDRESS | i2caddr, 2);
//   a = wirerecv(_wire);
//   ba = wirerecv(_wire);
//   ba <<= 8;
//   ba |= a;

//   return ba;
// }

// /*!
//  * @brief Enables the pull-up resistor on the specified pin
//  * @param p Pin to set
//  * @param d Value to set the pin
//  */
// void pullUp(uint8_t p, uint8_t d) {
//   updateRegisterBit(p, d, MCP23017_GPPUA, MCP23017_GPPUB);
// }

// private:
//   uint8_t i2caddr;
//   TwoWire *_wire; //!< pointer to a TwoWire object

// };

// attach to a CCMCP23017 value to represent a single switch
struct BitButton : ISwitch
{
    uint16_t &mMcpCurrentValue;
    int mKeyMask;

    explicit BitButton(uint16_t &mcpCurrentValue, int keyIndex /* 0-15 */)
        : mMcpCurrentValue(mcpCurrentValue), mKeyMask(1 << keyIndex)
    {
        CCASSERT(keyIndex >= 0 && keyIndex < 16);
    }

    virtual bool CurrentValue() const override
    {
        return !(mMcpCurrentValue & mKeyMask);
    }
};

struct CCMCP23017
{
    Adafruit_MCP23017 mMcp;
    uint16_t mCurrentValue = 0;
    uint16_t mPreviousValue = 0;

    BitButton mButtons[16] = {
        BitButton{mCurrentValue, 0},
        BitButton{mCurrentValue, 1},
        BitButton{mCurrentValue, 2},
        BitButton{mCurrentValue, 3},
        BitButton{mCurrentValue, 4},
        BitButton{mCurrentValue, 5},
        BitButton{mCurrentValue, 6},
        BitButton{mCurrentValue, 7},
        BitButton{mCurrentValue, 8},
        BitButton{mCurrentValue, 9},
        BitButton{mCurrentValue, 10},
        BitButton{mCurrentValue, 11},
        BitButton{mCurrentValue, 12},
        BitButton{mCurrentValue, 13},
        BitButton{mCurrentValue, 14},
        BitButton{mCurrentValue, 15},
    };

    explicit CCMCP23017(TwoWire *theWire, int address = 0x20)
    {
        NoInterrupts _ni;
        mMcp.begin(theWire);
        // theWire->setClock(400000); // use high speed mode. default speed = 100k
        for (int i = 0; i < 16; ++i)
        {
            mMcp.pinMode(i, INPUT);
            mMcp.pullUp(i, HIGH); // turn on a 100K pullup internally
        }
    }

    void Update()
    {
        NoInterrupts _ni;
        auto newCurrentVal = mMcp.readGPIOAB();
        mPreviousValue = mCurrentValue;
        mCurrentValue = newCurrentVal;
    }
};

} // namespace clarinoid
