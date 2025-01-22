using NAudio.Wave;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RhythmicAlignmentVisualizer
{

    public class SoundManager : IDisposable
    {
        private IWavePlayer waveOut;
        private AudioFileReader audioFile;

        public SoundManager(string audioFilePath)
        {
            // Initialize the WaveOutEvent for playback
            waveOut = new WaveOutEvent();

            // Initialize the AudioFileReader with the tick sound
            audioFile = new AudioFileReader(audioFilePath);

            // Set the playback device to the WaveOutEvent
            waveOut.Init(audioFile);
        }

        // Play the sound from the beginning
        public void PlaySound()
        {
            if (waveOut.PlaybackState == PlaybackState.Playing)
            {
                // Restart the sound if it's already playing
                waveOut.Stop();
                audioFile.Position = 0;
                waveOut.Play();
            }
            else
            {
                // Play the sound
                audioFile.Position = 0;
                waveOut.Play();
            }
        }

        // Dispose resources
        public void Dispose()
        {
            waveOut?.Stop();
            waveOut?.Dispose();
            audioFile?.Dispose();
        }
    }
}
