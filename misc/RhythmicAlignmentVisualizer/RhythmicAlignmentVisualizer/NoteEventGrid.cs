using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RhythmicAlignmentVisualizer
{
    // 
    public class NoteEventGrid : IVisualization
    {
        private IDisplay display;
        private IMetronome metronome;

        private const int gColumns = 24; // divisions per beat
        private const int gRows = 8;
        private static Size gCellSize = new Size(5, 5);

        private const int gCellCount = gColumns * gRows;
        private const int gCurrentCell = 0;
        private int gPixelsPerRow = gColumns * gCellSize.Width;
        private static int gLinearPixelCount = gCellCount * gCellSize.Width;

        // you cannot store more notes than pixels on the screen.
        private int[] mNoteEventLinearPixelPositions = new int[gLinearPixelCount];

        // when moving to a new cell, erase all notes in that cell.
        // can be done by just remembering last cell drawn.
        // if it changes, erase items in mNoteEventLinearPixelPositions

        private Point GetCurrentCell()
        {
            int row = metronome.CurrentBeatSerialNumber % gRows;
            int column = (int)Math.Floor(metronome.CurrentBeatFrac01 * gColumns);
            return new Point(column, row);
        }

        private int GetCurrentLinearPixel()
        {
            // 1 = row = 1 beat, but with offset
            return (int)Math.Floor(metronome.CurrentBeatFrac01 * gPixelsPerRow);
        }

        public void Init(IDisplay display, IMetronome metronome)
        {
            this.display = display;
            this.metronome = metronome;
            for (int i = 0; i < gLinearPixelCount; i++)
            {
                mNoteEventLinearPixelPositions[i] = -1;
            }
        }

        public void OnNoteOn()
        {
        }

        public void Update()
        {
            // remove events older than ...
            Render();
        }

        public void Render()
        {
            // draw grid
            // draw note events
            display.ClearDisplay();
            // display.DrawHLine
        }
    }
}
