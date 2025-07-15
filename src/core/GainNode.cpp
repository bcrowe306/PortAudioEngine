#include "GainNode.h"
#include "Logger.h"
#include <algorithm>

GainNode::GainNode(float initialGain, const std::string& name) 
    : AudioNode(name)
    , gainParameter(std::make_unique<AudioParameter>(name + "_Gain", initialGain, 0.0f, 4.0f, 20.0f))
{
    Logger::debug("GainNode '", name, "' created with initial gain: ", initialGain);
}

void GainNode::processCallback(
    choc::buffer::ChannelArrayView<const float> inputBuffers,
    choc::buffer::ChannelArrayView<float> outputBuffers,
    double sampleRate,
    int blockSize
) {
    // Update parameter sample rate if needed
    gainParameter->setSampleRate(sampleRate);
    
    auto numOutputChannels = outputBuffers.getNumChannels();
    auto numInputChannels = inputBuffers.getNumChannels();
    auto numSamples = outputBuffers.getNumFrames();
    
    // Process each output channel
    for (choc::buffer::ChannelCount outCh = 0; outCh < numOutputChannels; ++outCh) {
        if (outCh < numInputChannels) {
            // Apply gain with per-sample smoothing
            for (choc::buffer::FrameCount sample = 0; sample < numSamples; ++sample) {
                float currentGain = gainParameter->getNextValue();
                outputBuffers.getSample(outCh, sample) = inputBuffers.getSample(outCh, sample) * currentGain;
            }
        } else {
            // No input for this channel, but still advance the parameter
            for (choc::buffer::FrameCount sample = 0; sample < numSamples; ++sample) {
                gainParameter->getNextValue();
            }
            // Clear output for this channel
            for (choc::buffer::FrameCount sample = 0; sample < numSamples; ++sample) {
                outputBuffers.getSample(outCh, sample) = 0.0f;
            }
        }
    }
}
