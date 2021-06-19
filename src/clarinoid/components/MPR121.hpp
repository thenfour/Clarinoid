// Adafruit's implementation does not give enough flexibility to, for example,
// disable some things, or dynamically enable autoconfigure or setting per key thresholds.
// so this is a "fork"

#pragma once

namespace MPR121
{

// so because we depend on this, actually do include the Adafruit_MPR121.h library.
//#include <Adafruit_BusIO_Register.h>
//#include <Adafruit_I2CDevice.h>

// The default I2C address
// #define MPR121_I2CADDR_DEFAULT 0x5A        ///< default I2C address
// #define MPR121_TOUCH_THRESHOLD_DEFAULT 12  ///< default touch threshold value
// #define MPR121_RELEASE_THRESHOLD_DEFAULT 6 ///< default relese threshold value

/*!
 *  Device register map
 */
enum MPR121Register {
  TOUCHSTATUS_L = 0x00,
  TOUCHSTATUS_H = 0x01,
  FILTDATA_0L = 0x04,
  FILTDATA_0H = 0x05,
  BASELINE_0 = 0x1E,
  MHDR = 0x2B,
  NHDR = 0x2C,
  NCLR = 0x2D,
  FDLR = 0x2E,
  MHDF = 0x2F,
  NHDF = 0x30,
  NCLF = 0x31,
  FDLF = 0x32,
  NHDT = 0x33,
  NCLT = 0x34,
  FDLT = 0x35,

  TOUCHTH_0 = 0x41,
  RELEASETH_0 = 0x42,
  DEBOUNCE = 0x5B,
  CONFIG1 = 0x5C,
  CONFIG2 = 0x5D,
  CHARGECURR_0 = 0x5F,
  CHARGETIME_1 = 0x6C,
  ECR = 0x5E,
  AUTOCONFIG0 = 0x7B,
  AUTOCONFIG1 = 0x7C,
  UPLIMIT = 0x7D,
  LOWLIMIT = 0x7E,
  TARGETLIMIT = 0x7F,

  GPIODIR = 0x76,
  GPIOEN = 0x77,
  GPIOSET = 0x78,
  GPIOCLR = 0x79,
  GPIOTOGGLE = 0x7A,

