#pragma once

#include "AudioNode.h"
#include <atomic>
#include <array>

class LevelsNode : public AudioNode {
public:
    struct LevelData {
        float peakLeft = 0.0f;
        float peakRight = 0.0f;
        float rmsLeft = 0.0f;
        float rmsRight = 0.0f;
    };

    LevelsNode(const std::string& name = "LevelsNode");
    ~LevelsNode() override = default;

    void processCallback(
        choc::buffer::ChannelArrayView<const float> inputBuffers,
        choc::buffer::ChannelArrayView<float> outputBuffers,
        double sampleRate,
        int blockSize
    ) override;
    
    // Thread-safe level reading (call from UI thread)
    LevelData getCurrentLevels();
    
    // Reset peak hold
    void resetPeakHold();

private:
    // RMS calculation parameters
    static constexpr int RMS_WINDOW_SIZE = 4096;
    static constexpr float PEAK_DECAY_RATE = 0.999f;  // Per sample decay for peak hold
    
    // RMS sliding window buffers
    std::array<float, RMS_WINDOW_SIZE> rmsBufferLeft{};
    std::array<float, RMS_WINDOW_SIZE> rmsBufferRight{};
    int rmsWriteIndex = 0;
    float rmsSumLeft = 0.0f;
    float rmsSumRight = 0.0f;
    
    // Atomic level data for thread-safe reading
    std::atomic<float> atomicPeakLeft{0.0f};
    std::atomic<float> atomicPeakRight{0.0f};
    std::atomic<float> atomicRmsLeft{0.0f};
    std::atomic<float> atomicRmsRight{0.0f};
    
    // Peak hold values
    float peakHoldLeft = 0.0f;
    float peakHoldRight = 0.0f;
    
    void updateRMS(float leftSample, float rightSample);
    void updatePeaks(float leftSample, float rightSample);
};
