#pragma once

// represents a signal curve over time, for RGB colors
struct IAnimation
{
    virtual Color IAnimation_GetColor() = 0;
};

struct OffAnimation : IAnimation
{
    Color IAnimation_GetColor() override
    {
        return {0, 0, 0};
    }
};

OffAnimation gOffAnimation;

struct SolidColor : IAnimation
{
    Color mColor;
    Color IAnimation_GetColor() override
    {
        return mColor;
    }
};

struct IAnimationTarget
{
    virtual void IAnimationTarget_SetAnimation(IAnimation *anim) = 0;
};

struct PulseAnimation : IAnimation
{
    clarinoid::Stopwatch mT;

    PulseAnimation() = default;
    PulseAnimation(int durationMS) : mDurationMS(durationMS)
    {
    }

    int mDurationMS = 600;
    Color mForeColor = Colors::White;
    Color mBackColor = Colors::Black;

    void Trigger()
    {
        mT.Restart();
    }

    virtual Color IAnimation_GetColor()
    {
        int elapsedMS = mT.ElapsedTime().ElapsedMillisI();
        if (elapsedMS > mDurationMS)
            return mBackColor;
        if (elapsedMS < 0)
            return mBackColor; // can this happen? don't think so.
        float x = elapsedMS;
        x /= mDurationMS;
        auto ret = Color::Mix(mForeColor, mBackColor, x);
        // Serial.println(String("R=") + ret.R + ", level01=" + ret.GetLevel01() + ", levelA=" + ret.GetLevel() + "
        // (float(x)/MaxValue)=" + (float(ret.R) / Color::MaxValue));
        return ret;
    }
};
