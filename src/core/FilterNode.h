#pragma once

#include "AudioNode.h"
#include "AudioParameter.h"
#include <memory>
#include <array>

/**
 * FilterNode - A versatile audio filter with multiple filter types and rolloffs
 * 
 * This node implements classic analog-modeled filters with configurable parameters
 * for creative and corrective audio processing.
 * 
 * Features:
 * - Multiple filter types: Low-pass, High-pass, Band-pass, Notch
 * - Two rolloff slopes: 12dB/octave (2-pole) and 24dB/octave (4-pole)
 * - Real-time parameter control via AudioParameters
 * - Resonance control with self-oscillation capability
 * - Sample-accurate parameter smoothing
 * - Analog-modeled behavior using state variable topology
 * - Real-time safe processing
 */
class FilterNode : public AudioNode {
public:
    enum class FilterType {
        LowPass,    // Low-pass filter - passes frequencies below cutoff
        HighPass,   // High-pass filter - passes frequencies above cutoff
        BandPass,   // Band-pass filter - passes frequencies around cutoff
        Notch       // Notch filter - rejects frequencies around cutoff
    };

    enum class Rolloff {
        Slope12dB,  // 12dB per octave (2-pole filter)
        Slope24dB   // 24dB per octave (4-pole filter, cascaded 2-pole)
    };

    /**
     * Constructor
     * @param name Node name for debugging
     * @param type Initial filter type
     * @param rolloff Initial rolloff slope
     */
    explicit FilterNode(const std::string& name = "FilterNode", 
                       FilterType type = FilterType::LowPass,
                       Rolloff rolloff = Rolloff::Slope12dB);
    virtual ~FilterNode() = default;

    // AudioNode interface
    void prepare(const PrepareInfo& info) override;
    void processCallback(
        const float* const* inputBuffers,
        float* const* outputBuffers,
        int numInputChannels,
        int numOutputChannels,
        int numSamples,
        double sampleRate,
        int blockSize
    ) override;

    // ===== FILTER CONFIGURATION =====
    
    /**
     * Set filter type
     */
    void setFilterType(FilterType type) { filterType = type; }
    FilterType getFilterType() const { return filterType; }
    
    /**
     * Set rolloff slope
     */
    void setRolloff(Rolloff rolloff) { this->rolloff = rolloff; }
    Rolloff getRolloff() const { return rolloff; }

    // ===== PARAMETER ACCESS =====
    
    /**
     * Get filter parameters for external control
     */
    AudioParameter& getFrequencyParameter() { return *frequencyParam; }
    AudioParameter& getResonanceParameter() { return *resonanceParam; }
    
    const AudioParameter& getFrequencyParameter() const { return *frequencyParam; }
    const AudioParameter& getResonanceParameter() const { return *resonanceParam; }

    // ===== CONVENIENCE SETTERS =====
    
    /**
     * Set filter frequency in Hz
     */
    void setFrequency(float frequencyHz) { frequencyParam->setValue(frequencyHz); }
    
    /**
     * Set filter resonance (Q factor)
     * Range: 0.1 - 30.0 (higher values = more resonance)
     */
    void setResonance(float resonance) { resonanceParam->setValue(resonance); }
    
    /**
     * Set both frequency and resonance at once
     */
    void setFilter(float frequencyHz, float resonance);
    
    /**
     * Set up common filter presets
     */
    void setLowPassFilter(float frequencyHz, float resonance = 0.707f);
    void setHighPassFilter(float frequencyHz, float resonance = 0.707f);
    void setBandPassFilter(float centerHz, float resonance = 2.0f);
    void setNotchFilter(float centerHz, float resonance = 5.0f);

    // ===== UTILITY METHODS =====
    
    /**
     * Reset filter state (clear delay lines)
     */
    void resetFilterState();
    
    /**
     * Get filter response at given frequency (for visualization)
     * @param frequency Frequency in Hz
     * @return Magnitude response in dB
     */
    float getFrequencyResponse(float frequency) const;

private:
    // ===== FILTER STRUCTURES =====
    
    /**
     * State Variable Filter structure for one channel
     * Based on analog filter topology for warm, musical sound
     */
    struct SVFState {
        float lowpass = 0.0f;   // Low-pass output state
        float bandpass = 0.0f;  // Band-pass output state
        float highpass = 0.0f;  // High-pass output state
        float notch = 0.0f;     // Notch output state
        
        // Internal delay states
        float delay1 = 0.0f;    // First integrator delay
        float delay2 = 0.0f;    // Second integrator delay
        
        void reset() {
            lowpass = bandpass = highpass = notch = 0.0f;
            delay1 = delay2 = 0.0f;
        }
    };

    // ===== FILTER PROCESSING =====
    
    /**
     * Process one sample through the state variable filter
     * @param input Input sample
     * @param state Filter state for this channel
     * @param frequency Normalized frequency (0-1)
     * @param resonance Resonance amount
     * @return Filtered output sample
     */
    float processSVFSample(float input, SVFState& state, float frequency, float resonance);
    
    /**
     * Process 24dB filter (cascaded 12dB stages)
     * @param input Input sample
     * @param stage1 First filter stage
     * @param stage2 Second filter stage
     * @param frequency Normalized frequency
     * @param resonance Resonance amount
     * @return Filtered output sample
     */
    float process24dBSample(float input, SVFState& stage1, SVFState& stage2, 
                           float frequency, float resonance);
    
    /**
     * Calculate filter coefficients from frequency and resonance
     * @param frequencyHz Frequency in Hz
     * @param resonance Q factor
     * @param outFreq Normalized frequency output (0-1)
     * @param outResonance Processed resonance output
     */
    void calculateCoefficients(float frequencyHz, float resonance, 
                              float& outFreq, float& outResonance);

    // ===== PARAMETERS =====
    std::unique_ptr<AudioParameter> frequencyParam;  // Filter frequency in Hz
    std::unique_ptr<AudioParameter> resonanceParam;  // Filter resonance (Q factor)

    // ===== FILTER CONFIGURATION =====
    FilterType filterType = FilterType::LowPass;
    Rolloff rolloff = Rolloff::Slope12dB;

    // ===== FILTER STATE (per channel) =====
    static constexpr int MAX_CHANNELS = 8;
    std::array<SVFState, MAX_CHANNELS> filterStates12dB;   // 12dB filter states
    std::array<SVFState, MAX_CHANNELS> filterStates24dB_1; // 24dB first stage
    std::array<SVFState, MAX_CHANNELS> filterStates24dB_2; // 24dB second stage

    // ===== AUDIO STATE =====
    double sampleRate = 44100.0;
    int numActiveChannels = 0;
    
    // ===== CONSTANTS =====
    static constexpr float MIN_FREQUENCY = 20.0f;     // 20 Hz minimum
    static constexpr float MAX_FREQUENCY = 20000.0f;  // 20 kHz maximum
    static constexpr float MIN_RESONANCE = 0.1f;      // Minimum Q
    static constexpr float MAX_RESONANCE = 30.0f;     // Maximum Q (allows self-oscillation)
};
