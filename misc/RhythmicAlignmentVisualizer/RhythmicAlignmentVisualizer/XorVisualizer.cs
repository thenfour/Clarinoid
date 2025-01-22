using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RhythmicAlignmentVisualizer
{


    public class XorPatternVisualizer : IVisualization
    {
        private IDisplay display;
        private IMetronome metronome;

        private int width = 128;
        private int height = 64;
        private float checkerSize = 3; // Changed to float for precision

        private Stopwatch noteStopwatch = new Stopwatch();
        public float lastPlayedNoteBpmRatio = 1f;
        public float noteBeatPhase01 = 0f;
        private Stopwatch sw = new Stopwatch();

        private bool isFirstNote = true; // Flag to handle the first note-on event

        public void Init(IDisplay display, IMetronome metronome)
        {
            this.display = display;
            this.metronome = metronome;

            width = 128;
            height = 64;

            sw.Restart();
        }

        public void SetRatioAndPhase(float ratio, float phase01)
        {
            noteBeatPhase01 = phase01;
            lastPlayedNoteBpmRatio = ratio;
        }

        public void OnNoteOn()
        {
            // Get the duration of one beat in seconds
            float metronomeSeconds = 60f / metronome.BPM;

            if (!noteStopwatch.IsRunning)
            {
                // First note-on event; initialize the stopwatch without calculating the ratio
                noteStopwatch.Start();
                isFirstNote = false;
                return;
            }

            // Calculate the interval between the current and the last note-on event
            double intervalSeconds = noteStopwatch.Elapsed.TotalSeconds;

            if (intervalSeconds > 0)
            {
                // Calculate the ratio of metronome beat duration to note interval
                lastPlayedNoteBpmRatio = (float)(metronomeSeconds / intervalSeconds);
            }
            else
            {
                lastPlayedNoteBpmRatio = 1f; // Default ratio if interval is zero
            }

            // Capture the current beat phase from the metronome
            noteBeatPhase01 = metronome.CurrentBeatFrac01;

            // Restart the stopwatch for the next interval
            noteStopwatch.Restart();
        }

        public void OnMetronomeTick()
        {
            // Optional: Implement any behavior upon metronome ticks if needed
        }

        public static bool Xor(bool a, bool b) => a != b;

        public void Update()
        {
            // Clear the display for the new frame
            display.ClearDisplay();

            // Generate Pattern A (Metronome-Based Checkerboard)
            bool[,] patternA = GenerateCheckerPattern(
                phaseShift: 0f, // Static pattern for metronome-based checkers
                effectiveCheckerSize: checkerSize,  // Frequency can be adjusted based on BPM if desired
                shiftX: 0f,
                shiftY: 0f
            );

            // put the ratio in a 0.5 - 1.5 range
            float effectiveRatio = ((lastPlayedNoteBpmRatio % 1) + 0.5f);

            bool[,] patternB = GenerateCheckerPattern(
                phaseShift: noteBeatPhase01, // Shift based on the beat phase
                effectiveCheckerSize: effectiveRatio * 6 * checkerSize,               // Frequency adjusted by the ratio
                shiftX: 0f,
                shiftY: 0f
            );

            // Apply XOR between Pattern A and Pattern B
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    bool xorPixel = Xor(patternA[x, y], patternB[x, y]);
                    //bool xorPixel = patternB[x,y];
                    display.SetPixel(x, y, xorPixel);
                }
            }
        }

        private bool[,] GenerateCheckerPattern(float phaseShift, float effectiveCheckerSize, float shiftX, float shiftY)
        {
            bool[,] pattern = new bool[width, height];
            float phaseShiftInPixels = phaseShift * effectiveCheckerSize;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    float checkerX = (x + phaseShiftInPixels) / effectiveCheckerSize;
                    float checkerY = (y + phaseShiftInPixels) / effectiveCheckerSize;

                    // Use floor to get the checker index
                    int checkerIdxX = (int)Math.Floor(checkerX);
                    int checkerIdxY = (int)Math.Floor(checkerY);

                    // Set the pixel based on the checker indices
                    pattern[x, y] = (checkerIdxX + checkerIdxY) % 2 == 0;
                }
            }

            return pattern;
        }
    }
}