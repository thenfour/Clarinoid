using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace RhythmicAlignmentVisualizer
{

    public class SineFieldVisualizer : IVisualization
    {
        private List<LissajousVisualizerCurve> curves = new List<LissajousVisualizerCurve>();
        private IDisplay display;
        private IMetronome metronome;
        private const int curveCount = 1;

        public float frequencyX;
        public float frequencyY;

        public float Ratio => frequencyY / frequencyX;

        public void Init(IDisplay display, IMetronome metronome)
        {
            this.display = display;
            this.metronome = metronome;

            frequencyX = metronome.BPM / 60f;
            frequencyY = frequencyX;
            metronome.OnTick += Metronome_OnTick;
        }

        private void Metronome_OnTick()
        {
            frequencyX = metronome.BPM / 60f;
        }

        Stopwatch noteTimer = Stopwatch.StartNew();
        public void OnNoteOn()
        {
            var secondsSinceLastNote = noteTimer.Elapsed.TotalSeconds;
            noteTimer.Restart();
            var frequency = (float)(1 / Math.Max(0.0001, secondsSinceLastNote));
            frequencyY = frequency;
        }

        public void SetRatio(float ratio)
        {
            frequencyY = ratio * frequencyX;
        }

        private static float ZeroBeginningRange(float val01, float zeroRange01)
        {
            float mappedValue = val01 < zeroRange01 ? 0 : (val01 - zeroRange01) / (1 - zeroRange01);
            return mappedValue;
        }

        public void Update()
        {
            display.ClearDisplay();
            int height = 64;
            int width = 128;
            int scale = 4;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    float value01 = (float)Math.Sin((float)(x - width * .5f) / scale) *.5f+.5f;
                    value01 += (float)Math.Sin((float)(y - height * .5f) / scale) * .5f + .5f;
                    value01 *= 0.5f; // 0-1

                    float valueB01 = (float)Math.Sin((float)(x - width * .5f) / scale * Ratio * 2 * Math.PI) * .5f + .5f;
                    valueB01 += (float)Math.Sin((float)(y - height * .5f) / scale * Ratio * 2 * Math.PI) * .5f + .5f;
                    valueB01 *= 0.5f; // 0-1

                    float value = value01 - valueB01;
                    display.SetPixelShaded(x, y, value, FastDisplay.Bayer8x8Matrix);
                }
            }
        }
    }
}