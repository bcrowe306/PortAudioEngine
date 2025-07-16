#pragma once

#include "../../lib/choc/audio/choc_MIDI.h"
#include "Logger.h"
#include "ADSR.h"
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <cstdint>

/**
 * Voice state information for the allocator
 */
struct Voice {
    int note = -1;                          // MIDI note number (-1 = inactive)
    int velocity = 0;                       // Note velocity (0-127)
    int channel = 0;                        // MIDI channel (0-15)
    bool isActive = false;                  // Voice is currently playing
    bool isSustained = false;               // Voice is held by sustain pedal
    bool isReleasing = false;               // Voice is in release phase
    std::chrono::steady_clock::time_point triggerTime;  // When voice was triggered
    std::chrono::steady_clock::time_point releaseTime;  // When voice was released
    
    // Voice ID for tracking (useful for complex voice management)
    uint32_t voiceId = 0;
    
    // ADSR envelope for amplitude modulation
    ADSR amplitudeEnvelope;
    
    // Additional envelope slots for future use (filter, pitch, etc.)
    std::unique_ptr<ADSR> filterEnvelope;   // Optional filter envelope
    std::unique_ptr<ADSR> pitchEnvelope;    // Optional pitch envelope
    
    // Constructor to initialize amplitude envelope with voice-specific name
    Voice(int voiceIndex = -1) 
        : amplitudeEnvelope("Voice" + std::to_string(voiceIndex) + "_Amp") {
    }
    
    void reset() {
        note = -1;
        velocity = 0;
        channel = 0;
        isActive = false;
        isSustained = false;
        isReleasing = false;
        voiceId = 0;
        
        // Reset envelopes
        amplitudeEnvelope.reset();
        if (filterEnvelope) {
            filterEnvelope->reset();
        }
        if (pitchEnvelope) {
            pitchEnvelope->reset();
        }
    }
    
    bool isInUse() const {
        return isActive || isSustained || isReleasing;
    }
    
    /**
     * Initialize envelope with sample rate
     */
    void initializeEnvelopes(double sampleRate) {
        amplitudeEnvelope.setSampleRate(sampleRate);
        if (filterEnvelope) {
            filterEnvelope->setSampleRate(sampleRate);
        }
        if (pitchEnvelope) {
            pitchEnvelope->setSampleRate(sampleRate);
        }
    }
    
    /**
     * Create and enable filter envelope
     */
    void enableFilterEnvelope(int voiceIndex = -1) {
        if (!filterEnvelope) {
            filterEnvelope = std::make_unique<ADSR>("Voice" + std::to_string(voiceIndex) + "_Filter");
        }
    }
    
    /**
     * Create and enable pitch envelope
     */
    void enablePitchEnvelope(int voiceIndex = -1) {
        if (!pitchEnvelope) {
            pitchEnvelope = std::make_unique<ADSR>("Voice" + std::to_string(voiceIndex) + "_Pitch");
        }
    }
};

/**
 * Voice allocation strategies
 */
enum class VoiceStealingMode {
    OLDEST,          // Steal the oldest voice
    LOWEST_VELOCITY, // Steal the voice with lowest velocity
    HIGHEST_NOTE,    // Steal the highest note
    LOWEST_NOTE      // Steal the lowest note
};

/**
 * VoiceAllocator - Manages polyphonic voice allocation for synthesizers
 * 
 * Features:
 * - Configurable maximum voice count (default 16)
 * - MIDI note on/off handling
 * - Sustain pedal support (CC64)
 * - Voice stealing with multiple strategies
 * - Voice state tracking and management
 * - Thread-safe operation
 */
class VoiceAllocator {
public:
    /**
     * Constructor
     * @param maxVoices Maximum number of simultaneous voices (default: 16)
     * @param stealingMode Voice stealing strategy (default: OLDEST)
     */
    explicit VoiceAllocator(int maxVoices = 16, VoiceStealingMode stealingMode = VoiceStealingMode::OLDEST);
    
    /**
     * Destructor
     */
    ~VoiceAllocator() = default;
    
