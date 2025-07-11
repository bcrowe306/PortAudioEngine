#include "ADSRNode.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>

ADSRNode::ADSRNode(const std::string& name)
    : AudioNode(name)
{
    // Initialize ADSR parameters with musical defaults
    attackParam = std::make_unique<AudioParameter>(
        name + "_Attack", 
        0.01f,      // 10ms default attack
        0.001f,     // 1ms minimum
        5.0f,       // 5 second maximum
        10.0f       // 10ms smoothing
    );
    
    decayParam = std::make_unique<AudioParameter>(
        name + "_Decay",
        0.1f,       // 100ms default decay
        0.001f,     // 1ms minimum
        5.0f,       // 5 second maximum
        10.0f       // 10ms smoothing
    );
    
    sustainParam = std::make_unique<AudioParameter>(
        name + "_Sustain",
        0.7f,       // 70% default sustain
        0.0f,       // 0% minimum
        1.0f,       // 100% maximum
        10.0f       // 10ms smoothing
    );
    
    releaseParam = std::make_unique<AudioParameter>(
        name + "_Release",
        0.3f,       // 300ms default release
        0.001f,     // 1ms minimum
        10.0f,      // 10 second maximum
        10.0f       // 10ms smoothing
    );

    Logger::info("ADSRNode '", name, "' created with default ADSR: A=10ms, D=100ms, S=70%, R=300ms");
}

void ADSRNode::prepare(const PrepareInfo& info) {
    AudioNode::prepare(info);
    
    sampleRate = info.sampleRate;
    
    // Configure parameters with sample rate
    attackParam->setSampleRate(sampleRate);
    decayParam->setSampleRate(sampleRate);
    sustainParam->setSampleRate(sampleRate);
    releaseParam->setSampleRate(sampleRate);
    
    // Reset envelope state
    reset();
    
    Logger::debug("ADSRNode '", getName(), "' prepared for sampleRate=", sampleRate, 
                  " maxBufferSize=", info.maxBufferSize);
}

void ADSRNode::processCallback(
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

    // Process gate events at the start of the block
    if (pendingNoteOn.load()) {
        bool retrigger = pendingRetrigger.load();
        pendingNoteOn.store(false);
        pendingRetrigger.store(false);
        
        if (retrigger || currentStage == EnvelopeStage::Idle) {
            transitionToStage(EnvelopeStage::Attack);
            gateOn.store(true);
        }
    }
    
    if (pendingNoteOff.load()) {
        pendingNoteOff.store(false);
        gateOn.store(false);
        
        if (currentStage != EnvelopeStage::Idle && currentStage != EnvelopeStage::Release) {
            transitionToStage(EnvelopeStage::Release);
        }
    }

    // Process audio samples
    for (int sample = 0; sample < numSamples; ++sample) {
        // Calculate envelope value for this sample
        float envelopeValue = calculateNextEnvelopeSample();
        
        // Apply envelope to all output channels
        for (int channel = 0; channel < numOutputChannels; ++channel) {
            if (channel < numInputChannels) {
                // Apply envelope to input signal
                outputBuffers[channel][sample] = inputBuffers[channel][sample] * envelopeValue;
            } else {
                // No input for this channel, output silence
                outputBuffers[channel][sample] = 0.0f;
            }
        }
    }
}

void ADSRNode::noteOn(bool retrigger) {
    pendingNoteOn.store(true);
    pendingRetrigger.store(retrigger);
    
    Logger::debug("ADSRNode '", getName(), "' noteOn (retrigger=", retrigger, ")");
}

void ADSRNode::noteOff() {
    pendingNoteOff.store(true);
    
    Logger::debug("ADSRNode '", getName(), "' noteOff");
}

void ADSRNode::reset() {
    currentStage = EnvelopeStage::Idle;
    currentLevel = 0.0f;
    targetLevel = 0.0f;
    stageIncrement = 0.0f;
    samplesInCurrentStage = 0;
    totalSamplesForCurrentStage = 0;
    
    // Clear any pending events
    gateOn.store(false);
    pendingNoteOn.store(false);
    pendingNoteOff.store(false);
    pendingRetrigger.store(false);
    
    Logger::debug("ADSRNode '", getName(), "' reset");
}

void ADSRNode::setADSR(float attack, float decay, float sustain, float release) {
    attackParam->setValue(attack);
    decayParam->setValue(decay);
    sustainParam->setValue(sustain);
    releaseParam->setValue(release);
    
    Logger::debug("ADSRNode '", getName(), "' ADSR set: A=", attack, "s, D=", decay, 
                  "s, S=", sustain, ", R=", release, "s");
}

