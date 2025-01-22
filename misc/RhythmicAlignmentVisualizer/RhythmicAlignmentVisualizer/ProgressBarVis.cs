using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RhythmicAlignmentVisualizer
{

    public class ProgressBarVisualization : IVisualization
    {
        private IDisplay display;
        private IMetronome metronome;

        public void Init(IDisplay display, IMetronome metronome)
        {
            this.display = display;
            this.metronome = metronome;
        }

        public int mBrightness = 255;

        public void OnNoteOn()
        {
        }

        public void Update()
        {
            display.ClearDisplay();

            int width = 30;
            int height = 30;
            int brightness = (int)Math.Round(255 * metronome.CurrentBeatFrac01);

            //display.FillRectWithBrightness(FastDisplay.Bayer2x2Matrix, 0 * width, 0 * height, width - 1, height - 1, brightness);
            //display.FillRectWithBrightness(FastDisplay.Bayer4x4Matrix, 1 * width, 0 * height, width - 1, height - 1, brightness);
            //display.FillRectWithBrightness(FastDisplay.Bayer8x8Matrix, 2 * width, 0 * height, width - 1, height - 1, brightness);

            //display.FillRectWithBrightness(FastDisplay.Bayer2x2Matrix, 0 * width, 1 * height, width - 1, height - 1, mBrightness);
            //display.FillRectWithBrightness(FastDisplay.Bayer4x4Matrix, 1 * width, 1 * height, width - 1, height - 1, mBrightness);
            //display.FillRectWithBrightness(FastDisplay.Bayer8x8Matrix, 2 * width, 1 * height, width - 1, height - 1, mBrightness);

            display.DrawHLineDithered(FastDisplay.Bayer8x8Matrix, 4, 16, 120, mBrightness);
            display.FillCircleWithBrightness(FastDisplay.Bayer8x8Matrix, 64, 32, 12, mBrightness);
        }
    }

}
