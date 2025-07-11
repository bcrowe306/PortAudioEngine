#pragma once

#include "AudioNode.h"
#include <vector>
#include <atomic>
#include <string>

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
    void loadData(const std::vector<float>& data);
    void play();
    void stop();
    void reset();
    
    bool isPlaying() const { return playing.load(); }
    bool isFinished() const { return playPosition.load() >= audioData.size(); }
    
    // Playback settings
    void setGain(float gain) { this->gain = gain; }
    void setLoop(bool loop) { this->loop = loop; }
    
    // Status
    size_t getPlayPosition() const { return playPosition.load(); }
    size_t getDataSize() const { return audioData.size(); }
    double getPlaybackProgress() const;

private:
    std::vector<float> audioData;
    std::atomic<size_t> playPosition{0};
    std::atomic<bool> playing{false};
    
    float gain = 1.0f;
    bool loop = false;
};
