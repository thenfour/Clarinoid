# touchablePin
A Teensy extension allowing fast detection of touched pins

Created by David Elvig on 1/26/17.
- Based on Paul Stofregren's touchRead.c (see .cpp file for reference

Capacitive Touch pins are charged to get their full capacitance, and the time-to-charge is proportional the capacitance.
touchablePin works by assigning an untouched timing value with a call to initUntouched()
isTouched() then returns true as soon as the time to charge exceeds (untouchedTime * _maxfactor)
... not waiting for the pin to get fully charged.

The default value for MAX_FACTOR is 1.3, and can be adjusted in the #define below based on trial and error against your capacitive touch pin connected hardware.

touchablePin(void);  // If you use this, be sure to call setPin() before calling isTouched()
touchablePin(uint8_t); // sets a pin number on instantiation
touchablePin(uint8_t, float);  // sets a pin number and an alternative MAX_FACTOR
//Appropriately small _maxFactors lead to faster isTouched() return times.
//Too small _maxFactor will lead to false positives for isTouched().
touchablePin(uint8_t, float, int); // same as the above, and also changes the _numSamples
// attribute from the default of 4
// smaller is faster, larger senses more touches, perhaps more reliably.

To be useful, the pin must be untouched on start-up (or rather, when initUntouched() is called).

Use the touchablePin.touchRead() method to experiment with your setup.
It will return the same value as the unmodified touchPin() to check the ratio of your untouched and touched states.

TODO: The first call to the private method init() returns a too-small capacitance value (and it happens too fast)
The work-around is to call init() twice.  The To Do item is to figure out why (is there some pre-existing
charge on the pin?), and eliminate the extra call or document the reasoning.
