for teensy footprints & symbols, go to 
https://github.com/XenGi/teensy_library
https://github.com/XenGi/teensy.pretty

why are there Symbols & Symbols2?
because KiCad changed formats so Symbols is old stuff
and Symbols2 is new.


CapSliderDesigner.
this is a freecad SKETCH which allows generating dimensions of a cap touch slider chevron pattern.
this is only 1 line segment of the chevron; a real chevron just mirrors it however many times it wants.
the spreadsheet defines these parameters:
- PITCH: kinda self explanatory? distance along the slider axis.
- SEPARATION: how much distance between copper fills (true dist)

then you can drag to set the width. the reference dimensions are shown.
i aim to make a simple-to-model-in-kicad variation so drag width to wherever makes close-to-integral dimensions.

3P = guaranteed 3 pads at each Y location
P5 = pitch 5.0mm
S0.5 = separation = linear distance between copper areas
W = width
A = gap X mm (gap which, at angle, produces S separation between pads)
B = pad X mm
C = chevron offset X (slope)
x10 = 10 pads

