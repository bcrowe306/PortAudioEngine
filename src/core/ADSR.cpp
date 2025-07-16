#include "ADSR.h"
#include <algorithm>
#include <cmath>

ADSR::ADSR(const std::string& name)
    : name_(name)
    , sampleRate_(44100.0)
    , attackTime_(0.01)      // 10ms default attack
    , decayTime_(0.1)        // 100ms default decay
    , sustainLevel_(0.7)     // 70% default sustain
    , releaseTime_(0.3)      // 300ms default release
    , curve_(1.0)            // Slight exponential curve
    , currentStage_(Stage::IDLE)
    , currentValue_(0.0)
    , targetValue_(0.0)
    , increment_(0.0)
    , samplesRemaining_(0)
    , releaseStartValue_(0.0)
{
    Logger::debug("ADSR '{}' created with default parameters", name_);
}

void ADSR::setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    Logger::debug("ADSR '{}': Sample rate set to {:.1f} Hz", name_, sampleRate_);
}

void ADSR::setAttackTime(double attackTime) {
    attackTime_ = std::max(0.001, attackTime);  // Minimum 1ms
    Logger::debug("ADSR '{}': Attack time set to {:.3f}s", name_, attackTime_);
}

void ADSR::setDecayTime(double decayTime) {
    decayTime_ = std::max(0.001, decayTime);    // Minimum 1ms
    Logger::debug("ADSR '{}': Decay time set to {:.3f}s", name_, decayTime_);
}

void ADSR::setSustainLevel(double sustainLevel) {
    sustainLevel_ = std::clamp(sustainLevel, 0.0, 1.0);
    Logger::debug("ADSR '{}': Sustain level set to {:.3f}", name_, sustainLevel_);
}

void ADSR::setReleaseTime(double releaseTime) {
    releaseTime_ = std::max(0.001, releaseTime); // Minimum 1ms
    Logger::debug("ADSR '{}': Release time set to {:.3f}s", name_, releaseTime_);
}

void ADSR::setParameters(double attackTime, double decayTime, double sustainLevel, double releaseTime) {
    setAttackTime(attackTime);
    setDecayTime(decayTime);
    setSustainLevel(sustainLevel);
    setReleaseTime(releaseTime);
}

void ADSR::setCurve(double curve) {
    curve_ = std::max(0.0, curve);
    Logger::debug("ADSR '{}': Curve set to {:.3f}", name_, curve_);
}

void ADSR::trigger() {
    currentStage_ = Stage::ATTACK;
    targetValue_ = 1.0;
    setupCurrentStage();
    Logger::debug("ADSR '{}': Triggered (Attack phase)", name_);
}

void ADSR::release() {
    if (currentStage_ != Stage::IDLE && currentStage_ != Stage::RELEASE) {
        currentStage_ = Stage::RELEASE;
        releaseStartValue_ = currentValue_;
        targetValue_ = 0.0;
        setupCurrentStage();
        Logger::debug("ADSR '{}': Released from value {:.3f}", name_, releaseStartValue_);
    }
}

void ADSR::reset() {
    currentStage_ = Stage::IDLE;
    currentValue_ = 0.0;
    targetValue_ = 0.0;
    increment_ = 0.0;
    samplesRemaining_ = 0;
    releaseStartValue_ = 0.0;
    Logger::debug("ADSR '{}': Reset to idle", name_);
}

double ADSR::processSample() {
    if (currentStage_ == Stage::IDLE) {
        return 0.0;
    }
    
    // Process current stage
    if (samplesRemaining_ > 0) {
        if (curve_ <= 0.0) {
            // Linear interpolation
            currentValue_ += increment_;
        } else {
            // Exponential interpolation
            int totalSamples = static_cast<int>(getCurrentStageDuration() * sampleRate_);
            double progress = static_cast<double>(totalSamples - samplesRemaining_) / totalSamples;
            
            // Clamp progress to prevent numerical issues
            progress = std::clamp(progress, 0.0, 1.0);
            
            double curvedProgress = applyCurve(progress);
            
            double startValue = getCurrentStageStartValue();
            double endValue = targetValue_;
            
            // Ensure smooth interpolation
            currentValue_ = startValue + curvedProgress * (endValue - startValue);
        }
        
        samplesRemaining_--;
        
        // Check if we've reached the target
        if (samplesRemaining_ <= 0) {
            // Ensure exact target value to prevent discontinuities
            currentValue_ = targetValue_;
            advanceToNextStage();
        }
    } else if (currentStage_ == Stage::SUSTAIN) {
        currentValue_ = sustainLevel_;
    }
    
    // Apply gentle clamping to prevent pops (only if values are slightly out of range)
    if (currentValue_ < -0.001 || currentValue_ > 1.001) {
        currentValue_ = std::clamp(currentValue_, 0.0, 1.0);
    }
    
    return currentValue_;
}

void ADSR::processBlock(double* output, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        output[i] = processSample();
    }
}

bool ADSR::isActive() const {
    return currentStage_ != Stage::IDLE;
}

bool ADSR::isReleasing() const {
    return currentStage_ == Stage::RELEASE;
}

bool ADSR::isFinished() const {
    return currentStage_ == Stage::IDLE || 
           (currentStage_ == Stage::RELEASE && currentValue_ <= FINISHED_THRESHOLD);
}

