#include "PolyphonicSampler.h"
#include <algorithm>
#include <cmath>

PolyphonicSampler::PolyphonicSampler(const std::string& name, 
                                   int maxVoices, 
                                   VoiceStealingMode stealingMode)
    : AudioNode(name), voiceAllocator_(maxVoices, stealingMode)
{
    // Create all voice sample players
    voices_.reserve(maxVoices);
    for (int i = 0; i < maxVoices; ++i) {
        auto voiceName = name + "_Voice" + std::to_string(i);
        voices_.emplace_back(std::make_unique<SamplePlayerNode>(voiceName));
        
        // Connect ADSR envelopes from voice allocator to sample player
        auto& allocatorVoice = voiceAllocator_.getVoiceRef(i);
        voices_[i]->setAmplitudeEnvelope(&allocatorVoice.amplitudeEnvelope);
        // Filter and pitch envelopes will be connected when enabled
    }
    
    Logger::info("PolyphonicSampler '{}' created with {} voices", name, maxVoices);
}

void PolyphonicSampler::prepare(const PrepareInfo& info) {
    // Initialize voice allocator envelopes with sample rate
    voiceAllocator_.initializeEnvelopes(info.sampleRate);
    
    // Prepare all voice sample players
    for (auto& voice : voices_) {
        voice->prepare(info);
    }
    
    Logger::debug("PolyphonicSampler '{}' prepared: SR={} Hz, MaxBlock={}", 
                 getName(), info.sampleRate, info.maxBufferSize);
}

void PolyphonicSampler::processCallback(choc::buffer::ChannelArrayView<const float> input,
                                       choc::buffer::ChannelArrayView<float> output,
                                       double sampleRate,
                                       int numSamples) {
    // Clear output first
    output.clear();
    
    if (!hasSample()) {
        return;
    }
    
    const int outputChannels = static_cast<int>(output.getNumChannels());
    const int maxVoices = getMaxVoices();
    
    // Create temporary buffer for mixing voices
    choc::buffer::ChannelArrayBuffer<float> tempBuffer(outputChannels, numSamples);
    
    // Process each active voice and mix into output
    int activeVoices = 0;
    for (int voiceIndex = 0; voiceIndex < maxVoices; ++voiceIndex) {
        const auto& voice = voiceAllocator_.getVoice(voiceIndex);
        
        if (voice.isInUse()) {
            auto& sampler = *voices_[voiceIndex];
            
            // Get reference to allocator voice for envelope processing
            auto& allocatorVoice = voiceAllocator_.getVoiceRef(voiceIndex);
            
            // Process envelope for this block and check if finished
            double lastEnvelopeValue = 0.0;
            for (int i = 0; i < numSamples; ++i) {
                lastEnvelopeValue = allocatorVoice.amplitudeEnvelope.processSample();
                
                // Process optional envelopes
                if (allocatorVoice.filterEnvelope) {
                    allocatorVoice.filterEnvelope->processSample();
                }
                if (allocatorVoice.pitchEnvelope) {
                    allocatorVoice.pitchEnvelope->processSample();
                }
            }
            
            // Check if envelope is finished and voice should be stopped
            if (voice.isReleasing && allocatorVoice.amplitudeEnvelope.isFinished()) {
                Logger::debug("PolyphonicSampler '{}': Voice {} finished release", getName(), voiceIndex);
                sampler.stop();
                voiceAllocator_.markVoiceFinished(voiceIndex);
                continue;
            }
            
            // Check if voice should be playing
            if (sampler.getPlaybackState() != SamplePlayerNode::PlaybackState::PLAYING) {
                // Voice should be active but sampler isn't playing, mark as finished
                Logger::debug("PolyphonicSampler '{}': Voice {} not playing but still active, marking finished", 
                              getName(), voiceIndex);
                voiceAllocator_.markVoiceFinished(voiceIndex);
                continue;
            }
            
            // Clear temp buffer for this voice
            tempBuffer.clear();
            
            // Process this voice
            sampler.processCallback(input, tempBuffer.getView(), sampleRate, numSamples);
            
            // Mix this voice into the output
            for (int ch = 0; ch < outputChannels; ++ch) {
                for (int sample = 0; sample < numSamples; ++sample) {
                    output.getSample(static_cast<choc::buffer::ChannelCount>(ch), 
                                   static_cast<choc::buffer::FrameCount>(sample)) +=
                        tempBuffer.getSample(static_cast<choc::buffer::ChannelCount>(ch), 
                                           static_cast<choc::buffer::FrameCount>(sample));
                }
            }
            
            activeVoices++;
        }
    }
    
    // Update audio analysis
    updateAnalysis(output);
}

