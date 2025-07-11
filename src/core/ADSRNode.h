#pragma once

#include "AudioNode.h"
#include "AudioParameter.h"
#include <memory>
#include <atomic>

/**
 * ADSRNode - An ADSR envelope generator that applies gain to audio signals
 * 
 * This node implements a classic Attack, Decay, Sustain, Release envelope
 * with configurable parameters for musical expression and dynamics.
 * 
 * Features:
 * - Real-time safe ADSR envelope generation
 * - Configurable Attack, Decay, Sustain, Release via AudioParameters
 * - Gate control for note on/off events
 * - Sample-accurate envelope timing
 * - Multiple envelope curves (linear, exponential)
 * - Retrigger support for overlapping notes
 */
class ADSRNode : public AudioNode {
public:
    enum class EnvelopeStage {
        Idle,       // No envelope active
        Attack,     // Rising from 0 to 1
        Decay,      // Falling from 1 to sustain level
        Sustain,    // Holding at sustain level
        Release     // Falling from current level to 0
    };

    enum class CurveType {
        Linear,
        Exponential
    };

    /**
     * Constructor
     * @param name Node name for debugging
     */
    explicit ADSRNode(const std::string& name = "ADSRNode");
    virtual ~ADSRNode() = default;

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

    // ===== ENVELOPE CONTROL =====
    
    /**
     * Trigger the envelope (note on)
     * @param retrigger If true, restart from attack even if already active
     */
    void noteOn(bool retrigger = false);
    
    /**
     * Release the envelope (note off)
     */
    void noteOff();
    
    /**
     * Reset the envelope to idle state
     */
    void reset();
    
    /**
     * Check if envelope is currently active (not idle)
     */
    bool isActive() const { return currentStage != EnvelopeStage::Idle; }
    
    /**
     * Get current envelope stage
     */
    EnvelopeStage getCurrentStage() const { return currentStage; }
    
    /**
     * Get current envelope value (0.0 - 1.0)
     */
    float getCurrentLevel() const { return currentLevel; }

    // ===== PARAMETER ACCESS =====
    
    /**
     * Get ADSR parameters for external control
     */
    AudioParameter& getAttackParameter() { return *attackParam; }
    AudioParameter& getDecayParameter() { return *decayParam; }
    AudioParameter& getSustainParameter() { return *sustainParam; }
    AudioParameter& getReleaseParameter() { return *releaseParam; }
    
    const AudioParameter& getAttackParameter() const { return *attackParam; }
    const AudioParameter& getDecayParameter() const { return *decayParam; }
    const AudioParameter& getSustainParameter() const { return *sustainParam; }
    const AudioParameter& getReleaseParameter() const { return *releaseParam; }

    // ===== CONFIGURATION =====
    
    /**
     * Set the curve type for all envelope stages
     */
    void setCurveType(CurveType type) { curveType = type; }
    CurveType getCurveType() const { return curveType; }
    
    /**
     * Set minimum level for exponential curves (prevents division by zero)
     */
    void setMinimumLevel(float level) { minimumLevel = std::max(level, 1e-6f); }
    float getMinimumLevel() const { return minimumLevel; }

    // ===== CONVENIENCE SETTERS =====
    
    /**
     * Set ADSR values directly (in seconds for A,D,R and 0-1 for S)
     */
    void setAttack(float timeSeconds) { attackParam->setValue(timeSeconds); }
    void setDecay(float timeSeconds) { decayParam->setValue(timeSeconds); }
    void setSustain(float level) { sustainParam->setValue(level); }
    void setRelease(float timeSeconds) { releaseParam->setValue(timeSeconds); }
    
    /**
     * Set all ADSR values at once
     */
    void setADSR(float attack, float decay, float sustain, float release);

private:
    // ===== ENVELOPE CALCULATION =====
    
    /**
     * Calculate next envelope sample
     */
    float calculateNextEnvelopeSample();
    
    /**
     * Apply curve shaping to linear value
     */
    float applyCurve(float linearValue) const;
    
    /**
     * Transition to next envelope stage
     */
    void transitionToStage(EnvelopeStage newStage);
    
    /**
     * Calculate increment for current stage
     */
    void updateStageIncrement();

    // ===== PARAMETERS =====
    std::unique_ptr<AudioParameter> attackParam;   // Attack time in seconds (0.001 - 5.0)
    std::unique_ptr<AudioParameter> decayParam;    // Decay time in seconds (0.001 - 5.0)
    std::unique_ptr<AudioParameter> sustainParam;  // Sustain level (0.0 - 1.0)
    std::unique_ptr<AudioParameter> releaseParam;  // Release time in seconds (0.001 - 10.0)

    // ===== ENVELOPE STATE =====
    EnvelopeStage currentStage = EnvelopeStage::Idle;
    float currentLevel = 0.0f;
    float targetLevel = 0.0f;
    float stageIncrement = 0.0f;
    int samplesInCurrentStage = 0;
    int totalSamplesForCurrentStage = 0;

    // ===== CONFIGURATION =====
    CurveType curveType = CurveType::Exponential;
    float minimumLevel = 1e-6f;  // Minimum level for exponential curves
    
    // ===== GATE STATE =====
    std::atomic<bool> gateOn{false};
    std::atomic<bool> pendingNoteOn{false};
    std::atomic<bool> pendingNoteOff{false};
    std::atomic<bool> pendingRetrigger{false};
    
    // ===== AUDIO STATE =====
    double sampleRate = 44100.0;
};
