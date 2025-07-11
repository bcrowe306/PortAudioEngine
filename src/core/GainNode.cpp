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
    const float* const* inputBuffers,
    float* const* outputBuffers,
    int numInputChannels,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize
) {
    // Update parameter sample rate if needed
    gainParameter->setSampleRate(sampleRate);
    
    // Process each output channel
    for (int outCh = 0; outCh < numOutputChannels; ++outCh) {
        if (outCh < numInputChannels && inputBuffers[outCh]) {
            // Apply gain with per-sample smoothing
            for (int sample = 0; sample < numSamples; ++sample) {
                float currentGain = gainParameter->getNextValue();
                outputBuffers[outCh][sample] = inputBuffers[outCh][sample] * currentGain;
            }
        } else {
            // No input for this channel, but still advance the parameter
            for (int sample = 0; sample < numSamples; ++sample) {
                gainParameter->getNextValue();
            }
            clearBuffer(outputBuffers[outCh], numSamples);
        }
    }
}
