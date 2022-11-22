# Oscillator development

## Ring Mod

Ring mod is a fairly "weak" type of modulation, and I have never really found great uses for it. But it's simple to implement.

## Hard Sync

## Band-limiting


## FM

FM is simple to implement, but there are quirks going on to make it behave in a musical way.

### Chained modulation

Phase restart is essential when using FM, otherwise the character would be inconsistent. There would be issues when portamento varies between oscillators too, so it may be that these need to be synchronized between oscillators in the future.

### Feedback

FM Feedback is known for being unstable, but at least it's usable. I found with the naive implementation, it starts to break up in glitchy ways at higher values. Digital-sounding glitches and octave shifting beeps, while the identical algorithm in JSFX or C++ produce expected "stable" waveforms.

See https://github.com/thenfour/Clarinoid/issues/193

Reducing all the way down to a few float ops per sample, and dumping the numeric results of identical code, it's confirmed that it's the result of FM feedback, amplifying and accumulating floating point discrepencies.

* See: `exprcmd.cpp`
* See: `\notes\osc accuracy analysis x86 comparison.xlsx`

For example, modulation between FM oscillators is not a problem. The error doesn't accumulate.

Note: `double` does not seem to affect anything; same glitching occurs.

I could dive deeper but I doubt it would get me closer to a solution. It may be in the implementation of `arm_sin_f32` (although I checked a taylor series version and had the same issues). But I suspect it's in very small differences in multiplication. Multiplying phase by `2*pi` seemed to be a source of error.

I have not found a solution for this, but some mitigations to mind.

* "recording" certain values to capture a single wave cycle. This would eliminate the accumulating errors, and ensure wave cycles sound stable / consistent. Unfortunately there are issues with regards to wave cycles not being exact sample lengths (which would cause detuning at high frequencies). Maybe mitigations can be done, like just calculating 1 fixed long waveform, and interpolating out of it, like a sample player. I'm not sure that would work either though because the feedback sound will depend on the ratio of frequency to samplerate.
* Resetting feedback state every wave cycle. I tried this, setting to either 1 (the value of `sin()` at the start of a cycle) or 0 (the initial state i used in JSFX to get good results); both seem to possibly help, but introduce other glitches and i'm not sure it's actually helping. Prefer simpler solution. Also we're going to suffer the same as above: cycle crossing won't be at even bounds.
* Use fixed-point math. Oscillators don't really benefit from floating-point style behaviors so maybe using fixed 32-bit math would help stabilize things.