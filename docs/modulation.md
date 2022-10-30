# Modulations on Clarinoid

Trying to capture justifications for things here.

Modelled after the modulations I've used in the past. Some systems are not really translatable, like Renoise's meta devices or highly UI-dependent or grid-like types like FM8. But we have enough CPU to probably support whatever we need as long as it's focused and optimized.

The modulation systems that seem to make most sense to inspire from are Massive and Serum. And anyway modulations are not some kind of magic; the system sorta implies the design already. Recent work on 7jam also is largely the basis for this.

Note that modulations are critical for Clarinoid, for a few reasons:

* Mostly monophonic, therefore expression is more important. Modulations are where the richness and expression is.
* Everything revolves around breath, which is a modulation. It better be flexible in order to make diverse sounds.
* Because the filter is sorta blanketing everything, another reason we need other means to spice up the sound and keep it from all sounding the same.

## Algorithm

The fundamental algorithm to start with is

    output = base + modulation

Where `base` is a base value or signal (an oscillator frequency, or amplitude, or LFO rate). `Modulation` is a value or signal to add into the signal.

In practice, all signals are bipolar `[-1,1)`. Even modulation sources like Breath, which doesn't have any concept of negative values, is a signed signal with an unused negative pole.

### Ranges
Of course every parameter can have a different range. Breath is from `[0,1]` while filter cutoff frequency is `0-22000 hz`. These are all compressed into the `[-1,1)` signal range.

The function above is not that useful because there's no correspondance of scaling. Modulating frequency by LFO would heavily clip values and always scale the entire frequency range.

So accounting for scaling modulations to a certain range,

    output = base + modulation * scale

Now the user adjusts a scaling knob and the modulation is pretty useful just like this.

### Polarity

There are limitations with a simple `scale` parameter, and a simple `polarity` parameter can go a very long way for flexibility and workflow.

Scenario #1: You want `breath` to modulate `filter freq`, but you want the "resting" position to be at the extent rather than the base value. In other words, with simple scale as above, you get a transfer that looks like:

             0==> +         breath
    <--------|-------->     incoming signal
             ^base

But here's what you want:

           + <==0           breath
    <--------|-------->     incoming signal
             ^base

That's common. In Massive when I want to make this type of mapping, it requires constantly tweaking two parameters at the same time: Base value, and modulation scale. This scenario would do it ith 1 knob.

Scenario #2: You want `LFO` to modulate only in the positive pole. LFO is natively bipolar, but you can specify that you want it to be adjusted to be positive.

        - <==0==> +         LFO
    <--------|-------->     incoming signal
             ^base

But we'd like to be able to map it into positive pole:

           - <==0==> +    LFO
    <--------|-------->     incoming signal
             ^base

While we're at it, we can also invert the signal.

           + <==0==> -    LFO
    <--------|-------->     incoming signal
             ^base

Or inverting bipolar,

        + <==0==> -    LFO
    <--------|-------->     incoming signal
             ^base

Fortunately all these scenarios are possible with a single parameter which specifies a polarity mapping. And it's very efficient, every possible mapping reduced to a single multiple & add.

    ConvertPolarity(..) {
        if (naturalPolarity == POS && mSourcePolarity == N11){
          x = x*2-1
        } else (if naturalPolarity == BIPOLARN11 && mSourcePolarity == POS) {
          x = x*.5+.5
        } else if (naturalPolarity == POS && mSourcePolarity == positiveInv){
          x = 1-x
        } else (if naturalPolarity == BIPOLARN11 && mSourcePolarity == positiveInv) {
          x = .5-x*.5
        } else if (naturalPolarity == POS && mSourcePolarity == bipolarInv){
          x = .5-x*2
        } else (if naturalPolarity == BIPOLARN11 && mSourcePolarity == bipolarInv) {
          x = -x
        }
    }

## Curves

Effort was put into creating curves which are useful, accurate, flexible, and optimized.

For performance reasons a LUT is used and linear interpolation is used. X axis is sample value, and Y axis is `k`, representing a curve amount, `[-1,1]`. Since the Y axis is not interpolated, we just use integral indices. It's also more efficient to handle the `0` case where there's no mapping performed.

We cannot use a simple `pow` curve because it's sorta biased over towards one side, and because this is general purpose it should not bias one direction or another.

A circular curve is used then, which can transform from linear all the way to pushed up in the corner.

The original formula is:

    (1-x)^k + y^k = 1

solved for y, that's

    y = (1-x^k)^(1/p)

Then some `1-x` style transforms to mirror it around negative `k` and `x` values.


## Auxiliary signals

Serum has a system of "auxilliary" modulation sources, which allows basically modulating modulations. I do find it useful to think of it this way, instead of having to create a new modulation to modulate this one. So, better workflow, more optimal code, and it replaces certain kinds of hidden details like envelope velocity tracking.

Serum works in this way:

    y = x + (modSrcVal * scale * auxVal * auxScale)

I don't really like this though; I don't find it that useful. An aux source to me is an attenuation of the main modulation. It's not just another multiplication.

The example which illustrates this is: Envelope -> Volume, with velocity aux. This is probably the most common type of modulation imagineable. In this scenario, the mod scale sets the envelope peak, and aux amount would be the amount of attenuation caused by velocity.

The Serum way means this is not a very useful interaction. With a "small amount of velocity-based attenuation", i.e. a small aux scale, multiplying like above causes the env peak to be very quiet.

What we really want is that the aux value does a `lerp` between an `(attenuated modscale)` and `modscale`.

    scale = lerp(modScale - auxScale, modScale, auxVal)
    y = x + (modSrcVal * scale)

Now e.g. when `modScale=0.5`, `velocity=0.1`, and `auxScale=0.333`,

    scale = lerp(0.5 - 0.333, 0.5, 0.1)
    y = x + (modSrcVal * scale)

There's a pretty optimal way of doing this, by precalculating an `auxBase` (the left edge of the `lerp`).

    auxBase = 1 - auxScale // precalculated
    auxMul = auxBase + auxVal * auxScale

