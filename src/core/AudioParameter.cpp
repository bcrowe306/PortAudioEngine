#include "AudioParameter.h"
#include <algorithm>
#include <cmath>
#include "Logger.h"

AudioParameter::AudioParameter(const std::string& name,
                               float initialValue,
                               float minValue,
                               float maxValue,
                               float smoothingTimeMs)
    : name(name)
    , minValue(minValue)
    , maxValue(maxValue)
    , smoothingTimeMs(smoothingTimeMs)
    , currentValue(constrainValue(initialValue))
    , targetValue(currentValue)
{
    updateRampParameters();
   
}

void AudioParameter::setValue(float value) {
    setValue(value, smoothingTimeMs);
}

void AudioParameter::setValue(float value, float rampTimeMs) {
    float constrainedValue = constrainValue(value);
    targetValue.store(constrainedValue);
    
    // Calculate ramp parameters
    float timeDelta = rampTimeMs / 1000.0f; // Convert to seconds
    totalRampSamples = static_cast<int>(timeDelta * sampleRate);
    samplesRemaining = totalRampSamples;
    
    if (totalRampSamples > 0) {
        float valueDelta = constrainedValue - currentValue;
        
        if (rampType == RampType::Linear) {
            rampIncrement = valueDelta / totalRampSamples;
        } else { // Exponential
            // For exponential ramping, we'll use a coefficient approach
            if (std::abs(valueDelta) > 1e-6f) {
                rampIncrement = valueDelta / totalRampSamples; // Simplified for now
            } else {
                rampIncrement = 0.0f;
            }
        }
    } else {
        // Immediate change if ramp time is 0
        currentValue = constrainedValue;
        samplesRemaining = 0;
    }
    
}

void AudioParameter::setValueImmediate(float value) {
    float constrainedValue = constrainValue(value);
    currentValue = constrainedValue;
    targetValue.store(constrainedValue);
    samplesRemaining = 0;
    
    Logger::debug("AudioParameter '{}' setValueImmediate: {}", name, constrainedValue);
}

float AudioParameter::getNextValue() {
    if (samplesRemaining > 0) {
        // We're in the middle of a ramp
        if (rampType == RampType::Linear) {
            currentValue += rampIncrement;
        } else { // Exponential
            // Simple exponential approach - can be improved
            float progress = 1.0f - (static_cast<float>(samplesRemaining) / totalRampSamples);
            float exponentialProgress = 1.0f - std::exp(-5.0f * progress); // Exponential curve
            currentValue = currentValue + (targetValue.load() - currentValue) * (rampIncrement / (targetValue.load() - currentValue)) * exponentialProgress;
        }
        
        samplesRemaining--;
        
        // Ensure we hit the target exactly on the last sample
        if (samplesRemaining == 0) {
            currentValue = targetValue.load();
        }
    }
    
    // Apply value mapping if set
    return mapValue(currentValue);
}

void AudioParameter::setSampleRate(double newSampleRate) {
    sampleRate = newSampleRate;
    updateRampParameters();
    Logger::debug("AudioParameter '{}' sample rate set to: {}", name, sampleRate);
}

void AudioParameter::setSmoothingTime(float timeMs) {
    smoothingTimeMs = timeMs;
    updateRampParameters();
    Logger::debug("AudioParameter '{}' smoothing time set to: {}ms", name, timeMs);
}

void AudioParameter::setRange(float minVal, float maxVal) {
    if (minVal <= maxVal) {
        minValue = minVal;
        maxValue = maxVal;
        
        // Ensure current and target values are within new range
        currentValue = constrainValue(currentValue);
        targetValue.store(constrainValue(targetValue.load()));
        
        Logger::debug("AudioParameter '{}' range set to: [{},{}]", name, minValue, maxValue);
    } else {
        Logger::warn("AudioParameter '{}' invalid range: min={} max={}", name, minVal, maxVal);
    }
}

void AudioParameter::setNormalizedValue(float normalizedValue) {
    float clampedNormalized = std::clamp(normalizedValue, 0.0f, 1.0f);
    float actualValue = minValue + clampedNormalized * (maxValue - minValue);
    setValue(actualValue);
}

float AudioParameter::getNormalizedValue() const {
    if (std::abs(maxValue - minValue) < 1e-6f) {
        return 0.0f; // Avoid division by zero
    }
    return (currentValue - minValue) / (maxValue - minValue);
}

void AudioParameter::updateRampParameters() {
    // This can be called when sample rate or smoothing time changes
    // Current implementation recalculates on each setValue call
}

float AudioParameter::constrainValue(float value) const {
    return std::clamp(value, minValue, maxValue);
}

float AudioParameter::mapValue(float value) const {
    if (valueMapper) {
        return valueMapper(value);
    }
    return value;
}

// ===== AudioParameterGroup Implementation =====

void AudioParameterGroup::addParameter(const std::string& name, AudioParameter* parameter) {
    if (parameter) {
        parameters.emplace_back(name, parameter);
        Logger::debug("AudioParameterGroup '{}' added parameter: {}", groupName, name);
    }
}

AudioParameter* AudioParameterGroup::getParameter(const std::string& name) {
    for (auto& [paramName, param] : parameters) {
        if (paramName == name) {
            return param;
        }
    }
    Logger::warn("AudioParameterGroup '{}' parameter not found: {}", groupName, name);
    return nullptr;
}

void AudioParameterGroup::setSampleRate(double sampleRate) {
    for (auto& [name, param] : parameters) {
        if (param) {
            param->setSampleRate(sampleRate);
        }
    }
    Logger::debug("AudioParameterGroup '{}' sample rate set to: {}", groupName, sampleRate);
}

void AudioParameterGroup::setAllSmoothingTime(float timeMs) {
    for (auto& [name, param] : parameters) {
        if (param) {
            param->setSmoothingTime(timeMs);
        }
    }
    Logger::debug("AudioParameterGroup '{}' smoothing time set to: {}ms", groupName, timeMs);
}
