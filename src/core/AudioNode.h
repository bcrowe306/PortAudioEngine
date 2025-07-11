#pragma once

#include <vector>
#include <memory>
#include <string>

class AudioNode : public std::enable_shared_from_this<AudioNode> {
public:
    struct PrepareInfo {
        double sampleRate;
        int maxBufferSize;
        int numChannels;
    };

    AudioNode(const std::string& name = "AudioNode");
    virtual ~AudioNode() = default;

    // Audio processing lifecycle
    virtual void prepare(const PrepareInfo& info);
    virtual void processCallback(
        const float* const* inputBuffers,
        float* const* outputBuffers,
        int numInputChannels,
        int numOutputChannels,
        int numSamples,
        double sampleRate,
        int blockSize
    ) = 0;

    // Node management
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    
    bool isPrepared() const { return prepared; }
    bool isBypassed() const { return bypassed; }
    void setBypassed(bool bypass) { bypassed = bypass; }

protected:
    // Helper methods for derived classes
    void copyBuffer(const float* source, float* destination, int numSamples);
    void addToBuffer(const float* source, float* destination, int numSamples);
    void clearBuffer(float* buffer, int numSamples);
    void scaleBuffer(float* buffer, float gain, int numSamples);

    // Internal state
    PrepareInfo currentPrepareInfo;
    bool prepared = false;
    bool bypassed = false;

private:
    std::string name;
};
