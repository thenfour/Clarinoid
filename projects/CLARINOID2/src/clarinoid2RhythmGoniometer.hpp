#pragma once

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct NoteCountScroller
{
    Stopwatch _stopwatch;

    static constexpr int gMaxBuckets = 256; // maximum number of buckets to store
    std::array<int, gMaxBuckets> mBuckets;

    NoteCountScroller()
    {
        mBuckets.fill(0);
    }

    // We'll track how far time has advanced in terms of buckets
    int _lastClearedBucketIndex = 0;
    // This marks the oldest bucket index we have cleared so far.

    static constexpr int gBucketWidth = 2;     // horizontal px per bucket
    static constexpr int gMsPerBucket = 60;    // each bucket spans 100 ms
    static constexpr int gScrollerHeight = 30; // vertical px allocated for scroller
    static constexpr int gScreenWidth = 128;   // total screen width in px
    static constexpr int gScreenHeight = 55;   // total screen height in px

    /// Called when a note-on event occurs.
    /// Simply increment the current bucket's count.
    /// (We only do clearing in Render, so that even if no notes arrive,
    /// time-based clearing still happens.)
    void OnNoteOn()
    {
        int elapsedMs = (int)_stopwatch.ElapsedTime().ElapsedMillisI();
        int currentBucketIndex = elapsedMs / gMsPerBucket;

        // ring pos for the current bucket
        int ringPos = currentBucketIndex % gMaxBuckets;
        mBuckets[ringPos]++;
    }

    /// <summary>
    /// Renders the scroller along the bottom of the screen.
    /// The newest bucket is at the right edge, older to the left.
    /// Also clears any buckets that are older than the left edge.
    /// </summary>
    void Render(IDisplay *display)
    {
        int screenBottom = gScreenHeight - 1;

        int elapsedMs = (int)_stopwatch.ElapsedTime().ElapsedMillisI();
        int currentBucketIndex = elapsedMs / gMsPerBucket;

        // 1) Clear buckets older than the left edge
        // The leftmost bucket index:
        int leftmostBucketIndex = currentBucketIndex - (gScreenWidth / gBucketWidth);

        // Move _lastClearedBucketIndex forward until it matches leftmostBucketIndex
        // and set those old buckets to 0.
        while (_lastClearedBucketIndex < leftmostBucketIndex)
        {
            int ringPos = _lastClearedBucketIndex % gMaxBuckets;
            // clear that bucket
            mBuckets[ringPos] = 0;
            _lastClearedBucketIndex++;
        }

        // 2) Now draw the visible region
        // We'll iterate horizontally from x=0..ScreenWidth-1 in steps of BucketWidth
        for (int screenX = 0; screenX < gScreenWidth; screenX += gBucketWidth)
        {
            // distanceFromRight = how many px from the right edge
            int distanceFromRight = (gScreenWidth - 1) - screenX;
            // how many buckets to the left of current
            int bucketOffset = distanceFromRight / gBucketWidth;
            int bucketIndex = currentBucketIndex - bucketOffset;

            if (bucketIndex < 0)
            {
                // time before 0 => no data
                continue;
            }

            // ring pos
            int ringPos = bucketIndex % gMaxBuckets;
            if (ringPos < 0)
                ringPos += gMaxBuckets; // handle negative mod if needed

            // number of notes that happened in this bucket
            int noteCount = mBuckets[ringPos];

            // clamp noteCount so it won't exceed our scroller height
            // each note uses 2 px vertically (1 line + 1 gap),
            // but we only have 'ScrollerHeight' px total
            int maxLines = gScrollerHeight / 2;
            if (noteCount > maxLines)
                noteCount = maxLines;

            // draw each note as a horizontal line 1 px thick, with 1 px gap
            // from bottom to top
            for (int n = 0; n < noteCount; n++)
            {
                int lineY = screenBottom - (n * 2);
                // fill a horizontal line of width BucketWidth at lineY
                display->drawFastHLine(screenX, lineY, gBucketWidth, WHITE);
            }
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct RhythmGoniometerApp : DisplayApp
{
    int xUpdates = 0;

    MusicalStateTask &mMusicalStateTask;
    int lastKnownNoteOnSerial = 0;

    // NoteCountScroller mNoteCountScroller;

    static constexpr int gHeight = 44;
    static constexpr PointI gCenter{64, gHeight / 2};
    static constexpr RectI gClip{0, 0, 128, gHeight};
    static constexpr float gRadius = 26;

    static constexpr int gLineHoldMS = 1500;
    static constexpr int gLineDecayMS = 2500;
    static constexpr int gDotHoldMS = 100;
    static constexpr int gDotDecayMS = 600;

    struct NotePoint
    {
        PointI mPt;
        bool mActive = false;
        StopwatchLight mWhenAdded;
    };

    static constexpr int gMaxMarkers = 16;
    std::array<NotePoint, gMaxMarkers> mNotePoints;
    size_t mNotePointCursor = 0; // always point to the oldest entry.

    RhythmGoniometerApp(IDisplay &d, MusicalStateTask &musicalStateTask)
        : DisplayApp(d), mMusicalStateTask(musicalStateTask)
    {
    }

    virtual const char *DisplayAppGetName() override
    {
        return "rhythm gon";
    }

    virtual void UpdateApp() override
    {
        if (mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }

    void UpdateFrontPage()
    {
        xUpdates++;

        bool didNoteOn = false;
        if (mMusicalStateTask.mMusicalState.mMidiOut.noteOns != lastKnownNoteOnSerial)
        {
            lastKnownNoteOnSerial = mMusicalStateTask.mMusicalState.mMidiOut.noteOns;
            didNoteOn = true;
        }
        else
        {
            didNoteOn = false;
        }

        if (didNoteOn)
        {
            float beatFrac = mMusicalStateTask.mMetronome.GetBeatFrac();
            float currentBeatPos01 = beatFrac - 0.5f;
            auto pt = DeviationAngleToPoint(currentBeatPos01);
            AddNotePoint(pt);
            // mNoteCountScroller.OnNoteOn();
        }
    }

    void AddNotePoint(const PointI &np)
    {
        // notes are always added in order, so the list is always sorted by age from oldest to newest, as long as
        // you start from mNotePointCursor.

        mNotePoints[mNotePointCursor].mPt = np;
        mNotePoints[mNotePointCursor].mActive = true;
        mNotePoints[mNotePointCursor].mWhenAdded.Restart();

        mNotePointCursor = (mNotePointCursor + 1) % gMaxMarkers;
    }

    PointI DeviationAngleToPoint(float angle01)
    {
        // Convert angle to radians
        float angleRad = angle01 * gPI<float> * 2;

        // Calculate marker position on the circumference
        float markerX = gCenter.x + gRadius * fast::cos(angleRad);
        float markerY = gCenter.y + gRadius * fast::sin(angleRad);
        return {(int)markerX, (int)markerY};
    }

    int GetBrightness(int elapsedMS, int holdMs, int decayMs)
    {
        // no point using a floating point divide etc. operate on Q8.8
        int brightness = 256;
        if (elapsedMS > holdMs)
        {
            int fadeStage01 = ((elapsedMS - holdMs) << 8) / decayMs;
            brightness = 256 - fadeStage01;
        }
        return brightness; // if out of range, it gets clamped in the display device anyway.
    }

    virtual void RenderApp() override
    {
    }

    virtual void RenderFrontPage() override
    {
        UpdateFrontPage();
        // iterate through the whole array, starting from mNotePointCursor
        for (size_t __i = 1; __i < mNotePoints.size(); __i++)
        {
            size_t idx = (mNotePointCursor + __i) % mNotePoints.size();
            auto &a1 = mNotePoints[idx - 1];
            auto &a2 = mNotePoints[idx];
            if (!a1.mActive || !a2.mActive)
            {
                continue;
            }

            // todo: remove if faded
            int lineBrightness =
                GetBrightness((int)a2.mWhenAdded.ElapsedTime().ElapsedMillisI(), gLineHoldMS, gLineDecayMS);

            mDisplay.DrawInfiniteLineClipped(a1.mPt, a2.mPt, gClip, lineBrightness >> 2);

            mDisplay.DrawLineWithBrightness(a1.mPt, a2.mPt, lineBrightness);

            // mDisplay.DrawLine(a1.mPt, a2.mPt);

            mDisplay.FillCircleWithBrightness(
                a2.mPt, 4, GetBrightness((int)a2.mWhenAdded.ElapsedTime().ElapsedMillisI(), gDotHoldMS, gDotDecayMS));
        }

        // mNoteCountScroller.Render(&mDisplay);

        mDisplay.setCursor(0, 0);
        mDisplay.print(String(mMusicalStateTask.mMusicalState.mMidiOut.noteOns));
    }

    virtual void DisplayAppUpdate() override
    {
        DisplayApp::DisplayAppUpdate(); // update input
    }
};

} // namespace clarinoid