bool PolyphonicSampler::loadSample(const std::string& filePath) {
    try {
        choc::audio::WAVAudioFileFormat<false> wavFormat;
        auto reader = wavFormat.createReader(filePath);
        
        if (!reader) {
            Logger::error("PolyphonicSampler '{}': Failed to create reader for file: {}", 
                         getName(), filePath);
            return false;
        }
        
        // Load the entire file
        auto data = reader->loadFileContent();
        
        if (data.frames.getNumFrames() == 0) {
            Logger::error("PolyphonicSampler '{}': No audio data in file: {}", 
                         getName(), filePath);
            return false;
        }
        
        // Store sample data
        sampleBuffer_ = std::move(data.frames);
        sampleSampleRate_ = data.sampleRate;
        loadedFilePath_ = filePath;
        
        // Load sample into all voices
        bool allLoaded = true;
        for (int i = 0; i < getMaxVoices(); ++i) {
            if (!voices_[i]->loadSample(sampleBuffer_, sampleSampleRate_)) {
                Logger::error("PolyphonicSampler '{}': Failed to load sample into voice {}", 
                             getName(), i);
                allLoaded = false;
            } else {
                applyGlobalParametersToVoice(i);
            }
        }
        
        if (allLoaded) {
            Logger::info("PolyphonicSampler '{}': Loaded sample '{}' into {} voices - {} channels, {} samples, {:.1f} Hz", 
                        getName(), filePath, getMaxVoices(), sampleBuffer_.getNumChannels(), 
                        sampleBuffer_.getNumFrames(), sampleSampleRate_);
        }
        
        return allLoaded;
        
    } catch (const std::exception& e) {
        Logger::error("PolyphonicSampler '{}': Exception loading file '{}': {}", 
                     getName(), filePath, e.what());
        return false;
    }
}

bool PolyphonicSampler::loadSample(const choc::buffer::ChannelArrayBuffer<float>& buffer, double sampleRate) {
    if (buffer.getNumFrames() == 0) {
        Logger::error("PolyphonicSampler '{}': Cannot load empty buffer", getName());
        return false;
    }
    
    // Store sample data
    sampleBuffer_ = buffer;
    sampleSampleRate_ = sampleRate;
    loadedFilePath_ = "<buffer>";
    
    // Load sample into all voices
    bool allLoaded = true;
    for (int i = 0; i < getMaxVoices(); ++i) {
        if (!voices_[i]->loadSample(sampleBuffer_, sampleSampleRate_)) {
            Logger::error("PolyphonicSampler '{}': Failed to load sample into voice {}", 
                         getName(), i);
            allLoaded = false;
        } else {
            applyGlobalParametersToVoice(i);
        }
    }
    
    if (allLoaded) {
        Logger::info("PolyphonicSampler '{}': Loaded buffer into {} voices - {} channels, {} samples, {:.1f} Hz", 
                    getName(), getMaxVoices(), sampleBuffer_.getNumChannels(), 
                    sampleBuffer_.getNumFrames(), sampleSampleRate_);
    }
    
    return allLoaded;
}

void PolyphonicSampler::unloadSample() {
    // Stop all voices and unload samples
    allSoundOff();
    
    for (auto& voice : voices_) {
        voice->unloadSample();
    }
    
    sampleBuffer_ = choc::buffer::ChannelArrayBuffer<float>();
    sampleSampleRate_ = 44100.0;
    loadedFilePath_.clear();
    
    Logger::info("PolyphonicSampler '{}': Sample unloaded from all voices", getName());
}

