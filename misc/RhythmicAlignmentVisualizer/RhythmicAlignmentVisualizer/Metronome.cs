using RhythmicAlignmentVisualizer;
using System;
using System.Diagnostics;
using System.Timers;

public class Metronome : IMetronome, IDisposable
{
    private float bpm;
    private readonly Stopwatch stopwatch;
    private readonly Timer metronomeTimer;
    private float beatDuration; // Duration of one beat in seconds
    private readonly SoundManager soundManager;

    public event Action OnTick;

    public Metronome(float initialBPM = 120f)
    {
        soundManager = new SoundManager("tick.wav");
        metronomeTimer = new Timer();
        metronomeTimer.Interval = CalculateInterval(BPM) * 1000; // Convert to milliseconds
        metronomeTimer.Elapsed += MetronomeTimer_Elapsed;
        metronomeTimer.AutoReset = true;

        BPM = initialBPM;
        stopwatch = new Stopwatch();
        stopwatch.Start();

        metronomeTimer.Start();
    }

    public float BPM
    {
        get => bpm;
        set
        {
            bpm = Math.Max(10, value);
            beatDuration = 60f / bpm;
            metronomeTimer.Interval = CalculateInterval(bpm) * 1000; // Update timer interval
        }
    }

    public float CurrentBeatFrac01
    {
        get
        {
            if (beatDuration <= 0) return 0f;
            double elapsedSeconds = stopwatch.Elapsed.TotalSeconds;
            float frac = (float)((elapsedSeconds % beatDuration) / beatDuration);
            return frac;
        }
    }

    public int CurrentBeatSerialNumber { get; private set; } = 1;

    public double TimeSinceLastTick // seconds.
    {
        get => stopwatch.Elapsed.TotalSeconds;
    }

    private void MetronomeTimer_Elapsed(object sender, ElapsedEventArgs e)
    {
        CurrentBeatSerialNumber++;
        // Play the tick sound
        soundManager.PlaySound();

        // Reset the stopwatch to mark the start of the new beat
        stopwatch.Restart();

        // Raise the OnTick event
        OnTick?.Invoke();
    }

    private float CalculateInterval(float bpm)
    {
        // Returns the interval in seconds for one beat
        return 60f / bpm;
    }

    public void Dispose()
    {
        metronomeTimer?.Stop();
        metronomeTimer?.Dispose();
        stopwatch?.Stop();
        soundManager?.Dispose();
    }
}