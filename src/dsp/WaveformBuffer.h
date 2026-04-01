#pragma once

#include <atomic>
#include <cstring>

namespace cart {

class WaveformBuffer
{
public:
    static constexpr int kSize = 2048;

    void write (float sample)
    {
        int wp = writePos.load (std::memory_order_relaxed);
        buffer[wp] = sample;
        writePos.store ((wp + 1) % kSize, std::memory_order_release);
    }

    // Read the most recent `count` samples into dest (UI thread)
    void read (float* dest, int count) const
    {
        int wp = writePos.load (std::memory_order_acquire);
        int start = (wp - count + kSize) % kSize;
        for (int i = 0; i < count; ++i)
            dest[i] = buffer[(start + i) % kSize];
    }

    void reset()
    {
        std::memset (buffer, 0, sizeof (buffer));
        writePos.store (0, std::memory_order_relaxed);
    }

private:
    float buffer[kSize] = {};
    std::atomic<int> writePos { 0 };
};

} // namespace cart