bool PolyphonicSampler::hasSample() const {
    return !voices_.empty() && voices_[0]->hasSample();
}

const std::string& PolyphonicSampler::getLoadedFilePath() const {
    return loadedFilePath_;
}

int PolyphonicSampler::processMidiMessage(const choc::midi::ShortMessage& message) {
    if (message.isNoteOn()) {
        return noteOn(message.getNoteNumber(), message.getVelocity(), message.getChannel0to15());
    }
    else if (message.isNoteOff()) {
        return noteOff(message.getNoteNumber(), message.getChannel0to15());
    }
    else if (message.isController()) {
        // Delegate controller messages to voice allocator for sustain, all sound off, etc.
        return voiceAllocator_.processMidiMessage(message);
    }
    
    return -1; // No voice affected
}

int PolyphonicSampler::noteOn(int note, int velocity, int channel) {
    if (!hasSample()) {
        Logger::warn("PolyphonicSampler '{}': Cannot play note {} - no sample loaded", 
                    getName(), note);
        return -1;
    }
    
    // Allocate voice through voice allocator
    int voiceIndex = voiceAllocator_.noteOn(note, velocity, channel);
    
    if (voiceIndex >= 0) {
        // Configure and trigger the voice's sample player
        auto& sampler = *voices_[voiceIndex];
        
        // Set the note for pitch adjustment
        sampler.setCurrentNote(note);
        
        // Set velocity-based volume (simple linear mapping)
        float velocityVolume = velocity / 127.0f;
        sampler.setVolume(globalVolume_ * velocityVolume);
        
        // Trigger the sample
        sampler.trigger(note);
        
        Logger::debug("PolyphonicSampler '{}': Note ON - Note: {}, Velocity: {}, Voice: {}", 
                     getName(), note, velocity, voiceIndex);
    } else {
        Logger::debug("PolyphonicSampler '{}': Note ON failed - Note: {}, Velocity: {} (no available voice)", 
                     getName(), note, velocity);
    }
    
    return voiceIndex;
}

int PolyphonicSampler::noteOff(int note, int channel) {
    // Check if sustain pedal is pressed on ANY channel (common MIDI setup issue)
    bool sustainPressed = false;
    for (int ch = 0; ch < 16; ++ch) {
        if (voiceAllocator_.isSustainPedalPressed(ch)) {
            sustainPressed = true;
            break;
        }
    }
    
    if (sustainPressed) {
        // Find the voice playing this note and mark it as sustained (but keep playing)
        int voiceIndex = voiceAllocator_.findVoiceForNote(note, channel);
        if (voiceIndex >= 0) {
            voiceAllocator_.markVoiceAsSustained(voiceIndex, note, channel);
            Logger::debug("PolyphonicSampler '{}': Note OFF - Note: {}, Voice: {} (sustained)", 
                         getName(), note, voiceIndex);
        }
        return voiceIndex;
    } else {
        // Normal note off - release the voice but keep playing during envelope release
        int voiceIndex = voiceAllocator_.noteOff(note, channel);
        
        if (voiceIndex >= 0) {
            // Don't stop the sampler immediately - let it continue playing while the 
            // ADSR envelope fades out. The voice will be stopped when the envelope finishes.
            Logger::debug("PolyphonicSampler '{}': Note OFF - Note: {}, Voice: {} (releasing)", 
                         getName(), note, voiceIndex);
        }
        
        return voiceIndex;
    }
}

