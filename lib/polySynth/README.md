# Teensy polyBLEP Oscillator

A quasi-bandlimited oscillator for the Teensy Audio Library 

Usage: 
This Software only works together with the Teensy Audio Library.
In your Arduino Sketch, #include "polyBlepOscillator.h" and use as an Audio Object.

Inputs:
- Input 0: Frequency Modulation for Oscillator 1
- Input 1: Pulse Width Modulation for Oscillator 1
- Input 2: Frequency Modulation for Oscillator 2
- Input 3: Pulse Width Modulation for Oscillator 2
- Input 4: Frequency Modulation for Oscillator 3
- Input 5: Pulse Width Modulation for Oscillator 3

Outputs:
- Output 0: Output Oscillator 1
- Output 1: Output Oscillator 2
- Output 2: Output Oscillator 3

Functions: 
When setting the parameters, the first argument is the oscillator
(1, 2 or 3), the second the parameter. 
Further details can be seen in the header file.
