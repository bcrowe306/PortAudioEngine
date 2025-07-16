#pragma once

#include "AudioNode.h"
#include "VoiceAllocator.h"
#include "SamplePlayerNode.h"
#include "Logger.h"
#include <vector>
#include <memory>
#include <string>

/**
 * PolyphonicSampler - A polyphonic sampler that combines VoiceAllocator with SamplePlayerNode
 * 
 * Features:
 * - Polyphonic sample playback (multiple voices)
 * - MIDI note processing with voice allocation
 * - Sustain pedal support
 * - Voice stealing for efficient voice management
 * - Per-voice sample triggering and pitch adjustment
 * - Mixed output from all active voices
 */
class PolyphonicSampler : public AudioNode {
public:
    /**
     * Constructor
     * @param name Node name
     * @param maxVoices Maximum number of simultaneous voices (default: 16)
     * @param stealingMode Voice stealing strategy
     */
    explicit PolyphonicSampler(const std::string& name, 
                              int maxVoices = 16, 
                              VoiceStealingMode stealingMode = VoiceStealingMode::OLDEST);
    
    /**
     * Destructor
     */
    ~PolyphonicSampler() override = default;
    
    // =========================
    // AudioNode Interface
    // =========================
    
    void prepare(const PrepareInfo& info) override;
    
    void processCallback(choc::buffer::ChannelArrayView<const float> input,
                        choc::buffer::ChannelArrayView<float> output,
                        double sampleRate,
                        int numSamples) override;
    
    // =========================
    // Sample Management
    // =========================
    
    /**
     * Load a sample file for all voices
     * @param filePath Path to the audio file
     * @return True if sample was loaded successfully
     */
    bool loadSample(const std::string& filePath);
    
    /**
     * Load a sample from buffer for all voices
     * @param buffer Audio buffer to load
     * @param sampleRate Sample rate of the buffer
     * @return True if sample was loaded successfully
     */
    bool loadSample(const choc::buffer::ChannelArrayBuffer<float>& buffer, double sampleRate);
    
    /**
     * Unload the current sample from all voices
     */
    void unloadSample();
    
    /**
     * Check if a sample is loaded
     * @return True if sample is loaded
     */
    bool hasSample() const;
    
    /**
     * Get the loaded sample file path
     * @return File path or "<buffer>" if loaded from buffer
     */
    const std::string& getLoadedFilePath() const;
    
    // =========================
    // MIDI Processing
    // =========================
    
    /**
     * Process a MIDI message
     * @param message MIDI message to process
     * @return Voice index that was affected, or -1 if no voice was affected
     */
    int processMidiMessage(const choc::midi::ShortMessage& message);
    
    /**
     * Trigger a note
     * @param note MIDI note number (0-127)
     * @param velocity Note velocity (0-127)
     * @param channel MIDI channel (0-15)
     * @return Voice index allocated for this note, or -1 if failed
     */
    int noteOn(int note, int velocity, int channel = 0);
    
    /**
     * Release a note
     * @param note MIDI note number (0-127)
     * @param channel MIDI channel (0-15)
     * @return Voice index that was released, or -1 if note wasn't playing
     */
    int noteOff(int note, int channel = 0);
    
    /**
     * Handle sustain pedal
     * @param value Sustain value (0-127, <64 = off, >=64 = on)
     * @param channel MIDI channel (0-15)
     */
    void setSustainPedal(int value, int channel = 0);
    
    /**
     * Stop all notes
     * @param channel MIDI channel (0-15)
     */
    void allNotesOff(int channel = 0);
    
    /**
     * Stop all sound immediately
     * @param channel MIDI channel (0-15)
     */
    void allSoundOff(int channel = 0);
    
    // =========================
    // Voice Management
    // =========================
    
    /**
     * Get the voice allocator
     * @return Reference to the voice allocator
     */
    VoiceAllocator& getVoiceAllocator() { return voiceAllocator_; }
    
    /**
     * Get the voice allocator (const)
     * @return Const reference to the voice allocator
     */
    const VoiceAllocator& getVoiceAllocator() const { return voiceAllocator_; }
    
    /**
     * Get a specific voice's sample player
     * @param voiceIndex Voice index (0 to maxVoices-1)
     * @return Reference to the voice's sample player
     */
    SamplePlayerNode& getVoiceSampler(int voiceIndex);
    
