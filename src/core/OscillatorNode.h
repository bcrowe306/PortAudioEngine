#pragma once

#include "AudioNode.h"
#include "AudioParameter.h"
#include <memory>

// Simple oscillator node for testing
class OscillatorNode : public AudioNode {
public:
    enum class WaveType {
        Sine,
        Square,
        Sawtooth
    };

    OscillatorNode(float frequency = 440.0f, WaveType waveType = WaveType::Sine, const std::string& name = "OscillatorNode");
    
    void processCallback(
        choc::buffer::ChannelArrayView<const float> inputBuffers,
        choc::buffer::ChannelArrayView<float> outputBuffers,
        double sampleRate,
        int blockSize
    ) override;
    
    // Parameter access methods
    void setFrequency(float newFrequency) { frequencyParameter->setValue(newFrequency); }
    void setFrequencySmooth(float newFrequency, float rampTimeMs) { frequencyParameter->setValue(newFrequency, rampTimeMs); }
    void setFrequencyImmediate(float newFrequency) { frequencyParameter->setValueImmediate(newFrequency); }
    float getFrequency() const { return frequencyParameter->getCurrentValue(); }
    float getTargetFrequency() const { return frequencyParameter->getTargetValue(); }
    
    void setWaveType(WaveType newWaveType) { waveType = newWaveType; }
    WaveType getWaveType() const { return waveType; }
    
    // Direct parameter access for advanced control
    AudioParameter* getFrequencyParameter() { return frequencyParameter.get(); }

private:
    std::unique_ptr<AudioParameter> frequencyParameter;
    WaveType waveType;
    float phase = 0.0f;
    
    float generateSample(float frequency, float sampleRate);
};
