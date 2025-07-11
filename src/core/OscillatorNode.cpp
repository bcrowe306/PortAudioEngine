#include "OscillatorNode.h"
#include "Logger.h"
#include <cmath>

OscillatorNode::OscillatorNode(float frequency, WaveType waveType, const std::string& name) 
    : AudioNode(name)
    , frequencyParameter(std::make_unique<AudioParameter>(name + "_Frequency", frequency, 20.0f, 20000.0f, 100.0f))
    , waveType(waveType)
{
    Logger::debug("OscillatorNode '", name, "' created with frequency: ", frequency, "Hz");
}

void OscillatorNode::processCallback(
    const float* const* inputBuffers,
    float* const* outputBuffers,
    int numInputChannels,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize
) {
    // Update parameter sample rate if needed
    frequencyParameter->setSampleRate(sampleRate);
    
    // Generate oscillator output for each channel
    for (int outCh = 0; outCh < numOutputChannels; ++outCh) {
        for (int sample = 0; sample < numSamples; ++sample) {
            float currentFrequency = frequencyParameter->getNextValue();
            outputBuffers[outCh][sample] = generateSample(currentFrequency, static_cast<float>(sampleRate));
        }
    }
}

float OscillatorNode::generateSample(float frequency, float sampleRate) {
    float output = 0.0f;
    
    switch (waveType) {
        case WaveType::Sine:
            output = std::sin(2.0f * M_PI * phase);
            break;
            
        case WaveType::Square:
            output = (phase < 0.5f) ? 1.0f : -1.0f;
            break;
            
        case WaveType::Sawtooth:
            output = 2.0f * phase - 1.0f;
            break;
    }
    
    // Update phase
    phase += frequency / sampleRate;
    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
    
    return output * 0.8f; // Louder output for better audibility
}
