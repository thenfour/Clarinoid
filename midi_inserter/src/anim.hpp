#pragma once

#include "gui.hpp"

void DoDemo1(MidiPerformerGui &gui)
{
    // startup sequence
    int triFrame = 0;
    int txrxFrame = 0;
    int ws2812Frame = 0;
    clarinoid::Stopwatch swws2812;
    clarinoid::Stopwatch swtriangle;
    clarinoid::Stopwatch swtxrx;
    while (true)
    {
        if (swtriangle.ElapsedTime().ElapsedMillisI() > 120)
        {
            swtriangle.Restart();
            triFrame++;

            if ((triFrame % 3) == 0)
            {
                gui.TriggerOrange();
            }
            if ((triFrame % 3) == 1)
            {
                gui.TriggerBlue();
            }
            if ((triFrame % 3) == 2)
            {
                gui.TriggerRed();
            }
        }
        if (swtxrx.ElapsedTime().ElapsedMillisI() > 80)
        {
            swtxrx.Restart();
            txrxFrame++;
            if ((txrxFrame % 4) == 0)
            {
                gLedPin_MidiA_RX.Trigger();
            }
            if ((txrxFrame % 4) == 1)
            {
                gLedPin_MidiB_TX.Trigger();
            }
            if ((txrxFrame % 4) == 2)
            {
                gLedPin_MidiB_RX.Trigger();
            }
            if ((txrxFrame % 4) == 3)
            {
                gLedPin_MidiA_TX.Trigger();
            }
        }
        if (swws2812.ElapsedTime().ElapsedMillisI() > 2)
        {
            swws2812.Restart();
            ws2812Frame++;
            if (ws2812Frame > 24 * 7)
                break;
            //  0: first we fill 12 leds with red,
            // 12: then erase them
            // 24: then fill with green
            // 36: erase them
            //
            int shade = (ws2812Frame / 24) % 7; // this is if it's red, green, blue, or otherwise.
            int brightness = 16;
            shade += 1;
            int r = brightness * (shade & 1);
            int g = brightness * ((shade >> 1) & 1);
            int b = brightness * ((shade >> 2) & 1);
            int iled = ws2812Frame % 24; // which led frame within this shade?
            if (iled < 12)
            {
                // fill
                for (int i = 0; i < 12; ++i)
                {
                    if (i > iled)
                    {
                        gLedstrip.setPixelColor(i, 0);
                    }
                    else
                    {
                        gLedstrip.setPixelColor(i, r, g, b);
                    }
                }
            }
            else
            {
                // erase
                iled -= 12;
                for (int i = 0; i < 12; ++i)
                {
                    if (i < iled)
                    {
                        gLedstrip.setPixelColor(i, 0);
                    }
                    else
                    {
                        gLedstrip.setPixelColor(i, r, g, b);
                    }
                }
            }
            gLedstrip.Render();
        }

        PresentLeds(true);
        delay(12);
    }

    gLedPin_OrangeTriangle.SetLevel(0);
    gLedPin_BlueTriangle.SetLevel(0);
    gLedPin_RedTriangle.Trigger();
    gLedPin_MidiA_RX.Trigger();
    gLedPin_MidiB_TX.Trigger();
    gLedPin_MidiB_RX.Trigger();
    gLedPin_MidiA_TX.Trigger();

    for (int i = 0; i < 12; ++i)
    {
        gLedstrip.setPixelColor(i, 0);
    }
    gLedstrip.Render();
    PresentLeds();
}
