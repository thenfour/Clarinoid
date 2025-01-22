using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RhythmicAlignmentVisualizer
{

    public class ShadedTwisterVisualizer : IVisualization
    {
        private IDisplay display;
        private IMetronome metronome;

        public void Init(IDisplay display, IMetronome metronome)
        {
            this.display = display;
            this.metronome = metronome;
        }

        public float s;
        public int brightness = 255;

        public void OnNoteOn()
        {
        }

        public void Update()
        {
            display.ClearDisplay();

            //int brightness = (int)Math.Round(255 * metronome.CurrentBeatFrac01);

            //display.DrawLineWuFixedPoint(0, 0, 64, (int)(64 * metronome.CurrentBeatFrac01), brightness, FastDisplay.Bayer8x8Matrix);
            display.DrawLineWithBrightness(FastDisplay.Bayer8x8Matrix, 0, 10, 64, 10 + (int)(64 * metronome.CurrentBeatFrac01), brightness);

            //display.DrawLineWuFixedPoint(64, (int)(64 * s), 128, 0, brightness, FastDisplay.Bayer8x8Matrix);
            display.DrawLineWithBrightness(FastDisplay.Bayer8x8Matrix, 64, 10 + (int)(64 * s), 128, 10, brightness);
        }
    }

}
