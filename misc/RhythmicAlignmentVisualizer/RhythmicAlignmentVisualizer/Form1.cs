using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Media;
using System.Windows.Forms;
//using NAudio.Wave; // If using NAudio

namespace RhythmicAlignmentVisualizer
{

    public partial class Form1 : Form
    {
        private const int DISPLAY_WIDTH = 128;
        private const int DISPLAY_HEIGHT = 64;
        private Bitmap displayBitmap;
        private FastDisplay display;

        private Metronome metronome;
        private Timer frameTimer;

        ProgressBarVisualization progressBarVisualization = new ProgressBarVisualization();
        LissajousVisualizer lissajousVisualizer = new LissajousVisualizer();
        XorPatternVisualizer xorVisualizer = new XorPatternVisualizer();
        ShadedTwisterVisualizer twisterVis = new ShadedTwisterVisualizer();
        GoniometerVisualizer goniometerVisualizer = new GoniometerVisualizer();
        SineFieldVisualizer sinefield = new SineFieldVisualizer();
        IVisualization currentVis;

        List<IVisualization> visualizationList;

        public Form1()
        {
            currentVis = progressBarVisualization;

            visualizationList = new List<IVisualization>()
            {
                progressBarVisualization,
                lissajousVisualizer,
                xorVisualizer,
                twisterVis,
                goniometerVisualizer,
                sinefield,
            };


            InitializeComponent();
            InitializeDisplay();
            InitializeMetronome();
            InitializeInput();
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            base.OnFormClosing(e);
            frameTimer?.Stop();
            frameTimer?.Dispose();
            metronome?.Dispose();
        }


        private void InitializeMetronome()
        {
            metronome = new Metronome();
        }

        private int CalculateInterval(int bpm)
        {
            // BPM to milliseconds per beat
            return (int)(60000.0 / bpm);
        }

        private void btnStartStop_Click(object sender, EventArgs e)
        {
        }

        private void txtBPM_TextChanged(object sender, EventArgs e)
        {
            if (float.TryParse(txtBPM.Text, out float newBpm))
            {
                metronome.BPM = newBpm;
            }
        }

        private void InitializeDisplay()
        {
            display = new FastDisplay(DISPLAY_WIDTH, DISPLAY_HEIGHT);
            displayBitmap = display.GetBitmap();
            picDisplay.Image = displayBitmap;
        }

        private void PicDisplay_Paint(object sender, PaintEventArgs e)
        {
            e.Graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            e.Graphics.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.Half;
            e.Graphics.DrawImage(displayBitmap, new Rectangle(0, 0, picDisplay.Width, picDisplay.Height));
        }

        private void InitializeInput()
        {
            this.KeyPreview = true;
            this.KeyDown += Form1_KeyDown;
        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            var trapKeys = new[]
            {
                Keys.A, Keys.O, Keys.E, Keys.U,
                Keys.H, Keys.T, Keys.N, Keys.S,
                Keys.Space,
            };

            if (trapKeys.Contains(e.KeyCode))
            {
                foreach (var viz in visualizationList)
                {
                    viz.OnNoteOn();
                }
                e.SuppressKeyPress = true;
            }
        }


        private void FrameTimer_Tick(object sender, EventArgs e)
        {
            display.BeginFrame();
            currentVis.Update();
            display.EndFrame();
            picDisplay.Refresh();

            lblPhase.Text = $"phase: {xorVisualizer.noteBeatPhase01}";
            lblRatio.Text = $"ratio: {xorVisualizer.lastPlayedNoteBpmRatio}";

            lblLissajousRatio.Text = $"ratio: {lissajousVisualizer.Ratio}";
            lblSeinfield.Text = $"ratio: {sinefield.Ratio}";
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            frameTimer = new Timer();
            frameTimer.Interval = 20; // 20ms ~50 FPS
            frameTimer.Tick += FrameTimer_Tick;
            frameTimer.Start();

            foreach (var viz in visualizationList)
            {
                viz.Init(display, metronome);
            }
        }

        private void btnLissajous_Click(object sender, EventArgs e)
        {
            currentVis = lissajousVisualizer;
        }

        private void btnProgress_Click(object sender, EventArgs e)
        {
            currentVis = progressBarVisualization;
        }

        private void btnXor_Click(object sender, EventArgs e)
        {
            currentVis = xorVisualizer;
        }

        private void tbRatio_Scroll(object sender, EventArgs e)
        {
            float ratio = (float)tbRatio.Value / 1000;
            float phase = (float)tbPhase.Value / 1000;
            xorVisualizer.SetRatioAndPhase(ratio, phase);
            twisterVis.s = phase;
            sinefield.SetRatio(ratio);
            twisterVis.brightness = tbBrightness.Value;
            progressBarVisualization.mBrightness = tbBrightness.Value;
            lissajousVisualizer.SetRatio(ratio);
        }

        private void tbPhase_Scroll(object sender, EventArgs e)
        {
            tbRatio_Scroll(sender, e);
        }

        private void btnTwister_Click(object sender, EventArgs e)
        {
            currentVis = twisterVis;
        }

        private void btnGoniometer_Click(object sender, EventArgs e)
        {
            currentVis = goniometerVisualizer;
        }

        private void tbBrightness_Scroll(object sender, EventArgs e)
        {
            tbRatio_Scroll(sender, e);
        }

        private void btnSeinfield_Click(object sender, EventArgs e)
        {
            currentVis = this.sinefield;
        }
    }
}
