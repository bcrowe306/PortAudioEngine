#include "FilterNode.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>

FilterNode::FilterNode(const std::string& name, FilterType type, Rolloff rolloff)
    : AudioNode(name), filterType(type), rolloff(rolloff)
{
    // Initialize frequency parameter (20Hz - 20kHz, default 1kHz)
    frequencyParam = std::make_unique<AudioParameter>(
        name + "_Frequency",
        1000.0f,        // 1kHz default
        MIN_FREQUENCY,  // 20Hz minimum
        MAX_FREQUENCY,  // 20kHz maximum
        20.0f           // 20ms smoothing for frequency sweeps
    );
    
    // Initialize resonance parameter (0.1 - 30.0, default 0.707 for Butterworth response)
    resonanceParam = std::make_unique<AudioParameter>(
        name + "_Resonance",
        0.707f,         // Default Q for flat response
        MIN_RESONANCE,  // 0.1 minimum
        MAX_RESONANCE,  // 30.0 maximum (allows self-oscillation)
        10.0f           // 10ms smoothing
    );

    // Set up exponential mapping for frequency parameter (feels more natural)
    frequencyParam->setValueMapping([](float normalizedValue) {
        // Exponential curve: 20Hz to 20kHz
        return MIN_FREQUENCY * std::pow(MAX_FREQUENCY / MIN_FREQUENCY, normalizedValue);
    });

    Logger::info("FilterNode '", name, "' created: type=", static_cast<int>(type), 
                 " rolloff=", static_cast<int>(rolloff), " freq=1kHz Q=0.707");
}

void FilterNode::prepare(const PrepareInfo& info) {
    AudioNode::prepare(info);
    
    sampleRate = info.sampleRate;
    numActiveChannels = std::min(info.numChannels, MAX_CHANNELS);
    
    // Configure parameters with sample rate
    frequencyParam->setSampleRate(sampleRate);
    resonanceParam->setSampleRate(sampleRate);
    
    // Reset all filter states
    resetFilterState();
    
    Logger::debug("FilterNode '", getName(), "' prepared for sampleRate=", sampleRate, 
                  " channels=", numActiveChannels);
}

void FilterNode::processCallback(
    const float* const* inputBuffers,
    float* const* outputBuffers,
    int numInputChannels,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize)
{
    if (isBypassed()) {
        // Copy input to output when bypassed
        for (int channel = 0; channel < std::min(numInputChannels, numOutputChannels); ++channel) {
            copyBuffer(inputBuffers[channel], outputBuffers[channel], numSamples);
        }
        return;
    }

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample) {
        // Get current parameter values (with smoothing)
        float currentFreq = frequencyParam->getNextValue();
        float currentResonance = resonanceParam->getNextValue();
        
        // Calculate filter coefficients
        float normalizedFreq, processedResonance;
        calculateCoefficients(currentFreq, currentResonance, normalizedFreq, processedResonance);
        
        // Process each channel
        for (int channel = 0; channel < numOutputChannels; ++channel) {
            float inputSample = (channel < numInputChannels) ? inputBuffers[channel][sample] : 0.0f;
            float outputSample = 0.0f;
            
            if (rolloff == Rolloff::Slope12dB) {
                // 12dB per octave filtering
                outputSample = processSVFSample(inputSample, filterStates12dB[channel], 
                                              normalizedFreq, processedResonance);
            } else {
                // 24dB per octave filtering (cascaded stages)
                outputSample = process24dBSample(inputSample, 
                                               filterStates24dB_1[channel], 
                                               filterStates24dB_2[channel],
                                               normalizedFreq, processedResonance);
            }
            
            outputBuffers[channel][sample] = outputSample;
        }
    }
}

void FilterNode::setFilter(float frequencyHz, float resonance) {
    frequencyParam->setValue(frequencyHz);
    resonanceParam->setValue(resonance);
    
    Logger::debug("FilterNode '", getName(), "' filter set: freq=", frequencyHz, "Hz Q=", resonance);
}

void FilterNode::setLowPassFilter(float frequencyHz, float resonance) {
    setFilterType(FilterType::LowPass);
    setFilter(frequencyHz, resonance);
}

void FilterNode::setHighPassFilter(float frequencyHz, float resonance) {
    setFilterType(FilterType::HighPass);
    setFilter(frequencyHz, resonance);
}

void FilterNode::setBandPassFilter(float centerHz, float resonance) {
    setFilterType(FilterType::BandPass);
    setFilter(centerHz, resonance);
}

void FilterNode::setNotchFilter(float centerHz, float resonance) {
    setFilterType(FilterType::Notch);
    setFilter(centerHz, resonance);
}

void FilterNode::resetFilterState() {
    for (int channel = 0; channel < MAX_CHANNELS; ++channel) {
        filterStates12dB[channel].reset();
        filterStates24dB_1[channel].reset();
        filterStates24dB_2[channel].reset();
    }
    
    Logger::debug("FilterNode '", getName(), "' filter state reset");
}

