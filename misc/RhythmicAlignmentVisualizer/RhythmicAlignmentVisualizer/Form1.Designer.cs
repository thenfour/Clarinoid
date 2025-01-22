namespace RhythmicAlignmentVisualizer
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.txtBPM = new System.Windows.Forms.TextBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.btnLissajous = new System.Windows.Forms.Button();
            this.btnProgress = new System.Windows.Forms.Button();
            this.btnXor = new System.Windows.Forms.Button();
            this.tbRatio = new System.Windows.Forms.TrackBar();
            this.tbPhase = new System.Windows.Forms.TrackBar();
            this.lblPhase = new System.Windows.Forms.Label();
            this.lblRatio = new System.Windows.Forms.Label();
            this.btnTwister = new System.Windows.Forms.Button();
            this.btnGoniometer = new System.Windows.Forms.Button();
            this.lblBrightness = new System.Windows.Forms.Label();
            this.tbBrightness = new System.Windows.Forms.TrackBar();
            this.lblLissajousRatio = new System.Windows.Forms.Label();
            this.btnSeinfield = new System.Windows.Forms.Button();
            this.picDisplay = new RhythmicAlignmentVisualizer.DoubleBufferedPictureBox();
            this.lblSeinfield = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.tbRatio)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbPhase)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbBrightness)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.picDisplay)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(57, 26);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(27, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "bpm";
            // 
            // txtBPM
            // 
            this.txtBPM.Location = new System.Drawing.Point(214, 18);
            this.txtBPM.Name = "txtBPM";
            this.txtBPM.Size = new System.Drawing.Size(100, 20);
            this.txtBPM.TabIndex = 2;
            this.txtBPM.Text = "90";
            this.txtBPM.TextChanged += new System.EventHandler(this.txtBPM_TextChanged);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(388, 26);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(100, 20);
            this.textBox1.TabIndex = 4;
            // 
            // btnLissajous
            // 
            this.btnLissajous.Location = new System.Drawing.Point(241, 332);
            this.btnLissajous.Name = "btnLissajous";
            this.btnLissajous.Size = new System.Drawing.Size(75, 23);
            this.btnLissajous.TabIndex = 5;
            this.btnLissajous.Text = "btnLissajous";
            this.btnLissajous.UseVisualStyleBackColor = true;
            this.btnLissajous.Click += new System.EventHandler(this.btnLissajous_Click);
            // 
            // btnProgress
            // 
            this.btnProgress.Location = new System.Drawing.Point(241, 362);
            this.btnProgress.Name = "btnProgress";
            this.btnProgress.Size = new System.Drawing.Size(75, 23);
            this.btnProgress.TabIndex = 6;
            this.btnProgress.Text = "btnProgress";
            this.btnProgress.UseVisualStyleBackColor = true;
            this.btnProgress.Click += new System.EventHandler(this.btnProgress_Click);
            // 
            // btnXor
            // 
            this.btnXor.Location = new System.Drawing.Point(241, 391);
            this.btnXor.Name = "btnXor";
            this.btnXor.Size = new System.Drawing.Size(75, 23);
            this.btnXor.TabIndex = 7;
            this.btnXor.Text = "btnXor";
            this.btnXor.UseVisualStyleBackColor = true;
            this.btnXor.Click += new System.EventHandler(this.btnXor_Click);
            // 
            // tbRatio
            // 
            this.tbRatio.Location = new System.Drawing.Point(151, 415);
            this.tbRatio.Maximum = 2000;
            this.tbRatio.Minimum = 1;
            this.tbRatio.Name = "tbRatio";
            this.tbRatio.Size = new System.Drawing.Size(652, 45);
            this.tbRatio.TabIndex = 8;
            this.tbRatio.Value = 110;
            this.tbRatio.Scroll += new System.EventHandler(this.tbRatio_Scroll);
            // 
            // tbPhase
            // 
            this.tbPhase.Location = new System.Drawing.Point(154, 461);
            this.tbPhase.Maximum = 1000;
            this.tbPhase.Name = "tbPhase";
            this.tbPhase.Size = new System.Drawing.Size(652, 45);
            this.tbPhase.TabIndex = 9;
            this.tbPhase.Scroll += new System.EventHandler(this.tbPhase_Scroll);
            // 
            // lblPhase
            // 
            this.lblPhase.AutoSize = true;
            this.lblPhase.Location = new System.Drawing.Point(72, 461);
            this.lblPhase.Name = "lblPhase";
            this.lblPhase.Size = new System.Drawing.Size(48, 13);
            this.lblPhase.TabIndex = 10;
            this.lblPhase.Text = "phase01";
            // 
            // lblRatio
            // 
            this.lblRatio.AutoSize = true;
            this.lblRatio.Location = new System.Drawing.Point(72, 415);
            this.lblRatio.Name = "lblRatio";
            this.lblRatio.Size = new System.Drawing.Size(45, 13);
            this.lblRatio.TabIndex = 11;
            this.lblRatio.Text = "ratio 0-2";
            // 
            // btnTwister
            // 
            this.btnTwister.Location = new System.Drawing.Point(151, 546);
            this.btnTwister.Name = "btnTwister";
            this.btnTwister.Size = new System.Drawing.Size(75, 23);
            this.btnTwister.TabIndex = 12;
            this.btnTwister.Text = "twister";
            this.btnTwister.UseVisualStyleBackColor = true;
            this.btnTwister.Click += new System.EventHandler(this.btnTwister_Click);
            // 
            // btnGoniometer
            // 
            this.btnGoniometer.Location = new System.Drawing.Point(241, 546);
            this.btnGoniometer.Name = "btnGoniometer";
            this.btnGoniometer.Size = new System.Drawing.Size(75, 23);
            this.btnGoniometer.TabIndex = 13;
            this.btnGoniometer.Text = "goniometer";
            this.btnGoniometer.UseVisualStyleBackColor = true;
            this.btnGoniometer.Click += new System.EventHandler(this.btnGoniometer_Click);
            // 
            // lblBrightness
            // 
            this.lblBrightness.AutoSize = true;
            this.lblBrightness.Location = new System.Drawing.Point(72, 495);
            this.lblBrightness.Name = "lblBrightness";
            this.lblBrightness.Size = new System.Drawing.Size(55, 13);
            this.lblBrightness.TabIndex = 15;
            this.lblBrightness.Text = "brightness";
            // 
            // tbBrightness
            // 
            this.tbBrightness.Location = new System.Drawing.Point(154, 495);
            this.tbBrightness.Maximum = 255;
            this.tbBrightness.Name = "tbBrightness";
            this.tbBrightness.Size = new System.Drawing.Size(652, 45);
            this.tbBrightness.TabIndex = 14;
            this.tbBrightness.Scroll += new System.EventHandler(this.tbBrightness_Scroll);
            // 
            // lblLissajousRatio
            // 
            this.lblLissajousRatio.AutoSize = true;
            this.lblLissajousRatio.Location = new System.Drawing.Point(322, 342);
            this.lblLissajousRatio.Name = "lblLissajousRatio";
            this.lblLissajousRatio.Size = new System.Drawing.Size(35, 13);
            this.lblLissajousRatio.TabIndex = 16;
            this.lblLissajousRatio.Text = "label2";
            // 
            // btnSeinfield
            // 
            this.btnSeinfield.Location = new System.Drawing.Point(614, 69);
            this.btnSeinfield.Name = "btnSeinfield";
            this.btnSeinfield.Size = new System.Drawing.Size(75, 23);
            this.btnSeinfield.TabIndex = 17;
            this.btnSeinfield.Text = "seinfield";
            this.btnSeinfield.UseVisualStyleBackColor = true;
            this.btnSeinfield.Click += new System.EventHandler(this.btnSeinfield_Click);
            // 
            // picDisplay
            // 
            this.picDisplay.BackColor = System.Drawing.Color.Black;
            this.picDisplay.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.picDisplay.Location = new System.Drawing.Point(60, 69);
            this.picDisplay.Name = "picDisplay";
            this.picDisplay.Size = new System.Drawing.Size(512, 256);
            this.picDisplay.TabIndex = 3;
            this.picDisplay.TabStop = false;
            this.picDisplay.Paint += new System.Windows.Forms.PaintEventHandler(this.PicDisplay_Paint);
            // 
            // lblSeinfield
            // 
            this.lblSeinfield.AutoSize = true;
            this.lblSeinfield.Location = new System.Drawing.Point(715, 74);
            this.lblSeinfield.Name = "lblSeinfield";
            this.lblSeinfield.Size = new System.Drawing.Size(35, 13);
            this.lblSeinfield.TabIndex = 18;
            this.lblSeinfield.Text = "label2";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1149, 627);
            this.Controls.Add(this.lblSeinfield);
            this.Controls.Add(this.btnSeinfield);
            this.Controls.Add(this.lblLissajousRatio);
            this.Controls.Add(this.lblBrightness);
            this.Controls.Add(this.tbBrightness);
            this.Controls.Add(this.btnGoniometer);
            this.Controls.Add(this.btnTwister);
            this.Controls.Add(this.lblRatio);
            this.Controls.Add(this.lblPhase);
            this.Controls.Add(this.tbPhase);
            this.Controls.Add(this.tbRatio);
            this.Controls.Add(this.btnXor);
            this.Controls.Add(this.btnProgress);
            this.Controls.Add(this.btnLissajous);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.picDisplay);
            this.Controls.Add(this.txtBPM);
            this.Controls.Add(this.label1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.tbRatio)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbPhase)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbBrightness)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.picDisplay)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtBPM;
        private DoubleBufferedPictureBox picDisplay;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button btnLissajous;
        private System.Windows.Forms.Button btnProgress;
        private System.Windows.Forms.Button btnXor;
        private System.Windows.Forms.TrackBar tbRatio;
        private System.Windows.Forms.TrackBar tbPhase;
        private System.Windows.Forms.Label lblPhase;
        private System.Windows.Forms.Label lblRatio;
        private System.Windows.Forms.Button btnTwister;
        private System.Windows.Forms.Button btnGoniometer;
        private System.Windows.Forms.Label lblBrightness;
        private System.Windows.Forms.TrackBar tbBrightness;
        private System.Windows.Forms.Label lblLissajousRatio;
        private System.Windows.Forms.Button btnSeinfield;
        private System.Windows.Forms.Label lblSeinfield;
    }
}

