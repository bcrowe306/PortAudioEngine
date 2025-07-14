#pragma once

#include "AudioNode.h"
#include <cstddef>
#include <vector>
#include <atomic>
#include <string>
#include "../../lib/choc/audio/choc_SampleBuffers.h"

class AudioPlayer : public AudioNode {
public:
    AudioPlayer(const std::string& name = "AudioPlayer");
    ~AudioPlayer() = default;
    
    // AudioNode interface
    void processCallback(
        const float* const* inputBuffers,
        float* const* outputBuffers,
        int numInputChannels,
        int numOutputChannels,
        int numSamples,
        double sampleRate,
        int blockSize
    ) override;
    
    // Playback controls
    void loadData(const std::vector<float>& monoData);
    void loadData(const choc::buffer::ChannelArrayBuffer<float>& audioBuffer);
    void loadData(const float* const* channelData, int numChannels, int numFrames);
    void play();
    void stop();
    void reset();
    
    bool isPlaying() const { return playing.load(); }
    bool isFinished() const { return playPosition.load() >= getDataSize(); }
    
    // Playback settings
    void setGain(float gain) { this->gain = gain; }
    void setLoop(bool loop) { this->loop = loop; }
    
    // Status
    size_t getPlayPosition() const { return playPosition.load(); }
    size_t getDataSize() const { return audioData.getNumFrames(); }
    int getNumChannels() const { return static_cast<int>(audioData.getNumChannels()); }
    double getPlaybackProgress() const;

private:
    choc::buffer::ChannelArrayBuffer<float> audioData;
    std::atomic<size_t> playPosition{0};
    std::atomic<bool> playing{false};
    std::atomic<size_t> startPosition {0}; // Start position for playback
    std::atomic<size_t> endPosition {0}; // End position for playback
    std::atomic<bool> reverse{false}; // Reverse playback flag
    float gain = 1.0f;
    bool loop = false;
};