float FilterNode::getFrequencyResponse(float frequency) const {
    // Simplified frequency response calculation for visualization
    // This is an approximation for UI display purposes
    
    float currentFreq = frequencyParam->getCurrentValue();
    float currentQ = resonanceParam->getCurrentValue();
    
    float ratio = frequency / currentFreq;
    float logRatio = std::log2(ratio);
    
    float response = 0.0f; // dB
    
    switch (filterType) {
        case FilterType::LowPass:
            if (ratio > 1.0f) {
                response = -6.0f * logRatio; // -6dB per octave base
                if (rolloff == Rolloff::Slope24dB) {
                    response *= 2.0f; // -12dB per octave for 24dB filter
                }
            }
            break;
            
        case FilterType::HighPass:
            if (ratio < 1.0f) {
                response = 6.0f * logRatio; // +6dB per octave base
                if (rolloff == Rolloff::Slope24dB) {
                    response *= 2.0f; // +12dB per octave for 24dB filter
                }
            }
            break;
            
        case FilterType::BandPass:
            // Simplified band-pass response
            if (std::abs(logRatio) < 1.0f) {
                response = -3.0f * logRatio * logRatio; // Peak at center
            } else {
                response = -6.0f * std::abs(logRatio);
            }
            break;
            
        case FilterType::Notch:
            // Simplified notch response
            if (std::abs(logRatio) < 0.1f) {
                response = -40.0f; // Deep notch at center
            }
            break;
    }
    
    // Add resonance boost at cutoff frequency
    if (std::abs(ratio - 1.0f) < 0.1f && currentQ > 1.0f) {
        response += 20.0f * std::log10(currentQ);
    }
    
    return response;
}

float FilterNode::processSVFSample(float input, SVFState& state, float frequency, float resonance) {
    // State Variable Filter implementation
    // Based on Hal Chamberlin's design from "Musical Applications of Microprocessors"
    
    // Calculate intermediate values
    float feedback = resonance + resonance / (1.0f - frequency);
    
    // Apply feedback from delay states
    state.highpass = input - state.delay1 * feedback - state.delay2;
    
    // First integrator
    state.bandpass = state.delay1 + state.highpass * frequency;
    state.delay1 = state.bandpass;
    
    // Second integrator  
    state.lowpass = state.delay2 + state.bandpass * frequency;
    state.delay2 = state.lowpass;
    
    // Notch = input - bandpass
    state.notch = input - state.bandpass;
    
    // Clamp states to prevent instability
    state.delay1 = std::clamp(state.delay1, -4.0f, 4.0f);
    state.delay2 = std::clamp(state.delay2, -4.0f, 4.0f);
    
    // Return appropriate output based on filter type
    switch (filterType) {
        case FilterType::LowPass:
            return state.lowpass;
        case FilterType::HighPass:
            return state.highpass;
        case FilterType::BandPass:
            return state.bandpass;
        case FilterType::Notch:
            return state.notch;
        default:
            return state.lowpass;
    }
}

float FilterNode::process24dBSample(float input, SVFState& stage1, SVFState& stage2, 
                                   float frequency, float resonance) {
    // 24dB filter: cascade two 12dB stages
    // Reduce resonance for each stage to maintain overall character
    float stageResonance = resonance * 0.5f;
    
    // First stage
    float stage1Output = processSVFSample(input, stage1, frequency, stageResonance);
    
    // Second stage (processing output of first stage)
    FilterType originalType = filterType; // Save current type
    float stage2Output = processSVFSample(stage1Output, stage2, frequency, stageResonance);
    
    return stage2Output;
}

void FilterNode::calculateCoefficients(float frequencyHz, float resonance, 
                                      float& outFreq, float& outResonance) {
    // Clamp frequency to valid range
    frequencyHz = std::clamp(frequencyHz, MIN_FREQUENCY, MAX_FREQUENCY);
    
    // Calculate normalized frequency (0 to 1, where 1 = Nyquist)
    float nyquist = static_cast<float>(sampleRate * 0.5);
    outFreq = frequencyHz / nyquist;
    
    // Clamp to prevent aliasing and instability
    outFreq = std::clamp(outFreq, 0.001f, 0.99f);
    
    // Convert Q to internal resonance parameter
    // Higher Q = more resonance, adjusted for the SVF topology
    resonance = std::clamp(resonance, MIN_RESONANCE, MAX_RESONANCE);
    outResonance = 1.0f - (1.0f / resonance);
    outResonance = std::clamp(outResonance, 0.0f, 0.995f); // Prevent complete feedback
    
    // Additional frequency warping for better analog modeling at high frequencies
    if (outFreq > 0.1f) {
        // Pre-warp frequency for better high-frequency response
        outFreq = std::tan(M_PI * outFreq) / M_PI;
        outFreq = std::clamp(outFreq, 0.001f, 0.99f);
    }
}
