#pragma once

#include "AudioNode.h"
#include "AudioParameter.h"
#include <memory>

class GainNode : public AudioNode {
public:
    GainNode(float initialGain = 1.0f, const std::string& name = "GainNode");
    
    void processCallback(
        choc::buffer::ChannelArrayView<const float> inputBuffers,
        choc::buffer::ChannelArrayView<float> outputBuffers,
        double sampleRate,
        int blockSize
    ) override;
    
    // Parameter access methods
    void setGain(float newGain) { gainParameter->setValue(newGain); }
    void setGainSmooth(float newGain, float rampTimeMs) { gainParameter->setValue(newGain, rampTimeMs); }
    void setGainImmediate(float newGain) { gainParameter->setValueImmediate(newGain); }
    float getGain() const { return gainParameter->getCurrentValue(); }
    float getTargetGain() const { return gainParameter->getTargetValue(); }
    
    // Direct parameter access for advanced control
    AudioParameter* getGainParameter() { return gainParameter.get(); }

private:
    std::unique_ptr<AudioParameter> gainParameter;
};
