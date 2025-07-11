#include "LevelsNode.h"
#include <cmath>
#include <algorithm>

LevelsNode::LevelsNode(const std::string& name) : AudioNode(name) {
    // Initialize RMS buffers to zero
    rmsBufferLeft.fill(0.0f);
    rmsBufferRight.fill(0.0f);
}

void LevelsNode::processCallback(
    const float* const* inputBuffers,
    float* const* outputBuffers,
    int numInputChannels,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize
) {
    // Pass through audio (this is an analyzer, not an effect)
    if (numInputChannels >= 1 && numOutputChannels >= 1) {
        copyBuffer(inputBuffers[0], outputBuffers[0], numSamples);
    }
    if (numInputChannels >= 2 && numOutputChannels >= 2) {
        copyBuffer(inputBuffers[1], outputBuffers[1], numSamples);
    }
    
    // Clear any additional output channels
    for (int ch = numInputChannels; ch < numOutputChannels; ++ch) {
        clearBuffer(outputBuffers[ch], numSamples);
    }
    
    // Analyze levels
    for (int i = 0; i < numSamples; ++i) {
        float leftSample = (numInputChannels >= 1) ? inputBuffers[0][i] : 0.0f;
        float rightSample = (numInputChannels >= 2) ? inputBuffers[1][i] : leftSample;
        
        updatePeaks(leftSample, rightSample);
        updateRMS(leftSample, rightSample);
    }
}

void LevelsNode::updatePeaks(float leftSample, float rightSample) {
    float leftAbs = std::abs(leftSample);
    float rightAbs = std::abs(rightSample);
    
    // Update peak hold with decay
    peakHoldLeft = std::max(leftAbs, peakHoldLeft * PEAK_DECAY_RATE);
    peakHoldRight = std::max(rightAbs, peakHoldRight * PEAK_DECAY_RATE);
    
    // Store in atomic variables for thread-safe reading
    atomicPeakLeft.store(peakHoldLeft, std::memory_order_relaxed);
    atomicPeakRight.store(peakHoldRight, std::memory_order_relaxed);
}

void LevelsNode::updateRMS(float leftSample, float rightSample) {
    // Remove old sample from sliding window sum
    float oldLeftSample = rmsBufferLeft[rmsWriteIndex];
    float oldRightSample = rmsBufferRight[rmsWriteIndex];
    
    rmsSumLeft -= oldLeftSample * oldLeftSample;
    rmsSumRight -= oldRightSample * oldRightSample;
    
    // Add new sample to sliding window
    rmsBufferLeft[rmsWriteIndex] = leftSample;
    rmsBufferRight[rmsWriteIndex] = rightSample;
    
    rmsSumLeft += leftSample * leftSample;
    rmsSumRight += rightSample * rightSample;
    
    // Calculate RMS
    float rmsLeft = std::sqrt(rmsSumLeft / RMS_WINDOW_SIZE);
    float rmsRight = std::sqrt(rmsSumRight / RMS_WINDOW_SIZE);
    
    // Store in atomic variables for thread-safe reading
    atomicRmsLeft.store(rmsLeft, std::memory_order_relaxed);
    atomicRmsRight.store(rmsRight, std::memory_order_relaxed);
    
    // Advance write index
    rmsWriteIndex = (rmsWriteIndex + 1) % RMS_WINDOW_SIZE;
}

LevelsNode::LevelData LevelsNode::getCurrentLevels() {
    LevelData data;
    data.peakLeft = atomicPeakLeft.load(std::memory_order_relaxed);
    data.peakRight = atomicPeakRight.load(std::memory_order_relaxed);
    data.rmsLeft = atomicRmsLeft.load(std::memory_order_relaxed);
    data.rmsRight = atomicRmsRight.load(std::memory_order_relaxed);
    return data;
}

void LevelsNode::resetPeakHold() {
    peakHoldLeft = 0.0f;
    peakHoldRight = 0.0f;
    atomicPeakLeft.store(0.0f, std::memory_order_relaxed);
    atomicPeakRight.store(0.0f, std::memory_order_relaxed);
}
