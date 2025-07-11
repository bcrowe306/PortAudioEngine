#pragma once

#include "AudioNode.h"
#include <atomic>
#include <cmath>
#include <functional>

struct SongPosition {
    double bpm = 120.0;
    int timeSignatureNumerator = 4;
    int timeSignatureDenominator = 4;
    
    // Position tracking
    int64_t songPositionInTicks = 0;
    int64_t songPositionInSamples = 0;
    double songPositionInBeats = 0.0;  // Musical beat time (e.g., 3.5 = 3 and 1/2 beats)
    
    // Current musical position
    int currentBeat = 1;        // Beat within the bar (1-based)
    int currentBar = 1;         // Current bar number (1-based)
    int currentSixteenth = 1;   // Current sixteenth note (1-based, 1-16 per bar in 4/4)
    
    // Helper constants
    static constexpr int TICKS_PER_QUARTER_NOTE = 960;  // Standard MIDI resolution
};

// Callback function types
using TickCallback = std::function<void(const SongPosition& position, int64_t tick)>;
using SampleCallback = std::function<void(const SongPosition& position, int64_t sample)>;
using BeatCallback = std::function<void(const SongPosition& position, int beat, int bar)>;
using BarCallback = std::function<void(const SongPosition& position, int bar)>;

class PlayheadNode : public AudioNode {
public:
    PlayheadNode();
    virtual ~PlayheadNode() = default;

    // AudioNode interface
    void prepare(const PrepareInfo& info) override;
    void processCallback(
        const float* const* inputBuffers,
        float* const* outputBuffers,
        int numInputChannels,
        int numOutputChannels,
        int numSamples,
        double sampleRate,
        int blockSize
    ) override;

    // Transport controls
    void play();
    void stop();
    void pause();
    bool isPlaying() const { return playing.load(); }
    bool isPaused() const { return paused.load(); }

    // Position control
    void jumpToPosition(int64_t ticks);
    void jumpToPosition(double beats);
    void jumpToPosition(int bar, int beat);
    void jumpToSample(int64_t samples);
    
    // Tempo and time signature
    void setBpm(double newBpm);
    void setTimeSignature(int numerator, int denominator);
    double getBpm() const { return songPosition.bpm; }
    std::pair<int, int> getTimeSignature() const { 
        return {songPosition.timeSignatureNumerator, songPosition.timeSignatureDenominator}; 
    }

    // Position queries
    SongPosition getCurrentPosition() const { return songPosition; }
    int64_t getCurrentTick() const { return songPosition.songPositionInTicks; }
    int64_t getCurrentSample() const { return songPosition.songPositionInSamples; }
    double getCurrentBeat() const { return songPosition.songPositionInBeats; }
    
    // Musical position queries
    int getCurrentBar() const { return songPosition.currentBar; }
    int getCurrentBeatInBar() const { return songPosition.currentBeat; }
    int getCurrentSixteenth() const { return songPosition.currentSixteenth; }
    
    // Callback registration
    void setTickCallback(TickCallback callback) { tickCallback = callback; }
    void setSampleCallback(SampleCallback callback) { sampleCallback = callback; }
    void setBeatCallback(BeatCallback callback) { beatCallback = callback; }
    void setBarCallback(BarCallback callback) { barCallback = callback; }
    
    // Clear callbacks
    void clearCallbacks() { 
        tickCallback = nullptr; 
        sampleCallback = nullptr; 
        beatCallback = nullptr; 
        barCallback = nullptr; 
    }

private:
    // Internal state
    SongPosition songPosition;
    std::atomic<bool> playing{false};
    std::atomic<bool> paused{false};
    
    // Cached calculations for performance
    double ticksPerSample = 0.0;
    double samplesPerTick = 0.0;
    double samplesPerBeat = 0.0;
    double ticksPerBeat = 0.0;
    double samplesPerBar = 0.0;
    
    // Internal methods
    void updateCachedValues();
    void updateMusicalPosition();
    void calculateMusicalPosition(int64_t ticks, int& bar, int& beat, int& sixteenth, double& beatTime);
    
    // Thread safety for position updates
    mutable std::atomic<bool> positionUpdateFlag{false};
    SongPosition pendingPosition;
    
    // Callback functions
    TickCallback tickCallback = nullptr;
    SampleCallback sampleCallback = nullptr;
    BeatCallback beatCallback = nullptr;
    BarCallback barCallback = nullptr;
    
    // Previous values for change detection
    int64_t lastTick = -1;
    int64_t lastSample = -1;
    int lastBeat = -1;
    int lastBar = -1;
    
    // Helper methods
    double ticksToBeats(int64_t ticks) const;
    int64_t beatsToTicks(double beats) const;
    int64_t samplesToTicks(int64_t samples) const;
    int64_t ticksToSamples(int64_t ticks) const;
};