float ADSRNode::calculateNextEnvelopeSample() {
    // Update parameter values (this handles smoothing)
    float attack = attackParam->getNextValue();
    float decay = decayParam->getNextValue();
    float sustain = sustainParam->getNextValue();
    float release = releaseParam->getNextValue();
    
    // Handle stage transitions
    bool stageComplete = false;
    
    if (totalSamplesForCurrentStage > 0) {
        samplesInCurrentStage++;
        stageComplete = (samplesInCurrentStage >= totalSamplesForCurrentStage);
    }
    
    // Process current stage
    switch (currentStage) {
        case EnvelopeStage::Idle:
            currentLevel = 0.0f;
            break;
            
        case EnvelopeStage::Attack:
            if (stageComplete) {
                // Attack complete, move to decay
                currentLevel = 1.0f;
                transitionToStage(EnvelopeStage::Decay);
            } else {
                // Continue attack
                currentLevel += stageIncrement;
                currentLevel = std::min(currentLevel, 1.0f);
            }
            break;
            
        case EnvelopeStage::Decay:
            if (stageComplete) {
                // Decay complete, move to sustain
                currentLevel = sustain;
                transitionToStage(EnvelopeStage::Sustain);
            } else {
                // Continue decay
                currentLevel += stageIncrement; // Will be negative
                currentLevel = std::max(currentLevel, sustain);
            }
            break;
            
        case EnvelopeStage::Sustain:
            // Hold at sustain level (may change via parameter automation)
            currentLevel = sustain;
            break;
            
        case EnvelopeStage::Release:
            if (stageComplete) {
                // Release complete, go to idle
                currentLevel = 0.0f;
                transitionToStage(EnvelopeStage::Idle);
            } else {
                // Continue release
                currentLevel += stageIncrement; // Will be negative
                currentLevel = std::max(currentLevel, 0.0f);
            }
            break;
    }
    
    // Apply curve shaping
    return applyCurve(currentLevel);
}

float ADSRNode::applyCurve(float linearValue) const {
    if (curveType == CurveType::Linear) {
        return linearValue;
    } else { // Exponential
        // Exponential curve: more natural sounding for most parameters
        // f(x) = x^2 for attack/decay, keeps 0 and 1 endpoints
        if (linearValue <= minimumLevel) {
            return 0.0f;
        }
        return linearValue * linearValue;
    }
}

void ADSRNode::transitionToStage(EnvelopeStage newStage) {
    EnvelopeStage previousStage = currentStage;
    currentStage = newStage;
    samplesInCurrentStage = 0;
    
    // Calculate stage parameters
    switch (newStage) {
        case EnvelopeStage::Idle:
            targetLevel = 0.0f;
            stageIncrement = 0.0f;
            totalSamplesForCurrentStage = 0;
            break;
            
        case EnvelopeStage::Attack: {
            targetLevel = 1.0f;
            float attackTime = attackParam->getCurrentValue();
            totalSamplesForCurrentStage = static_cast<int>(attackTime * sampleRate);
            if (totalSamplesForCurrentStage > 0) {
                stageIncrement = (targetLevel - currentLevel) / totalSamplesForCurrentStage;
            } else {
                stageIncrement = 0.0f;
                currentLevel = targetLevel;
            }
            break;
        }
        
        case EnvelopeStage::Decay: {
            targetLevel = sustainParam->getCurrentValue();
            float decayTime = decayParam->getCurrentValue();
            totalSamplesForCurrentStage = static_cast<int>(decayTime * sampleRate);
            if (totalSamplesForCurrentStage > 0) {
                stageIncrement = (targetLevel - currentLevel) / totalSamplesForCurrentStage;
            } else {
                stageIncrement = 0.0f;
                currentLevel = targetLevel;
            }
            break;
        }
        
        case EnvelopeStage::Sustain:
            targetLevel = sustainParam->getCurrentValue();
            stageIncrement = 0.0f;
            totalSamplesForCurrentStage = 0; // Indefinite
            currentLevel = targetLevel;
            break;
            
        case EnvelopeStage::Release: {
            targetLevel = 0.0f;
            float releaseTime = releaseParam->getCurrentValue();
            totalSamplesForCurrentStage = static_cast<int>(releaseTime * sampleRate);
            if (totalSamplesForCurrentStage > 0) {
                stageIncrement = (targetLevel - currentLevel) / totalSamplesForCurrentStage;
            } else {
                stageIncrement = 0.0f;
                currentLevel = targetLevel;
            }
            break;
        }
    }
    
    Logger::debug("ADSRNode '", getName(), "' stage transition: ", 
                  static_cast<int>(previousStage), " -> ", static_cast<int>(newStage),
                  " (currentLevel=", currentLevel, ", targetLevel=", targetLevel, 
                  ", samples=", totalSamplesForCurrentStage, ")");
}
