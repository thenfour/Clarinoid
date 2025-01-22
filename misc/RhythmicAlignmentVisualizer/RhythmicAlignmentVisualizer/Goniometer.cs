using RhythmicAlignmentVisualizer;
using System;
using System.Collections.Generic;
using System.Diagnostics;

public class NoteCountScroller
{
    // StopWatch tracks elapsed time since scroller started
    private Stopwatch _stopwatch = Stopwatch.StartNew();

    // RING-BUFFER FOR BUCKETS
    private const int MaxBuckets = 256; // maximum number of buckets to store
    private int[] _buckets = new int[MaxBuckets];

    // We'll track how far time has advanced in terms of buckets
    private int _lastClearedBucketIndex = 0;
    // This marks the oldest bucket index we have cleared so far.

    // SCROLLER CONFIG
    public int BucketWidth = 2;       // horizontal px per bucket
    public int MsPerBucket = 30;     // each bucket spans 100 ms
    public int ScrollerHeight = 10;   // vertical px allocated for scroller
    public int ScreenWidth = 128;     // total screen width in px
    public int ScreenHeight = 55;     // total screen height in px

    //public void OnMetronomeTick(IMetronome metronome)
    //{
    //    MsPerBucket = (int)(60000 / metronome.BPM) / 12;
    //}

    /// <summary>
    /// Called when a note-on event occurs.
    /// Simply increment the current bucket's count.
    /// (We only do clearing in Render, so that even if no notes arrive,
    /// time-based clearing still happens.)
    /// </summary>
    public void OnNoteOn()
    {
        int elapsedMs = (int)_stopwatch.ElapsedMilliseconds;
        int currentBucketIndex = elapsedMs / MsPerBucket;

        // ring pos for the current bucket
        int ringPos = currentBucketIndex % MaxBuckets;
        _buckets[ringPos]++;
    }

    /// <summary>
    /// Renders the scroller along the bottom of the screen.
    /// The newest bucket is at the right edge, older to the left.
    /// Also clears any buckets that are older than the left edge.
    /// </summary>
    public void Render(IDisplay display)
    {
        int screenBottom = ScreenHeight - 1;
        // int scrollerTop = screenBottom - (ScrollerHeight - 1);

        int elapsedMs = (int)_stopwatch.ElapsedMilliseconds;
        int currentBucketIndex = elapsedMs / MsPerBucket;

        // 1) Clear buckets older than the left edge
        // The leftmost bucket index:
        int leftmostBucketIndex = currentBucketIndex - (ScreenWidth / BucketWidth);

        // Move _lastClearedBucketIndex forward until it matches leftmostBucketIndex
        // and set those old buckets to 0.
        while (_lastClearedBucketIndex < leftmostBucketIndex)
        {
            int ringPos = _lastClearedBucketIndex % MaxBuckets;
            // clear that bucket
            _buckets[ringPos] = 0;
            _lastClearedBucketIndex++;
        }

        // 2) Now draw the visible region
        // We'll iterate horizontally from x=0..ScreenWidth-1 in steps of BucketWidth
        for (int screenX = 0; screenX < ScreenWidth; screenX += BucketWidth)
        {
            // distanceFromRight = how many px from the right edge
            int distanceFromRight = (ScreenWidth - 1) - screenX;
            // how many buckets to the left of current
            int bucketOffset = distanceFromRight / BucketWidth;
            int bucketIndex = currentBucketIndex - bucketOffset;

            if (bucketIndex < 0)
            {
                // time before 0 => no data
                continue;
            }

            // ring pos
            int ringPos = bucketIndex % MaxBuckets;
            if (ringPos < 0) ringPos += MaxBuckets; // handle negative mod if needed

            // number of notes that happened in this bucket
            int noteCount = _buckets[ringPos];

            // clamp noteCount so it won't exceed our scroller height
            // each note uses 2 px vertically (1 line + 1 gap),
            // but we only have 'ScrollerHeight' px total
            int maxLines = ScrollerHeight / 2;
            if (noteCount > maxLines)
                noteCount = maxLines;

            // draw each note as a horizontal line 1 px thick, with 1 px gap
            // from bottom to top
            for (int n = 0; n < noteCount; n++)
            {
                int lineY = screenBottom - (n * 2);
                // fill a horizontal line of width BucketWidth at lineY
                display.FillRect(screenX, lineY, BucketWidth, 1 /* height */);
            }
        }
    }
}



