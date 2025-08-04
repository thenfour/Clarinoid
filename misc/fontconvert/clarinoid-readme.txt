so this is from adafruit's gfx library, and they include an annoyingly convoluted
method of building this on windows.

but freetype comes with visual studio projects, so i found it much simpler to
just create a new VS project with this .c file in it and statically link with 
freetype in VS.