    // =========================
    // MIDI Message Processing
    // =========================
    
    /**
     * Process a MIDI message and update voice allocation
     * @param message MIDI message to process
     * @return Voice index that was affected, or -1 if no voice was affected
     */
    int processMidiMessage(const choc::midi::ShortMessage& message);
    
    /**
     * Handle note on message
     * @param note MIDI note number (0-127)
     * @param velocity Note velocity (0-127)
     * @param channel MIDI channel (0-15)
     * @return Voice index allocated for this note, or -1 if failed
     */
    int noteOn(int note, int velocity, int channel = 0);
    
    /**
     * Handle note off message
     * @param note MIDI note number (0-127)
     * @param channel MIDI channel (0-15)
     * @return Voice index that was released, or -1 if note wasn't playing
     */
    int noteOff(int note, int channel = 0);
    
    /**
     * Handle sustain pedal (CC64)
     * @param value Sustain value (0-127, <64 = off, >=64 = on)
     * @param channel MIDI channel (0-15)
     */
    void setSustainPedal(int value, int channel = 0);
    
    /**
     * Handle all notes off (CC123)
     * @param channel MIDI channel (0-15)
     */
    void allNotesOff(int channel = 0);
    
    /**
     * Handle all sound off (CC120)
     * @param channel MIDI channel (0-15)
     */
    void allSoundOff(int channel = 0);
    
    // =========================
    // Voice State Management
    // =========================
    
    /**
     * Get voice information by index
     * @param voiceIndex Voice index (0 to maxVoices-1)
     * @return Reference to voice data
     */
    const Voice& getVoice(int voiceIndex) const;
    
    /**
     * Get mutable voice reference (for synth engines to update voice state)
     * @param voiceIndex Voice index (0 to maxVoices-1)
     * @return Mutable reference to voice data
     */
    Voice& getVoiceRef(int voiceIndex);
    
    /**
     * Find voice index playing a specific note
     * @param note MIDI note number
     * @param channel MIDI channel (optional, -1 for any channel)
     * @return Voice index, or -1 if not found
     */
    int findVoiceForNote(int note, int channel = -1) const;
    
    /**
     * Get all active voices
     * @return Vector of voice indices that are currently active
     */
    std::vector<int> getActiveVoices() const;
    
    /**
     * Get number of active voices
     * @return Count of currently active voices
     */
    int getActiveVoiceCount() const;
    
    /**
     * Mark a voice as finished (called by synth engine when voice completes)
     * @param voiceIndex Voice index to mark as finished
     */
    void markVoiceFinished(int voiceIndex);
    
    /**
     * Mark a voice as sustained (when sustain pedal is pressed and note off received)
     * @param voiceIndex Voice index to mark as sustained
     * @param note MIDI note that was released
     * @param channel MIDI channel
     */
    void markVoiceAsSustained(int voiceIndex, int note, int channel);
    
    // =========================
    // Configuration
    // =========================
    
    /**
     * Set maximum number of voices
     * @param maxVoices New maximum voice count
     */
    void setMaxVoices(int maxVoices);
    
    /**
     * Get maximum number of voices
     * @return Maximum voice count
     */
    int getMaxVoices() const { return maxVoices_; }
    
    /**
     * Set voice stealing mode
     * @param mode New voice stealing strategy
     */
    void setVoiceStealingMode(VoiceStealingMode mode) { stealingMode_ = mode; }
    
    /**
     * Get current voice stealing mode
     * @return Current voice stealing strategy
     */
    VoiceStealingMode getVoiceStealingMode() const { return stealingMode_; }
    
    /**
     * Enable/disable sustain pedal globally
     * @param enabled True to enable sustain pedal processing
     */
    void setSustainEnabled(bool enabled) { sustainEnabled_ = enabled; }
    
    /**
     * Check if sustain pedal is enabled
     * @return True if sustain pedal processing is enabled
     */
    bool isSustainEnabled() const { return sustainEnabled_; }
    
    // =========================
    // ADSR Configuration
    // =========================
    