void PolyphonicSampler::setSustainPedal(int value, int channel) {
    voiceAllocator_.setSustainPedal(value, channel);
    
    // If sustain is released, stop any voices that were being sustained
    // Check if ANY sustain pedal is still pressed before releasing sustained voices
    if (value < 64) {
        bool anyChannelSustained = false;
        for (int ch = 0; ch < 16; ++ch) {
            if (voiceAllocator_.isSustainPedalPressed(ch)) {
                anyChannelSustained = true;
                break;
            }
        }
        
        // Only release sustained voices if NO sustain pedal is pressed on any channel
        if (!anyChannelSustained) {
            for (int i = 0; i < getMaxVoices(); ++i) {
                const auto& voice = voiceAllocator_.getVoice(i);
                // Stop any voice that was sustained
                if (voice.isSustained) {
                    voices_[i]->stop();
                    voiceAllocator_.markVoiceFinished(i);
                    Logger::debug("PolyphonicSampler '{}': Stopping sustained voice {} (note {})", 
                                 getName(), i, voice.note);
                }
            }
        }
    }
    
    Logger::debug("PolyphonicSampler '{}': Sustain pedal: {}", getName(), value >= 64 ? "ON" : "OFF");
}

void PolyphonicSampler::allNotesOff(int channel) {
    voiceAllocator_.allNotesOff(channel);
    
    // Stop all voice sample players
    for (auto& voice : voices_) {
        voice->stop();
    }
    
    Logger::debug("PolyphonicSampler '{}': All notes off", getName());
}

void PolyphonicSampler::allSoundOff(int channel) {
    voiceAllocator_.allSoundOff(channel);
    
    // Stop all voice sample players immediately
    for (auto& voice : voices_) {
        voice->stop();
    }
    
    Logger::debug("PolyphonicSampler '{}': All sound off", getName());
}

SamplePlayerNode& PolyphonicSampler::getVoiceSampler(int voiceIndex) {
    if (voiceIndex < 0 || voiceIndex >= getMaxVoices()) {
        throw std::out_of_range("Voice index out of range");
    }
    return *voices_[voiceIndex];
}

const SamplePlayerNode& PolyphonicSampler::getVoiceSampler(int voiceIndex) const {
    if (voiceIndex < 0 || voiceIndex >= getMaxVoices()) {
        throw std::out_of_range("Voice index out of range");
    }
    return *voices_[voiceIndex];
}

int PolyphonicSampler::getActiveVoiceCount() const {
    return voiceAllocator_.getActiveVoiceCount();
}

int PolyphonicSampler::getMaxVoices() const {
    return voiceAllocator_.getMaxVoices();
}

void PolyphonicSampler::setVoiceStealingMode(VoiceStealingMode mode) {
    voiceAllocator_.setVoiceStealingMode(mode);
    Logger::debug("PolyphonicSampler '{}': Voice stealing mode set to {}", getName(), static_cast<int>(mode));
}

VoiceStealingMode PolyphonicSampler::getVoiceStealingMode() const {
    return voiceAllocator_.getVoiceStealingMode();
}

void PolyphonicSampler::setGain(float gain) {
    globalGain_ = gain;
    for (auto& voice : voices_) {
        voice->setGain(gain);
    }
}

void PolyphonicSampler::setVolume(float volume) {
    globalVolume_ = volume;
    for (auto& voice : voices_) {
        voice->setVolume(volume);
    }
}

void PolyphonicSampler::setInterpolationMode(SamplePlayerNode::InterpolationMode mode) {
    globalInterpolationMode_ = mode;
    for (auto& voice : voices_) {
        voice->setInterpolationMode(mode);
    }
}

void PolyphonicSampler::setLoop(bool loop) {
    globalLoop_ = loop;
    for (auto& voice : voices_) {
        voice->setLoop(loop);
    }
}

void PolyphonicSampler::setBaseNote(int baseNote) {
    globalBaseNote_ = baseNote;
    for (auto& voice : voices_) {
        voice->setBaseNote(baseNote);
    }
}

void PolyphonicSampler::setTranspose(int transpose) {
    globalTranspose_ = transpose;
    for (auto& voice : voices_) {
        voice->setTranspose(transpose);
    }
}

void PolyphonicSampler::setDetune(float detune) {
    globalDetune_ = detune;
    for (auto& voice : voices_) {
        voice->setDetune(detune);
    }
}