void ADSR::printInfo() const {
    Logger::info("=== ADSR '{}' Info ===", name_);
    Logger::info("Sample Rate: {:.1f} Hz", sampleRate_);
    Logger::info("Attack: {:.3f}s, Decay: {:.3f}s, Sustain: {:.3f}, Release: {:.3f}s", 
                attackTime_, decayTime_, sustainLevel_, releaseTime_);
    Logger::info("Curve: {:.3f}", curve_);
    Logger::info("Current Stage: {}, Value: {:.3f}", 
                static_cast<int>(currentStage_), currentValue_);
    Logger::info("Active: {}, Releasing: {}, Finished: {}", 
                isActive(), isReleasing(), isFinished());
    Logger::info("========================");
}

// Private methods

double ADSR::calculateLinearIncrement(double startValue, double endValue, double timeSeconds) {
    if (timeSeconds <= 0.0 || sampleRate_ <= 0.0) {
        return 0.0;
    }
    
    int totalSamples = static_cast<int>(timeSeconds * sampleRate_);
    if (totalSamples <= 0) {
        return 0.0;
    }
    
    return (endValue - startValue) / totalSamples;
}

void ADSR::calculateExponentialParameters(double startValue, double endValue, double timeSeconds) {
    if (timeSeconds <= 0.0 || sampleRate_ <= 0.0) {
        samplesRemaining_ = 0;
        return;
    }
    
    samplesRemaining_ = static_cast<int>(timeSeconds * sampleRate_);
    targetValue_ = endValue;
    
    // For exponential curves, we don't use increment_
    // Instead, we calculate the curved progress in processSample()
    increment_ = 0.0;
}

double ADSR::applyCurve(double linearValue) const {
    if (curve_ <= 0.0) {
        return linearValue;
    }
    
    // Clamp input to valid range to prevent numerical issues
    linearValue = std::clamp(linearValue, 0.0, 1.0);
    
    // Apply exponential curve with proper direction for different stages
    if (currentStage_ == Stage::ATTACK) {
        // For attack: concave curve (fast start, slow finish)
        // Use power curve: x^(1/curve) for attack to get natural exponential feel
        if (curve_ == 1.0) {
            return linearValue * linearValue;
        } else {
            return std::pow(linearValue, 1.0 / curve_);
        }
    } else {
        // For decay/release: convex curve (slow start, fast finish)
        // Use inverted power curve for more natural decay
        if (curve_ == 1.0) {
            return 1.0 - std::pow(1.0 - linearValue, 2.0);
        } else {
            return 1.0 - std::pow(1.0 - linearValue, curve_);
        }
    }
}

void ADSR::advanceToNextStage() {
    switch (currentStage_) {
        case Stage::ATTACK:
            currentStage_ = Stage::DECAY;
            // Ensure we start decay from exactly 1.0 to prevent discontinuity
            currentValue_ = 1.0;
            targetValue_ = sustainLevel_;
            setupCurrentStage();
            break;
            
        case Stage::DECAY:
            currentStage_ = Stage::SUSTAIN;
            // Ensure we enter sustain at exactly the sustain level
            currentValue_ = sustainLevel_;
            break;
            
        case Stage::SUSTAIN:
            // Stay in sustain until released
            break;
            
        case Stage::RELEASE:
            currentStage_ = Stage::IDLE;
            currentValue_ = 0.0;
            break;
            
        case Stage::IDLE:
            // Already idle
            break;
    }
}

void ADSR::setupCurrentStage() {
    int totalSamples = static_cast<int>(getCurrentStageDuration() * sampleRate_);
    samplesRemaining_ = std::max(1, totalSamples); // Ensure at least 1 sample
    
    switch (currentStage_) {
        case Stage::ATTACK:
            // Attack goes from current value (should be 0) to 1.0
            targetValue_ = 1.0;
            if (curve_ <= 0.0) {
                increment_ = calculateLinearIncrement(currentValue_, 1.0, attackTime_);
            } else {
                increment_ = 0.0; // Will use exponential calculation
            }
            break;
            
        case Stage::DECAY:
            // Decay goes from current value (should be 1.0) to sustain level
            targetValue_ = sustainLevel_;
            if (curve_ <= 0.0) {
                increment_ = calculateLinearIncrement(currentValue_, sustainLevel_, decayTime_);
            } else {
                increment_ = 0.0; // Will use exponential calculation
            }
            break;
            
        case Stage::RELEASE:
            // Release goes from current value to 0.0
            releaseStartValue_ = currentValue_; // Store where we started the release
            targetValue_ = 0.0;
            if (curve_ <= 0.0) {
                increment_ = calculateLinearIncrement(currentValue_, 0.0, releaseTime_);
            } else {
                increment_ = 0.0; // Will use exponential calculation
            }
            break;
            
        case Stage::SUSTAIN:
        case Stage::IDLE:
            increment_ = 0.0;
            samplesRemaining_ = 0;
            break;
    }
}

double ADSR::getCurrentStageDuration() const {
    switch (currentStage_) {
        case Stage::ATTACK:  return attackTime_;
        case Stage::DECAY:   return decayTime_;
        case Stage::RELEASE: return releaseTime_;
        case Stage::SUSTAIN:
        case Stage::IDLE:
        default:             return 0.0;
    }
}

double ADSR::getCurrentStageStartValue() const {
    switch (currentStage_) {
        case Stage::ATTACK:  return 0.0;           // Attack starts from 0
        case Stage::DECAY:   return 1.0;           // Decay starts from peak (1.0)
        case Stage::RELEASE: return releaseStartValue_; // Release starts from wherever we were
        case Stage::SUSTAIN: return sustainLevel_; // Sustain holds at sustain level
        case Stage::IDLE:
        default:             return 0.0;
    }
}