    /**
     * Set amplitude ADSR parameters for all voices
     * @param attackTime Attack time in seconds
     * @param decayTime Decay time in seconds
     * @param sustainLevel Sustain level (0.0 to 1.0)
     * @param releaseTime Release time in seconds
     */
    void setAmplitudeADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime);
    
    /**
     * Set amplitude ADSR curve for all voices
     * @param curve Curve amount (0.0 = linear, > 0.0 = exponential)
     */
    void setAmplitudeADSRCurve(double curve);
    
    /**
     * Enable filter envelopes for all voices
     */
    void enableFilterEnvelopes();
    
    /**
     * Set filter ADSR parameters for all voices (must call enableFilterEnvelopes first)
     * @param attackTime Attack time in seconds
     * @param decayTime Decay time in seconds
     * @param sustainLevel Sustain level (0.0 to 1.0)
     * @param releaseTime Release time in seconds
     */
    void setFilterADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime);
    
    /**
     * Enable pitch envelopes for all voices
     */
    void enablePitchEnvelopes();
    
    /**
     * Set pitch ADSR parameters for all voices (must call enablePitchEnvelopes first)
     * @param attackTime Attack time in seconds
     * @param decayTime Decay time in seconds
     * @param sustainLevel Sustain level (0.0 to 1.0)
     * @param releaseTime Release time in seconds
     */
    void setPitchADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime);
    
    /**
     * Initialize all voice envelopes with the current sample rate
     * @param sampleRate Audio engine sample rate
     */
    void initializeEnvelopes(double sampleRate);
    
    // =========================
    // Status and Debug
    // =========================
    
    /**
     * Reset all voices and clear state
     */
    void reset();
    
    /**
     * Get current sustain pedal state for a channel
     * @param channel MIDI channel (0-15)
     * @return True if sustain pedal is pressed
     */
    bool isSustainPedalPressed(int channel = 0) const;
    
    /**
     * Print current voice allocation state (for debugging)
     */
    void printVoiceState() const;

private:
    // =========================
    // Private Methods
    // =========================
    
    /**
     * Find an available voice slot
     * @return Voice index, or -1 if no voices available
     */
    int findAvailableVoice();
    
    /**
     * Steal a voice based on current stealing mode
     * @return Voice index to steal, or -1 if no voice can be stolen
     */
    int stealVoice();
    
    /**
     * Find oldest voice for stealing
     * @return Voice index of oldest voice
     */
    int findOldestVoice() const;
    
    /**
     * Find voice with lowest velocity for stealing
     * @return Voice index of lowest velocity voice
     */
    int findLowestVelocityVoice() const;
    
    /**
     * Find voice with highest note for stealing
     * @return Voice index of highest note voice
     */
    int findHighestNoteVoice() const;
    
    /**
     * Find voice with lowest note for stealing
     * @return Voice index of lowest note voice
     */
    int findLowestNoteVoice() const;
    
    /**
     * Release all sustained voices for a channel
     * @param channel MIDI channel
     */
    void releaseSustainedVoices(int channel);
    
    /**
     * Release all sustained voices globally (across all channels)
     * Used for cross-channel sustain pedal support
     */
    void releaseSustainedVoicesGlobal();
    
    /**
     * Generate unique voice ID
     * @return New unique voice ID
     */
    uint32_t generateVoiceId() { return ++nextVoiceId_; }
    
    // =========================
    // Member Variables
    // =========================
    
    std::vector<Voice> voices_;                    // Voice pool
    int maxVoices_;                               // Maximum number of voices
    VoiceStealingMode stealingMode_;              // Voice stealing strategy
    
    // Sustain pedal state per channel (0-15)
    std::vector<bool> sustainPedalPressed_;       // Sustain pedal state per channel
    bool sustainEnabled_;                         // Global sustain enable/disable
    
    // Voice ID tracking
    uint32_t nextVoiceId_;                        // Next voice ID to assign
    
    // Statistics (optional, for debugging/monitoring)
    mutable std::chrono::steady_clock::time_point lastUpdate_;
    mutable int voiceStealCount_;                 // Number of times voices were stolen
};