void PolyphonicSampler::setSampleRegion(int startSample, int endSample) {
    globalStartSample_ = startSample;
    globalEndSample_ = endSample;
    for (auto& voice : voices_) {
        voice->setSampleRegion(startSample, endSample);
    }
}

void PolyphonicSampler::setLoopRegion(int loopStart, int loopEnd) {
    globalLoopStart_ = loopStart;
    globalLoopEnd_ = loopEnd;
    for (auto& voice : voices_) {
        voice->setLoopRegion(loopStart, loopEnd);
    }
}

float PolyphonicSampler::getPeakLevel() const {
    return currentPeakLevel_;
}

float PolyphonicSampler::getRMSLevel() const {
    return currentRMSLevel_;
}

void PolyphonicSampler::printSamplerInfo() const {
    Logger::info("=== PolyphonicSampler '{}' Info ===", getName());
    Logger::info("Sample: {}", loadedFilePath_);
    
    if (hasSample()) {
        Logger::info("Channels: {}, Samples: {}, Sample Rate: {:.1f} Hz", 
                    sampleBuffer_.getNumChannels(), sampleBuffer_.getNumFrames(), sampleSampleRate_);
    }
    
    Logger::info("Voices: {} / {} active", getActiveVoiceCount(), getMaxVoices());
    Logger::info("Global Settings:");
    Logger::info("  Gain: {:.2f}, Volume: {:.2f}", globalGain_, globalVolume_);
    Logger::info("  Base Note: {}, Transpose: {}, Detune: {:.1f}c", 
                globalBaseNote_, globalTranspose_, globalDetune_);
    Logger::info("  Loop: {}, Interpolation: {}", 
                globalLoop_ ? "ON" : "OFF",
                (globalInterpolationMode_ == SamplePlayerNode::InterpolationMode::NONE ? "NONE" :
                 globalInterpolationMode_ == SamplePlayerNode::InterpolationMode::LINEAR ? "LINEAR" : "CUBIC"));
    Logger::info("Audio Levels: Peak: {:.3f}, RMS: {:.3f}", currentPeakLevel_, currentRMSLevel_);
    Logger::info("=====================================");
}

void PolyphonicSampler::printActiveVoicesInfo() const {
    Logger::info("=== Active Voices for '{}' ===", getName());
    
    int activeCount = 0;
    for (int i = 0; i < getMaxVoices(); ++i) {
        const auto& voice = voiceAllocator_.getVoice(i);
        if (voice.isInUse()) {
            const auto& sampler = *voices_[i];
            Logger::info("Voice {}: Note {}, Velocity {}, State: {}", 
                        i, voice.note, voice.velocity,
                        (sampler.getPlaybackState() == SamplePlayerNode::PlaybackState::PLAYING ? "PLAYING" :
                         sampler.getPlaybackState() == SamplePlayerNode::PlaybackState::PAUSED ? "PAUSED" : "STOPPED"));
            activeCount++;
        }
    }
    
    if (activeCount == 0) {
        Logger::info("No active voices");
    }
    
    Logger::info("=============================");
}

// =========================
// ADSR Configuration Methods
// =========================

void PolyphonicSampler::setAmplitudeADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime) {
    Logger::info("PolyphonicSampler '{}': Setting amplitude ADSR - A:{:.3f}s D:{:.3f}s S:{:.3f} R:{:.3f}s", 
                getName(), attackTime, decayTime, sustainLevel, releaseTime);
    
    voiceAllocator_.setAmplitudeADSR(attackTime, decayTime, sustainLevel, releaseTime);
}

void PolyphonicSampler::setAmplitudeADSRCurve(double curve) {
    Logger::info("PolyphonicSampler '{}': Setting amplitude ADSR curve to {:.3f}", getName(), curve);
    
    voiceAllocator_.setAmplitudeADSRCurve(curve);
}

