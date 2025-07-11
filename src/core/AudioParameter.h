#pragma once

#include <atomic>
#include <string>
#include <functional>
#include <vector>

/**
 * AudioParameter - A class for handling audio parameters with smoothing and automation.
 * 
 * Features:
 * - Thread-safe parameter updates
 * - Automatic smoothing to prevent audio artifacts
 * - Min/Max value constraints
 * - Linear and exponential ramping
 * - Parameter automation support
 * - Real-time safe operations
 */
class AudioParameter {
public:
    enum class RampType {
        Linear,
        Exponential
    };

    /**
     * Constructor
     * @param name Parameter name for debugging
     * @param initialValue Starting value
     * @param minValue Minimum allowed value
     * @param maxValue Maximum allowed value
     * @param smoothingTimeMs Time in milliseconds for parameter changes to complete
     */
    AudioParameter(const std::string& name = "Parameter",
                   float initialValue = 0.0f,
                   float minValue = 0.0f,
                   float maxValue = 1.0f,
                   float smoothingTimeMs = 50.0f);

    ~AudioParameter() = default;

    // Non-copyable
    AudioParameter(const AudioParameter&) = delete;
    AudioParameter& operator=(const AudioParameter&) = delete;

    // ===== PARAMETER SETTING =====
    
    /**
     * Set the target value for the parameter
     * This is thread-safe and can be called from any thread
     */
    void setValue(float value);
    
    /**
     * Set the target value with custom ramp time
     */
    void setValue(float value, float rampTimeMs);
    
    /**
     * Set the value immediately without smoothing (use carefully!)
     */
    void setValueImmediate(float value);
    
    /**
     * Set the ramp type for future parameter changes
     */
    void setRampType(RampType type) { rampType = type; }

    // ===== PARAMETER GETTING =====
    
    /**
     * Get the current smoothed value (call this in audio callback)
     * This advances the internal smoothing and should be called once per audio block
     */
    float getNextValue();
    
    /**
     * Get the current value without advancing smoothing
     */
    float getCurrentValue() const { return currentValue; }
    
    /**
     * Get the target value
     */
    float getTargetValue() const { return targetValue.load(); }
    
    /**
     * Check if the parameter is currently ramping
     */
    bool isRamping() const { return samplesRemaining > 0; }

    // ===== CONFIGURATION =====
    
    /**
     * Set the sample rate (call this when sample rate changes)
     */
    void setSampleRate(double sampleRate);
    
    /**
     * Set the smoothing time in milliseconds
     */
    void setSmoothingTime(float timeMs);
    
    /**
     * Set the parameter range
     */
    void setRange(float minVal, float maxVal);
    
    /**
     * Set a custom value mapping function for exponential/logarithmic scaling
     */
    void setValueMapping(std::function<float(float)> mapper) { valueMapper = mapper; }

    // ===== ACCESSORS =====
    
    const std::string& getName() const { return name; }
    float getMinValue() const { return minValue; }
    float getMaxValue() const { return maxValue; }
    float getSmoothingTime() const { return smoothingTimeMs; }

    // ===== NORMALIZED VALUES (0.0 - 1.0) =====
    
    /**
     * Set parameter using normalized value (0.0 - 1.0)
     */
    void setNormalizedValue(float normalizedValue);
    
    /**
     * Get current value as normalized (0.0 - 1.0)
     */
    float getNormalizedValue() const;

private:
    void updateRampParameters();
    float constrainValue(float value) const;
    float mapValue(float value) const;

    // Parameter info
    std::string name;
    float minValue;
    float maxValue;
    
    // Current state
    float currentValue;
    std::atomic<float> targetValue;
    
    // Smoothing parameters
    float smoothingTimeMs;
    double sampleRate = 44100.0;
    RampType rampType = RampType::Linear;
    
    // Ramp state
    float rampIncrement;
    int samplesRemaining = 0;
    int totalRampSamples;
    
    // Advanced features
    std::function<float(float)> valueMapper;
};

/**
 * AudioParameterGroup - A collection of related parameters
 */
class AudioParameterGroup {
public:
    AudioParameterGroup(const std::string& name) : groupName(name) {}
    
    void addParameter(const std::string& name, AudioParameter* parameter);
    AudioParameter* getParameter(const std::string& name);
    
    void setSampleRate(double sampleRate);
    void setAllSmoothingTime(float timeMs);
    
    const std::string& getName() const { return groupName; }

private:
    std::string groupName;
    std::vector<std::pair<std::string, AudioParameter*>> parameters;
};
