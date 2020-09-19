

#pragma once

template <size_t N>
class SimpleMovingAverage
{
  public:
    void Update(float sample)
    {
        if (num_samples_ < N)
        {
            samples_[num_samples_++] = sample;
            total_ += sample;
        }
        else
        {
            float& oldest = samples_[num_samples_++ % N];
            total_ += sample - oldest;
            oldest = sample;
        }
    }

    float GetValue() const {
      return total_ / min(num_samples_, N);
    }
    size_t GetSampleCount() const { return num_samples_; }

    void Clear() {
      total_ = 0;
      num_samples_ = 0;
    }

  private:
    float samples_[N];
    size_t num_samples_ = 0;
    float total_ = 0;
};