    /**
     * Get a specific voice's sample player (const)
     * @param voiceIndex Voice index (0 to maxVoices-1)
     * @return Const reference to the voice's sample player
     */
    const SamplePlayerNode& getVoiceSampler(int voiceIndex) const;
    
    /**
     * Get number of active voices
     * @return Count of currently active voices
     */
    int getActiveVoiceCount() const;
    
    /**
     * Get maximum number of voices
     * @return Maximum voice count
     */
    int getMaxVoices() const;
    
    /**
     * Set voice stealing mode
     * @param mode Voice stealing strategy
     */
    void setVoiceStealingMode(VoiceStealingMode mode);
    
    /**
     * Get current voice stealing mode
     * @return Current voice stealing strategy
     */
    VoiceStealingMode getVoiceStealingMode() const;
    
    // =========================
    // Global Sample Parameters
    // =========================
    
    /**
     * Set gain for all voices
     * @param gain Gain value (0.0 to 1.0+)
     */
    void setGain(float gain);
    
    /**
     * Set volume for all voices
     * @param volume Volume value (0.0 to 1.0)
     */
    void setVolume(float volume);
    
    /**
     * Set interpolation mode for all voices
     * @param mode Interpolation mode
     */
    void setInterpolationMode(SamplePlayerNode::InterpolationMode mode);
    
    /**
     * Set loop mode for all voices
     * @param loop True to enable looping
     */
    void setLoop(bool loop);
    
    /**
     * Set base note for all voices
     * @param baseNote MIDI note number (0-127)
     */
    void setBaseNote(int baseNote);
    
    /**
     * Set transpose for all voices
     * @param transpose Semitones to transpose (-48 to +48)
     */
    void setTranspose(int transpose);
    
    /**
     * Set detune for all voices
     * @param detune Detune in cents (-100 to +100)
     */
    void setDetune(float detune);
    
    /**
     * Set sample region for all voices
     * @param startSample Start sample index
     * @param endSample End sample index (0 = use full sample)
     */
    void setSampleRegion(int startSample, int endSample);
    
    /**
     * Set loop region for all voices
     * @param loopStart Loop start sample index
     * @param loopEnd Loop end sample index (0 = use end of sample region)
     */
    void setLoopRegion(int loopStart, int loopEnd);
    
    // =========================
    // Analysis & Info
    // =========================
    
    /**
     * Get current peak level across all voices
     * @return Peak level (0.0 to 1.0+)
     */
    float getPeakLevel() const;
    
    /**
     * Get current RMS level across all voices
     * @return RMS level (0.0 to 1.0+)
     */
    float getRMSLevel() const;
    
    /**
     * Print detailed information about the sampler and all voices
     */
    void printSamplerInfo() const;
    
    /**
     * Print information about active voices only
     */
    void printActiveVoicesInfo() const;
    
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

private:
    // =========================
    // Private Members
    // =========================
    
    VoiceAllocator voiceAllocator_;
    std::vector<std::unique_ptr<SamplePlayerNode>> voices_;
    
    // Global parameters applied to new voices
    float globalGain_ = 1.0f;
    float globalVolume_ = 1.0f;
    SamplePlayerNode::InterpolationMode globalInterpolationMode_ = SamplePlayerNode::InterpolationMode::LINEAR;
    bool globalLoop_ = false;
    int globalBaseNote_ = 60; // Middle C
    int globalTranspose_ = 0;
    float globalDetune_ = 0.0f;
    int globalStartSample_ = 0;
    int globalEndSample_ = 0;
    int globalLoopStart_ = 0;
    int globalLoopEnd_ = 0;
    
    // Current sample info
    std::string loadedFilePath_;
    choc::buffer::ChannelArrayBuffer<float> sampleBuffer_;
    double sampleSampleRate_ = 44100.0;
    
    // Audio analysis
    float currentPeakLevel_ = 0.0f;
    float currentRMSLevel_ = 0.0f;
    
    // =========================
    // Private Methods
    // =========================
    
    /**
     * Initialize a voice with current global parameters
     * @param voiceIndex Index of voice to initialize
     */
    void initializeVoice(int voiceIndex);
    
    /**
     * Apply global parameters to a specific voice
     * @param voiceIndex Index of voice to configure
     */
    void applyGlobalParametersToVoice(int voiceIndex);
    
    /**
     * Update audio analysis from mixed output
     * @param output Output buffer to analyze
     */
    void updateAnalysis(const choc::buffer::ChannelArrayView<float>& output);
};
