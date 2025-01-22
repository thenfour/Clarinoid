using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RhythmicAlignmentVisualizer
{
    public interface IDisplay
    {
        void ClearDisplay();
        void SetPixel(int x, int y, bool on);
        void SetPixelShaded(int x, int y, int brightness, DitherMatrix matrix);
        void SetPixelShaded(int x, int y, float brightness01, DitherMatrix matrix);
        void DrawHLine(int y, int xStart, int xEnd);
        void DrawVLine(int x, int yStart, int yEnd);
        void FillRect(int x, int y, int width, int height);
        void FillRectWithBrightness(DitherMatrix matrix, int x, int y, int width, int height, int brightness);
        void DrawInfiniteLineClipped(
            int x0, int y0,
            int x1, int y1,
            int clipLeft, int clipTop,
            int clipRight, int clipBottom,
            DitherMatrix matrix,
            int brightness);
        void DrawLineWithBrightness(DitherMatrix matrix, int x0, int y0, int x1, int y1, int brightness);
        void DrawCircle(float cx, float cy, float r);
        void FillCircleWithBrightness(
            DitherMatrix matrix,
            int x0,   // center X
            int y0,   // center Y
            int r,    // radius
            int brightness);

        void DrawHLineDithered(
            DitherMatrix matrix,
            int x, int y,
            int length,
            int brightness);

        //void DrawLineWuFixedPoint(
        //int x0, int y0,
        //int x1, int y1,
        //int brightness,
        //DitherMatrix matrix);
    }

    public interface IMetronome
    {
        float BPM { get; set; }
        float CurrentBeatFrac01 { get; }
        int CurrentBeatSerialNumber { get; }
        double TimeSinceLastTick { get; }

        // Optional: Event to notify visualizations on each metronome tick
        event Action OnTick;
    }

    public interface IVisualization
    {
        void Init(IDisplay display, IMetronome metronome);
        void OnNoteOn();
        void Update(); // called once every frame (~20ms)
    }
}
