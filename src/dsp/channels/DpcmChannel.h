#pragma once

#include <cstdint>
#include <vector>
#include "../ApuConstants.h"

namespace cart {

/// DPCM (Delta PCM) sample playback channel.
class DpcmChannel
{
public:
    void reset();
    void setSampleRate (double sr);

    /// Load a DPCM sample (raw bytes)
    void loadSample (const std::vector<uint8_t>& data);

    /// Set rate index (0–15)
    void setRateIndex (int index, bool ntsc = true);

    /// Set loop mode
    void setLoop (bool loop)    { loopEnabled = loop; }

    void setEnabled (bool en);

    /// Start playback
    void noteOn();
    void noteOff();

    float process();

    /// Current 7-bit output (0–127)
    uint8_t output() const      { return outputLevel; }

    bool isActive() const       { return playing; }

private:
    double   sampleRate    = 44100.0;
    double   phase         = 0.0;
    double   phaseInc      = 0.0;

    std::vector<uint8_t> sampleData;
    size_t   sampleIndex   = 0;
    int      bitsRemaining = 0;
    uint8_t  currentByte   = 0;
    uint8_t  outputLevel   = 64;      // 7-bit, starts at midpoint
    bool     playing       = false;
    bool     loopEnabled   = false;
    bool     enabled       = false;
};

} // namespace cart
