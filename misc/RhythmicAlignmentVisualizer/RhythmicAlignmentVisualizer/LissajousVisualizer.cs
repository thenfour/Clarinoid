using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace RhythmicAlignmentVisualizer
{

    public class MovingAverage
    {
        private Queue<double> samples = new Queue<double>();
        private int windowSize = 2;
        private double sampleAccumulator = 0;
        public double Average { get; private set; }

        public void Clear()
        {
            sampleAccumulator = 0;
            samples.Clear();
        }

        /// <summary>
        /// Computes a new windowed average each time a new sample arrives
        /// </summary>
        /// <param name="newSample"></param>
        public void Update(double newSample)
        {
            sampleAccumulator += newSample;
            samples.Enqueue(newSample);

            if (samples.Count > windowSize)
            {
                sampleAccumulator -= samples.Dequeue();
            }

            Average = sampleAccumulator / samples.Count;
        }
    }

    // note: moving avg is not necessary; it doesn't make things clearer
    // note: phase restarting is also not necessary. it causes more chaos than it solves
    public class LissajousVisualizerCurve
    {
        private Stopwatch sw = Stopwatch.StartNew();
        private IDisplay display;
        private IMetronome metronome;

        private MovingAverage movingAverageFrequencyY = new MovingAverage();

        public double frequencyX;
        public double frequencyY;

        // Visual parameters
        private int centerX = 64;
        private int centerY = 32;
        private double amplitudeX = 60f;
        private double amplitudeY = 30f;

        private double ratioMultiplier = 2.0; // because the player is always playing FASTER than the metronome (typically 4x)

        public LissajousVisualizerCurve(IDisplay display, IMetronome metronome)
        {
            this.display = display;
            this.metronome = metronome;

            frequencyX = metronome.BPM / 60f;
            frequencyY = frequencyX;
            movingAverageFrequencyY.Update(frequencyY);
            metronome.OnTick += Metronome_OnTick;

            phaseX = 0f;
            phaseY = 0f;
        }

        private void Metronome_OnTick()
        {
            frequencyX = metronome.BPM / 60f;
            //phaseX = phaseY = 0f;
        }

        Stopwatch noteTimer = Stopwatch.StartNew();

        public void OnNoteOn()
        {
            var secondsSinceLastNote = noteTimer.Elapsed.TotalSeconds;
            if (secondsSinceLastNote > 2)
            {
                movingAverageFrequencyY.Clear();
            }
            // because you're always playing FASTER than metronome
            secondsSinceLastNote *= ratioMultiplier;
            noteTimer.Restart();
            var frequency = 1 / Math.Max(0.0001, secondsSinceLastNote);

            frequencyY = frequency;
            movingAverageFrequencyY.Update(frequencyY);

            //phaseX = phaseY = 0f;
        }

        public void SetRatio(double ratio)
        {
            //phaseX = phaseY = 0f;
            frequencyY = (ratio) * frequencyX;
            movingAverageFrequencyY.Clear();
            movingAverageFrequencyY.Update(frequencyY);
        }

        double phaseX = 0;
        double phaseY = 0;

        public void Update()
        {

            const int SEGMENT_COUNT = 32;
            const double maxPhasePerFrame = Math.PI*3;


            //double maxPhase = (double)(8 * Math.PI); // just 1 cycle is kinda OK but because the ratios are different you should do more to really highlight the chaos.
            double phaseStep = maxPhasePerFrame / SEGMENT_COUNT;
            var phaseIncX = phaseStep * frequencyX;
            var phaseIncY = phaseStep * frequencyY;
            //var phaseIncY = phaseStep * movingAverageFrequencyY.Average;
            var phX = phaseX;
            var phY = phaseY;
            Point? prev = null;
            for (double a = 0; a < maxPhasePerFrame; a += phaseStep)
            {
                // draw segment of lissajous
                phX += phaseIncX;
                phY += phaseIncY;

                phaseX += phaseIncX;
                phaseY += phaseIncY;

                // Calculate sine values from the current phases
                double xSine = (double)Math.Sin(phX);
                double ySine = (double)Math.Sin(phY);

                // Map them to screen coords
                int x = centerX + (int)(amplitudeX * xSine);
                int y = centerY + (int)(amplitudeY * ySine);

                if (prev != null)
                {
                    int brightness = 255;
                    //int brightness = (int)(a * 255 / maxPhasePerFrame);

                    display.DrawLineWithBrightness(FastDisplay.Bayer8x8Matrix, prev.Value.X, prev.Value.Y,
                        x, y, brightness);
                }
                prev = new Point(x, y);
            }

            sw.Restart();
        }

    }



    public class LissajousVisualizer : IVisualization
    {
        private List<LissajousVisualizerCurve> curves = new List<LissajousVisualizerCurve> ();
        private IDisplay display;
        private IMetronome metronome;
        private const int curveCount = 1;

        public double Ratio => curves[0].frequencyX / curves[0].frequencyY;

        public void Init(IDisplay display, IMetronome metronome)
        {
            this.display = display;
            this.metronome = metronome;

            for (int i = 0; i < curveCount; i++)
            {
                //curves.Add(new LissajousVisualizerCurve(display, metronome, (float)(i*2*Math.PI/curveCount)));
                curves.Add(new LissajousVisualizerCurve(display, metronome));
            }
        }

        public void OnNoteOn()
        {
            foreach (var curve in curves)
            {
                curve.OnNoteOn();
            }
        }

        public void SetRatio(float ratio)
        {
            foreach (var curve in curves)
            {
                curve.SetRatio(ratio);
            }
        }

        public void Update()
        {
            display.ClearDisplay();

            foreach (var curve in curves)
            {
                curve.Update();
            }
        }
    }
}