#pragma once

#include "Logger.h"
#include <algorithm>
#include <cmath>

/**
 * @brief A flexible ADSR (Attack, Decay, Sustain, Release) envelope generator
 * 
 * This class provides a general-purpose ADSR envelope that can be used for:
 * - Amplitude envelopes (gain modulation)
 * - Filter envelopes (cutoff frequency modulation)
 * - Pitch envelopes (frequency modulation)
 * - Any other parameter that needs envelope control
 * 
 * The envelope outputs values in the range [0.0, 1.0] which can then be
 * scaled and applied to the target parameter.
 */
class ADSR {
public:
    enum class Stage {
        IDLE,       // Not triggered, output is 0
        ATTACK,     // Rising from 0 to peak
        DECAY,      // Falling from peak to sustain level
        SUSTAIN,    // Holding at sustain level
        RELEASE     // Falling from current level to 0
    };

    /**
     * @brief Construct a new ADSR envelope
     * @param name Optional name for debugging/logging
     */
    explicit ADSR(const std::string& name = "ADSR");

    /**
     * @brief Initialize the ADSR with sample rate
     * @param sampleRate The audio sample rate
     */
    void setSampleRate(double sampleRate);

    /**
     * @brief Set attack time in seconds
     * @param attackTime Time to reach peak from 0 (seconds)
     */
    void setAttackTime(double attackTime);

    /**
     * @brief Set decay time in seconds
     * @param decayTime Time to reach sustain level from peak (seconds)
     */
    void setDecayTime(double decayTime);

    /**
     * @brief Set sustain level (0.0 to 1.0)
     * @param sustainLevel The level to hold during sustain phase
     */
    void setSustainLevel(double sustainLevel);

    /**
     * @brief Set release time in seconds
     * @param releaseTime Time to reach 0 from current level (seconds)
     */
    void setReleaseTime(double releaseTime);

    /**
     * @brief Set all ADSR parameters at once
     * @param attackTime Attack time in seconds
     * @param decayTime Decay time in seconds
     * @param sustainLevel Sustain level (0.0 to 1.0)
     * @param releaseTime Release time in seconds
     */
    void setParameters(double attackTime, double decayTime, double sustainLevel, double releaseTime);

    /**
     * @brief Trigger the envelope (start attack phase)
     */
    void trigger();

    /**
     * @brief Release the envelope (start release phase)
     */
    void release();

    /**
     * @brief Reset the envelope to idle state
     */
    void reset();

    /**
     * @brief Process one sample and return the envelope value
     * @return Envelope value in range [0.0, 1.0]
     */
    double processSample();

    /**
     * @brief Process a block of samples
     * @param output Pointer to output buffer
     * @param numSamples Number of samples to process
     */
    void processBlock(double* output, int numSamples);

    /**
     * @brief Check if the envelope is active (not idle and not finished)
     * @return True if envelope is active
     */
    bool isActive() const;

    /**
     * @brief Check if the envelope is in release phase
     * @return True if in release phase
     */
    bool isReleasing() const;

    /**
     * @brief Check if the envelope has finished (reached idle after release)
     * @return True if envelope is finished
     */
    bool isFinished() const;

    /**
     * @brief Get the current envelope value without processing
     * @return Current envelope value in range [0.0, 1.0]
     */
    double getCurrentValue() const { return currentValue_; }

    /**
     * @brief Get the current stage
     * @return Current ADSR stage
     */
    Stage getCurrentStage() const { return currentStage_; }

    /**
     * @brief Get attack time in seconds
     */
    double getAttackTime() const { return attackTime_; }

    /**
     * @brief Get decay time in seconds
     */
    double getDecayTime() const { return decayTime_; }

    /**
     * @brief Get sustain level
     */
    double getSustainLevel() const { return sustainLevel_; }

    /**
     * @brief Get release time in seconds
     */
    double getReleaseTime() const { return releaseTime_; }

    /**
     * @brief Set the curve shape for envelope segments (linear vs exponential)
     * @param curve Curve amount: 0.0 = linear, > 0.0 = exponential (typical: 1.0-4.0)
     */
    void setCurve(double curve);

    /**
     * @brief Print debug information about the ADSR
     */
    void printInfo() const;

private:
    std::string name_;
    double sampleRate_;
    
    // ADSR parameters
    double attackTime_;     // Attack time in seconds
    double decayTime_;      // Decay time in seconds
    double sustainLevel_;   // Sustain level (0.0 to 1.0)
    double releaseTime_;    // Release time in seconds
    double curve_;          // Curve shape (0.0 = linear, > 0.0 = exponential)
    
    // Internal state
    Stage currentStage_;
    double currentValue_;
    double targetValue_;
    double increment_;
    int samplesRemaining_;
    
    // Release state
    double releaseStartValue_;  // Value when release was triggered
    
    // Threshold for considering envelope "finished"
    static constexpr double FINISHED_THRESHOLD = 0.001;
    
    /**
     * @brief Calculate increment per sample for linear transitions
     * @param startValue Starting value
     * @param endValue Ending value
     * @param timeSeconds Transition time in seconds
     * @return Increment per sample
     */
    double calculateLinearIncrement(double startValue, double endValue, double timeSeconds);
    
    /**
     * @brief Calculate parameters for exponential transitions
     * @param startValue Starting value
     * @param endValue Ending value
     * @param timeSeconds Transition time in seconds
     */
    void calculateExponentialParameters(double startValue, double endValue, double timeSeconds);
    
    /**
     * @brief Apply curve shaping to a linear value
     * @param linearValue Linear value (0.0 to 1.0)
     * @return Curved value
     */
    double applyCurve(double linearValue) const;
    
    /**
     * @brief Transition to the next stage
     */
    void advanceToNextStage();
    
    /**
     * @brief Set up parameters for the current stage
     */
    void setupCurrentStage();
    
    /**
     * @brief Get the duration of the current stage in seconds
     */
    double getCurrentStageDuration() const;
    
    /**
     * @brief Get the start value for the current stage
     */
    double getCurrentStageStartValue() const;
};
