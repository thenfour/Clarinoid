
#pragma once

#include <clarinoid/application/Metronome.hpp>

#include "LoopstationMemory.hpp"
#include "Loopstation.hpp"

namespace clarinoid
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LooperAndHarmonizer
{
  AppSettings* mAppSettings;
  Metronome* mMetronome;
  ScaleFollower *mScaleFollower;

  explicit LooperAndHarmonizer(AppSettings* appSettings, Metronome* mMetronome, ScaleFollower *mScaleFollower) :
    mAppSettings(appSettings),
    mMetronome(mMetronome),
    mScaleFollower(mScaleFollower),

    mHarmonizer(appSettings)
  {
  }


  uint8_t mCurrentlyWritingLayer = 0;
  LoopEventStream mLayers[LOOP_LAYERS];

  LoopStatus mStatus;
  Stopwatch mLoopTimer;

  Harmonizer mHarmonizer;

  bool mArmed = false;
  float mBeatTimeToStartRecording = 0;

  // after the trigger fires (or immediately for LooperTrigger::Immediate, this is what starts loop recording.)
  void TriggerSetStart(const MusicalVoice& liveVoice)
  {
    mArmed = false;
    // reset the loop start, set loop time now.
    mLoopTimer.Restart();
    mCurrentlyWritingLayer = 0;
    mStatus.mState = LooperState::StartSet;
    mStatus.mCurrentLoopTimeMS = 0;
    mLayers[0].StartRecording(mStatus, liveVoice, Ptr(LOOPSTATION_BUFFER), Ptr(EndPtr(LOOPSTATION_BUFFER)));
  }

  // UI actions.
  void LoopIt(const MusicalVoice& mv)
  {
    // basically this is a one-button loop function.
    // if the loop beginning is not set, begin measuring loop length.
    // if loop length is not set, set the length and commit to the next layer.
    // if the loop length is set, just commit to next layer.
    switch (mStatus.mState) {
    case LooperState::Idle:
      switch (mAppSettings->mLooperSettings.mTrigger) {
        case LooperTrigger::Immediate:
          TriggerSetStart(mv);
          break;
        default:
        case LooperTrigger::NoteOn:
        case LooperTrigger::NoteOff:
          mArmed = true; // triggering happens on update.
          break;
        case LooperTrigger::Beat1:
          mBeatTimeToStartRecording = ceilf(mMetronome->GetBeatFloat() + 1);
          mArmed = true;
          break;
        case LooperTrigger::Beat2:
          mBeatTimeToStartRecording = ceilf(mMetronome->GetBeatFloat() + 2);
          mArmed = true;
          break;
        case LooperTrigger::Beat4:
          mBeatTimeToStartRecording = ceilf(mMetronome->GetBeatFloat() + 4);
          mArmed = true;
          break;
        case LooperTrigger::Beat8:
          mBeatTimeToStartRecording = ceilf(mMetronome->GetBeatFloat() + 8);
          mArmed = true;
          break;
      }
      break;
    case LooperState::StartSet:
      // set loop duration, set up next loop layer.
      if (mStatus.mCurrentLoopTimeMS < LOOP_MIN_DURATION) {
        break;
      }
      UpdateCurrentLoopTimeMS();
      mStatus.mLoopDurationMS = mStatus.mCurrentLoopTimeMS;
      mStatus.mState = LooperState::DurationSet;
      // FALL-THROUGH
    case LooperState::DurationSet:
      // tell the currently-writing layer it's over.
      UpdateCurrentLoopTimeMS();

      // figure out what the new layer will be.
      LoopEventStream* newLayer = nullptr;
      size_t iNewLayer = 0;
      for (size_t inew = 0; inew < SizeofStaticArray(mLayers); ++ inew)
      {
        if (mCurrentlyWritingLayer == inew)
          continue;
        auto& l = mLayers[inew];
        if (l.IsInUse()) { 
          continue;
        }
        // found it.
        iNewLayer = inew;
        newLayer = &l;
        break;
      }

      if (newLayer) { // without a layer to move to, we can't do anything. you need a "live" layer always.

        // wrap up existing recording
        Ptr buf = mLayers[mCurrentlyWritingLayer].WrapUpRecording();
        mLayers[mCurrentlyWritingLayer].mIsPlaying = true;

        // and start with the new layer.
        mCurrentlyWritingLayer = (uint8_t)iNewLayer;
        newLayer->StartRecording(mStatus, mv, buf, Ptr(EndPtr(LOOPSTATION_BUFFER)));
      }

      break;
    } // switch
  }

  // see notes https://github.com/thenfour/Clarinoid/issues/25
  // blitting all subsequent layers to reduce fragmentation would be a lot of overhead when it's very
  // unlikely that we will OOM. specific strategies to tighten space can be employed (first, when you wrap up recording, THEN move the recorded layer to the biggest free block)
  // but for now until that's an actual problem, just eat up more buffer.
  void ClearLayer(size_t layer)
  {
    CCASSERT(layer < SizeofStaticArray(mLayers));
    if (layer == mCurrentlyWritingLayer)
    {
      // you can't "clear" the currently writing layer. this is the "live" layer where we're
      // not actually playing the recorded material, so there would be no user-facing effect.
      return;
    }
    auto& l = mLayers[layer];
    l.Stop(); // this will mark the layer free for use (after it plays its note-off) and its memory will be frozen and forgotten.
  }

  void Clear()
  {
    mArmed = false;
    mCurrentlyWritingLayer = 0;
    mStatus.mState = LooperState::Idle;
    //mLoopTimer.Restart();
    for (auto& l : mLayers) {
      l.Stop();
    }
  }

  void UpdateCurrentLoopTimeMS() {
    uint32_t ret = (uint32_t)(mLoopTimer.ElapsedMicros() / 1000); // theoretical error for very long loop times.
    if (mStatus.mState == LooperState::DurationSet) {
      // handle wrapping.
      ret %= mStatus.mLoopDurationMS;
    }
    mStatus.mCurrentLoopTimeMS = ret;
  }

  size_t Update(const MusicalVoice& liveVoice, const MusicalVoiceTransitionEvents& transitionEvents, MusicalVoice* outp, MusicalVoice* outpEnd) {
    /*
    1. record state
    2. for each layer, each harmonizer layer, if it can already be filled in, do it.
    3. feed scale follower with the current stuff AND ALL "live" notes.
    4. repeat step 2 for remaining voices.

      live notes are important @ #3, because you can be playing:
      - a comping layer of just Eb, with harm voices of -3 and -5 intervals
      - static bass line of C.
      the bass line is concrete; will automatically be in context.
      the comping layer will be delayed because it's dynamic
      the performer expects that Eb is considered here, and the scale is deduced as C minor.
      without live notes considered, it would probably guess C major becasue it doesn't know
      about Eb.

    - synth engine should not read anything that could be written here
    - synth engine should read all musical state from voices, not from ewimusicalstate.

    */

    if (mArmed) {
      switch (mAppSettings->mLooperSettings.mTrigger) {
        default:
        case LooperTrigger::Immediate:
          CCASSERT(false);
          break;
        case LooperTrigger::NoteOn:
          if (transitionEvents.mNeedsNoteOn) {
            TriggerSetStart(liveVoice);
          }
          break;
        case LooperTrigger::NoteOff:
          if (transitionEvents.mNeedsNoteOff) {
            TriggerSetStart(liveVoice);
          }
          break;
        case LooperTrigger::Beat1:
        case LooperTrigger::Beat2:
        case LooperTrigger::Beat4:
        case LooperTrigger::Beat8:
          // ask metronome if we crossed a beat, countdown.
          if (mMetronome->GetBeatFloat() >= mBeatTimeToStartRecording) {
            TriggerSetStart(liveVoice);
          }
          break;
      }
    }

    // Calculate the loop time for this event. Don't repeat millis() calls, and ensure the time doesn't change through this processing.
    UpdateCurrentLoopTimeMS();

    // in order to feed the scale follower with a single buffer for both "live" (may not be played!) and harmonized voices,
    // go through all looper layers & harmonizer voices, and if we know the note then put it. if there are 0 notes output,
    // then just put the "live" note but mark it as muted.

    MusicalVoice* pout = outp;
    MusicalVoice* pLiveVoices[LOOP_LAYERS] = {0}; // keep track of where i read layer state into, for later when deduced voices are filled in.

    // do the same for other layers; they're read from stream.
    for (uint8_t iLayer = 0; iLayer < SizeofStaticArray(mLayers); ++iLayer) {
      auto& l = mLayers[iLayer];
      bool layerIsUsed = false;
      if (iLayer == mCurrentlyWritingLayer) {
        // LIVE voice.
        layerIsUsed = true;
        *pout = liveVoice; // copy it straight from live
        if (mStatus.mState != LooperState::Idle) {
          l.Write(liveVoice); // record it if that's what we're doing.
        }
      }
      else
      {
        // recorded layer voice.
        // consume events sequentially. even if the layer is not playing, it may send a note off. "use" the layer in this case.
        // here's a dilemma: imagine a note has a release envelope after a note-off. In otherd words we cannot tell from note offs whether
        // a voice should be active or not. in that case, the synth should really be responsible for "holding" these voices on.
        // here we just want to track whether a voice is ON or has events.
        layerIsUsed = l.ReadUntilLoopTime(*pout);
      }

      if (layerIsUsed) {
        pLiveVoices[iLayer] = pout;// save this musicalvoice for later harmonizing (key=layer!)
        pout += mHarmonizer.Harmonize(iLayer, pout, transitionEvents, pout + 1, outpEnd, Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices);
      }
    }

    //auto old = gAppSettings.mDeducedScale;
    mAppSettings->mDeducedScale = mScaleFollower->Update(outp, pout - outp);
    // if (gAppSettings.mDeducedScale != old) {
    //   Serial.println(String("scale changed to ") + gAppSettings.mDeducedScale.ToString());
    // }

    // go through and fill in all deduced voices. logically, the "live voices" will all remain untouched so voices just get filled in.
    //int deduced = 0;
    for (uint8_t iLayer = 0; iLayer < SizeofStaticArray(mLayers); ++iLayer) {
      auto& l = mLayers[iLayer];
      if (pLiveVoices[iLayer] && (l.mIsPlaying || (iLayer == mCurrentlyWritingLayer))) {
        //deduced ++;
        pout += mHarmonizer.Harmonize(iLayer, pLiveVoices[iLayer], transitionEvents, pout, outpEnd, Harmonizer::VoiceFilterOptions::OnlyDeducedVoices);
      }
    }

    //CCPlot(String("musical poly: ") + int(pout - outp) + ", deduced layers touched: " + deduced);

    return pout - outp;
  }
};

} // namespace clarinoid