  SOFTRESET = 0x80,
};

//.. thru to 0x1C/0x1D

/*!
 *  @brief  Class that stores state and functions for interacting with MPR121
 *  proximity capacitive touch sensor controller.
 */
struct MPR121Device {
// public:
//   // Hardware I2C
//   Adafruit_MPR121();

//   bool begin(uint8_t i2caddr = MPR121_I2CADDR_DEFAULT, TwoWire *theWire = &Wire,
//              uint8_t touchThreshold = MPR121_TOUCH_THRESHOLD_DEFAULT,
//              uint8_t releaseThreshold = MPR121_RELEASE_THRESHOLD_DEFAULT);

//   uint16_t filteredData(uint8_t t);
//   uint16_t baselineData(uint8_t t);

//   uint8_t readRegister8(uint8_t reg);
//   uint16_t readRegister16(uint8_t reg);
//   void writeRegister(uint8_t reg, uint8_t value);
//   uint16_t touched(void);
//   // Add deprecated attribute so that the compiler shows a warning
//   void setThreshholds(uint8_t touch, uint8_t release)
//       __attribute__((deprecated));
//   void setThresholds(uint8_t touch, uint8_t release);

// private:
  Adafruit_I2CDevice *i2c_dev = NULL;
//};

//#endif
/*!
 * @file Adafruit_MPR121.cpp
 *
 *  @mainpage Adafruit MPR121 arduino driver
 *
 *  @section intro_sec Introduction
 *
 *  This is a library for the MPR121 I2C 12-chan Capacitive Sensor
 *
 *  Designed specifically to work with the MPR121 sensor from Adafruit
 *  ----> https://www.adafruit.com/products/1982
 *
 *  These sensors use I2C to communicate, 2+ pins are required to
 *  interface
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing
 *  products from Adafruit!
 *
 *  @section author Author
 *
 *  Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 *  @section license License
 *
 *  BSD license, all text here must be included in any redistribution.
 */

//#include "Adafruit_MPR121.h"

// uncomment to use autoconfig !
//#define AUTOCONFIG // use autoconfig (Yes it works pretty well!)

/*!
 *  @brief    Begin an MPR121 object on a given I2C bus. This function resets
 *            the device and writes the default settings.
 *  @param    i2caddr
 *            the i2c address the device can be found on. Defaults to 0x5A.
 *  @param    *theWire
 *            Wire object
 *  @param    touchThreshold
 *            touch detection threshold value
 *  @param    releaseThreshold
 *            release detection threshold value
 *  @returns  true on success, false otherwise
 */
bool begin(uint8_t i2caddr, TwoWire *theWire, uint8_t touchThreshold = 12, uint8_t releaseThreshold = 6, uint8_t electrodesInUse = 12, bool autoconfig = false) {

  if (i2c_dev) {
    delete i2c_dev;
  }
  i2c_dev = new Adafruit_I2CDevice(i2caddr, theWire);

  if (!i2c_dev->begin()) {
    return false;
  }

  // soft reset
  writeRegister(MPR121Register::SOFTRESET, 0x63);
  delay(1);
//   for (uint8_t i = 0; i < 0x7F; i++) {
//     //  Serial.print("$"); Serial.print(i, HEX);
//     //  Serial.print(": 0x"); Serial.println(readRegister8(i));
//   }

  writeRegister(MPR121Register::ECR, 0x0);

  uint8_t c = readRegister8(MPR121_CONFIG2);

  if (c != 0x24)
    return false;

  setThresholds(touchThreshold, releaseThreshold);
  writeRegister(MPR121Register::MHDR, 0x01);
  writeRegister(MPR121Register::NHDR, 0x01);
  writeRegister(MPR121Register::NCLR, 0x0E);
  writeRegister(MPR121Register::FDLR, 0x00);

  writeRegister(MPR121Register::MHDF, 0x01);
  writeRegister(MPR121Register::NHDF, 0x05);
  writeRegister(MPR121Register::NCLF, 0x01);
  writeRegister(MPR121Register::FDLF, 0x00);

  writeRegister(MPR121Register::NHDT, 0x00);
  writeRegister(MPR121Register::NCLT, 0x00);
  writeRegister(MPR121Register::FDLT, 0x00);

  writeRegister(MPR121Register::DEBOUNCE, 0);
  writeRegister(MPR121Register::CONFIG1, 0x10); // default, 16uA charge current
  writeRegister(MPR121Register::CONFIG2, 0x20); // 0.5uS encoding, 1ms period

    if (autoconfig) {
        writeRegister(MPR121Register::AUTOCONFIG0, 0x0B);

        // correct values for Vdd = 3.3V
        writeRegister(MPR121Register::UPLIMIT, 200);     // ((Vdd - 0.7)/Vdd) * 256
        writeRegister(MPR121Register::TARGETLIMIT, 180); // UPLIMIT * 0.9
        writeRegister(MPR121Register::LOWLIMIT, 130);    // UPLIMIT * 0.65
    }

  writeRegister(MPR121Register::ECR, B10000000 + electrodesInUse);// 5 bits for baseline tracking, proximity disabled + X electrodes running
  return true;
}

/*!
 *  @brief      DEPRECATED. Use Adafruit_MPR121::setThresholds(uint8_t touch,
 *              uint8_t release) instead.
 *  @param      touch
 *              see Adafruit_MPR121::setThresholds(uint8_t touch, uint8_t
 * *release)
 *  @param      release
 *              see Adafruit_MPR121::setThresholds(uint8_t touch, *uint8_t
 * release)
 */
void setThreshholds(uint8_t touch, uint8_t release) {
  setThresholds(touch, release);
}

/*!
 *  @brief      Set the touch and release thresholds for all 13 channels on the
 *              device to the passed values. The threshold is defined as a
 *              deviation value from the baseline value, so it remains constant
 * even baseline value changes. Typically the touch threshold is a little bigger
 * than the release threshold to touch debounce and hysteresis. For typical
 * touch application, the value can be in range 0x05~0x30 for example. The
 * setting of the threshold is depended on the actual application. For the
 * operation details and how to set the threshold refer to application note
 * AN3892 and MPR121 design guidelines.
 *  @param      touch
 *              the touch threshold value from 0 to 255.
 *  @param      release
 *              the release threshold from 0 to 255.
 */
void setThresholds(uint8_t touch, uint8_t release) {
  // set all thresholds (the same)
  for (uint8_t i = 0; i < 12; i++) {
    writeRegister(MPR121Register::TOUCHTH_0 + 2 * i, touch);
    writeRegister(MPR121Register::RELEASETH_0 + 2 * i, release);
  }
}

void setThresholds(uint8_t keyIndex, uint8_t touch, uint8_t release) {
  writeRegister(MPR121Register::TOUCHTH_0 + 2 * keyIndex, touch);
  writeRegister(MPR121Register::RELEASETH_0 + 2 * keyIndex, release);
}

/*!
 *  @brief      Read the filtered data from channel t. The ADC raw data outputs
 *              run through 3 levels of digital filtering to filter out the high
 * frequency and low frequency noise encountered. For detailed information on
 * this filtering see page 6 of the device datasheet.
 *  @param      t
 *              the channel to read
 *  @returns    the filtered reading as a 10 bit unsigned value
 */
uint16_t filteredData(uint8_t t) {
  if (t > 12)
    return 0;
  return readRegister16(MPR121Register::FILTDATA_0L + t * 2);
}

/*!
 *  @brief      Read the baseline value for the channel. The 3rd level filtered
 *              result is internally 10bit but only high 8 bits are readable
 * from registers 0x1E~0x2A as the baseline value output for each channel.
 *  @param      t
 *              the channel to read.
 *  @returns    the baseline data that was read
 */
uint16_t baselineData(uint8_t t) {
  if (t > 12)
    return 0;
  uint16_t bl = readRegister8(MPR121Register::BASELINE_0 + t);
  return (bl << 2);
}

/**
 *  @brief      Read the touch status of all 13 channels as bit values in a 12
 * bit integer.
 *  @returns    a 12 bit integer with each bit corresponding to the touch status
 *              of a sensor. For example, if bit 0 is set then channel 0 of the
 * device is currently deemed to be touched.
 */
uint16_t touched(void) {
  uint16_t t = readRegister16(MPR121Register::TOUCHSTATUS_L);
  return t & 0x0FFF;
}

/*!
 *  @brief      Read the contents of an 8 bit device register.
 *  @param      reg the register address to read from
 *  @returns    the 8 bit value that was read.
 */
uint8_t readRegister8(uint8_t reg) {
  Adafruit_BusIO_Register thereg = Adafruit_BusIO_Register(i2c_dev, reg, 1);

  return (thereg.read());
}

/*!
 *  @brief      Read the contents of a 16 bit device register.
 *  @param      reg the register address to read from
 *  @returns    the 16 bit value that was read.
 */
uint16_t readRegister16(uint8_t reg) {
  Adafruit_BusIO_Register thereg =
      Adafruit_BusIO_Register(i2c_dev, reg, 2, LSBFIRST);

  return (thereg.read());
}

/*!
    @brief  Writes 8-bits to the specified destination register
    @param  reg the register address to write to
    @param  value the value to write
*/
void writeRegister(uint8_t reg, uint8_t value) {
  // MPR121 must be put in Stop Mode to write to most registers
  bool stop_required = true;

  // first get the current set value of the MPR121_ECR register
  Adafruit_BusIO_Register ecr_reg =
      Adafruit_BusIO_Register(i2c_dev, MPR121Register::ECR, 1);

  uint8_t ecr_backup = ecr_reg.read();
  if ((reg == MPR121Register::ECR) || ((0x73 <= reg) && (reg <= 0x7A))) {
    stop_required = false;
  }

  if (stop_required) {
    // clear this register to set stop mode
    ecr_reg.write(0x00);
  }

  Adafruit_BusIO_Register the_reg = Adafruit_BusIO_Register(i2c_dev, reg, 1);
  the_reg.write(value);

  if (stop_required) {
    // write back the previous set ECR settings
    ecr_reg.write(ecr_backup);
  }
}

};

} // namespace MPR121