public class GoniometerVisualizer : IVisualization
{
    NoteCountScroller noteCountScroller = new NoteCountScroller();
    private IDisplay display;
    private IMetronome metronome;

    private int width = 128;
    private int height = 55;
    private float centerX;
    private float centerY;
    private float radius;

    private class NotePoint
    {
        public Stopwatch whenAdded;
        public int px;
        public int py;
    }

    private readonly List<NotePoint> notePoints = new List<NotePoint>();
    private const int maxMarkers = 16;
    private const float holdMS = 1800;
    private const float decayTimeMS = 3000;

    public void Init(IDisplay display, IMetronome metronome)
    {
        this.display = display;
        this.metronome = metronome;
        this.metronome.OnTick += Metronome_OnTick;

        centerX = width / 2f;
        centerY = height / 2f;
        radius = 26;// Math.Min(width, height) / 2f;
    }

    private void Metronome_OnTick()
    {
        //this.noteCountScroller.OnMetronomeTick(metronome);
    }

    public void OnNoteOn()
    {
        noteCountScroller.OnNoteOn();

        float beatFrac = metronome.CurrentBeatFrac01;
        float currentBeatPos01 = beatFrac - 0.5f;
        float angleDegrees = currentBeatPos01 * 360;
        var pt = DeviationAngleToPoint(angleDegrees);
        notePoints.Add(new NotePoint
        {
            px = (int)Math.Round(pt.x),
            py = (int)Math.Round(pt.y),
            whenAdded = Stopwatch.StartNew()
        });

        if (notePoints.Count > maxMarkers)
            notePoints.RemoveAt(0);
    }

    private int GetBrightness(int elapsedMS, int holdMs, int decayMs)
    {
        float brightness = 1;
        if (elapsedMS > holdMs)
        {
            float fadeStage01 = (float)(elapsedMS - holdMs) / decayMs;
            brightness = Math.Max(0, 1 - fadeStage01);
        }
        return (int)(brightness * 255);
    }

    public void Update()
    {
        // Clear the display for the new frame
        display.ClearDisplay();

        for (int i = 1; i < notePoints.Count; i++)
        {
            var a1 = notePoints[i - 1];
            var a2 = notePoints[i];

            // todo: remove if faded
            int lineBrightness = GetBrightness((int)a2.whenAdded.ElapsedMilliseconds, (int)holdMS, (int)decayTimeMS);

            display.DrawInfiniteLineClipped(
                a1.px,
                a1.py,
                a2.px,
                a2.py,
                0, 0,
                129, 54,
                FastDisplay.Bayer8x8Matrix,
                 lineBrightness >> 2
                );

            display.DrawLineWithBrightness(
                FastDisplay.Bayer8x8Matrix,
                a1.px,
                a1.py,
                a2.px,
                a2.py,
                lineBrightness);

            display.FillCircleWithBrightness(FastDisplay.Bayer8x8Matrix, a2.px, a2.py, 4,
                GetBrightness((int)a2.whenAdded.ElapsedMilliseconds, 0, 800));
        }

        noteCountScroller.Render(display);
    }

    private (float x, float y) DeviationAngleToPoint(float angleDegrees)
    {
        // Convert angle to radians
        float angleRad = angleDegrees * (float)Math.PI / 180f;

        // Calculate marker position on the circumference
        float markerX = centerX + radius * (float)Math.Cos(angleRad);
        float markerY = centerY + radius * (float)Math.Sin(angleRad);
        return (markerX, markerY);
    }
}