void PolyphonicSampler::enableFilterEnvelopes() {
    Logger::info("PolyphonicSampler '{}': Enabling filter envelopes", getName());
    
    voiceAllocator_.enableFilterEnvelopes();
    
    // Connect filter envelopes to sample players
    for (int i = 0; i < getMaxVoices(); ++i) {
        auto& allocatorVoice = voiceAllocator_.getVoiceRef(i);
        if (allocatorVoice.filterEnvelope) {
            voices_[i]->setFilterEnvelope(allocatorVoice.filterEnvelope.get());
        }
    }
}

void PolyphonicSampler::setFilterADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime) {
    Logger::info("PolyphonicSampler '{}': Setting filter ADSR - A:{:.3f}s D:{:.3f}s S:{:.3f} R:{:.3f}s", 
                getName(), attackTime, decayTime, sustainLevel, releaseTime);
    
    voiceAllocator_.setFilterADSR(attackTime, decayTime, sustainLevel, releaseTime);
}

void PolyphonicSampler::enablePitchEnvelopes() {
    Logger::info("PolyphonicSampler '{}': Enabling pitch envelopes", getName());
    
    voiceAllocator_.enablePitchEnvelopes();
    
    // Connect pitch envelopes to sample players
    for (int i = 0; i < getMaxVoices(); ++i) {
        auto& allocatorVoice = voiceAllocator_.getVoiceRef(i);
        if (allocatorVoice.pitchEnvelope) {
            voices_[i]->setPitchEnvelope(allocatorVoice.pitchEnvelope.get());
        }
    }
}

void PolyphonicSampler::setPitchADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime) {
    Logger::info("PolyphonicSampler '{}': Setting pitch ADSR - A:{:.3f}s D:{:.3f}s S:{:.3f} R:{:.3f}s", 
                getName(), attackTime, decayTime, sustainLevel, releaseTime);
    
    voiceAllocator_.setPitchADSR(attackTime, decayTime, sustainLevel, releaseTime);
}

// =========================
// Private Helper Methods
// =========================

void PolyphonicSampler::applyGlobalParametersToVoice(int voiceIndex) {
    if (voiceIndex < 0 || voiceIndex >= static_cast<int>(voices_.size())) {
        return;
    }
    
    auto& voice = *voices_[voiceIndex];
    
    // Apply global parameters to this voice
    voice.setGain(globalGain_);
    voice.setVolume(globalVolume_);
    voice.setInterpolationMode(globalInterpolationMode_);
    voice.setLoop(globalLoop_);
    voice.setBaseNote(globalBaseNote_);
    voice.setTranspose(globalTranspose_);
    voice.setDetune(globalDetune_);
    
    // Apply sample region settings
    if (globalEndSample_ > 0) {
        voice.setSampleRegion(globalStartSample_, globalEndSample_);
    }
    
    // Apply loop region settings
    if (globalLoopEnd_ > 0) {
        voice.setLoopRegion(globalLoopStart_, globalLoopEnd_);
    }
}

void PolyphonicSampler::updateAnalysis(const choc::buffer::ChannelArrayView<float>& output) {
    if (output.getNumFrames() == 0) {
        return;
    }
    
    float peak = 0.0f;
    float rms = 0.0f;
    int totalSamples = 0;
    
    // Calculate peak and RMS across all channels
    for (choc::buffer::ChannelCount ch = 0; ch < output.getNumChannels(); ++ch) {
        for (choc::buffer::FrameCount frame = 0; frame < output.getNumFrames(); ++frame) {
            float sample = std::abs(output.getSample(ch, frame));
            peak = std::max(peak, sample);
            rms += sample * sample;
            totalSamples++;
        }
    }
    
    if (totalSamples > 0) {
        rms = std::sqrt(rms / totalSamples);
    }
    
    // Apply simple exponential smoothing
    const float alpha = 0.1f;
    currentPeakLevel_ = peak * alpha + currentPeakLevel_ * (1.0f - alpha);
    currentRMSLevel_ = rms * alpha + currentRMSLevel_ * (1.0f - alpha);
}
