#include "VoiceAllocator.h"
#include <algorithm>
#include <iostream>

VoiceAllocator::VoiceAllocator(int maxVoices, VoiceStealingMode stealingMode)
    : maxVoices_(maxVoices)
    , stealingMode_(stealingMode)
    , sustainEnabled_(true)
    , nextVoiceId_(1)
    , voiceStealCount_(0)
{
    // Initialize voice pool with proper indices
    voices_.reserve(maxVoices_);
    for (int i = 0; i < maxVoices_; ++i) {
        voices_.emplace_back(i);  // Voice constructor takes voice index
        voices_[i].reset();
    }
    
    // Initialize sustain pedal state for all 16 MIDI channels
    sustainPedalPressed_.resize(16, false);
    
    Logger::info("VoiceAllocator initialized with {} voices", maxVoices_);
}

int VoiceAllocator::processMidiMessage(const choc::midi::ShortMessage& message) {
    if (message.isNoteOn()) {
        return noteOn(message.getNoteNumber(), message.getVelocity(), message.getChannel0to15());
    }
    else if (message.isNoteOff()) {
        return noteOff(message.getNoteNumber(), message.getChannel0to15());
    }
    else if (message.isController()) {
        int controller = message.getControllerNumber();
        int value = message.getControllerValue();
        int channel = message.getChannel0to15();
        
        switch (controller) {
            case 64: // Sustain pedal
                setSustainPedal(value, channel);
                break;
            case 120: // All sound off
                allSoundOff(channel);
                break;
            case 123: // All notes off
                allNotesOff(channel);
                break;
        }
    }
    
    return -1; // No voice affected
}

int VoiceAllocator::noteOn(int note, int velocity, int channel) {
    if (velocity == 0) {
        // Velocity 0 note-on is treated as note-off
        return noteOff(note, channel);
    }
    
    Logger::debug("VoiceAllocator: Note ON - Note: {}, Vel: {}, Ch: {}", note, velocity, channel);
    
    // Check if this note is already playing (for retriggering)
    int existingVoice = findVoiceForNote(note, channel);
    if (existingVoice != -1) {
        // Retrigger existing voice
        Voice& voice = voices_[existingVoice];
        voice.velocity = velocity;
        voice.triggerTime = std::chrono::steady_clock::now();
        voice.isReleasing = false;
        voice.isSustained = false;
        voice.voiceId = generateVoiceId();
        
        // Retrigger ADSR envelopes
        voice.amplitudeEnvelope.trigger();
        if (voice.filterEnvelope) {
            voice.filterEnvelope->trigger();
        }
        if (voice.pitchEnvelope) {
            voice.pitchEnvelope->trigger();
        }
        
        Logger::debug("VoiceAllocator: Retriggering voice {}", existingVoice);
        return existingVoice;
    }
    
    // Find available voice
    int voiceIndex = findAvailableVoice();
    if (voiceIndex == -1) {
        // No available voice, steal one
        voiceIndex = stealVoice();
        if (voiceIndex == -1) {
            Logger::warn("VoiceAllocator: Failed to allocate voice for note {}", note);
            return -1;
        }
        voiceStealCount_++;
        Logger::debug("VoiceAllocator: Stole voice {} for note {}", voiceIndex, note);
    }
    
    // Allocate voice
    Voice& voice = voices_[voiceIndex];
    voice.note = note;
    voice.velocity = velocity;
    voice.channel = channel;
    voice.isActive = true;
    voice.isSustained = false;
    voice.isReleasing = false;
    voice.triggerTime = std::chrono::steady_clock::now();
    voice.voiceId = generateVoiceId();
    
    // Trigger ADSR envelopes
    voice.amplitudeEnvelope.trigger();
    if (voice.filterEnvelope) {
        voice.filterEnvelope->trigger();
    }
    if (voice.pitchEnvelope) {
        voice.pitchEnvelope->trigger();
    }
    
    Logger::debug("VoiceAllocator: Allocated voice {} for note {}", voiceIndex, note);
    return voiceIndex;
}

int VoiceAllocator::noteOff(int note, int channel) {
    Logger::debug("VoiceAllocator: Note OFF - Note: {}, Ch: {}", note, channel);
    
    int voiceIndex = findVoiceForNote(note, channel);
    if (voiceIndex == -1) {
        Logger::debug("VoiceAllocator: Note {} not found for note off", note);
        return -1;
    }
    
    Voice& voice = voices_[voiceIndex];
    
    // Check if sustain pedal is pressed
    if (sustainEnabled_ && sustainPedalPressed_[channel]) {
        // Mark as sustained instead of releasing
        voice.isActive = false;     // Key is no longer pressed
        voice.isSustained = true;   // But held by sustain pedal
        voice.isReleasing = false;  // Not releasing, sustained
        Logger::debug("VoiceAllocator: Voice {} sustained (pedal pressed)", voiceIndex);
    } else {
        // Release the voice
        voice.isActive = false;     // Key is no longer pressed
        voice.isReleasing = true;   // Voice is releasing
        voice.releaseTime = std::chrono::steady_clock::now();
        
        // Release ADSR envelopes
        voice.amplitudeEnvelope.release();
        if (voice.filterEnvelope) {
            voice.filterEnvelope->release();
        }
        if (voice.pitchEnvelope) {
            voice.pitchEnvelope->release();
        }
        
        Logger::debug("VoiceAllocator: Voice {} released", voiceIndex);
    }
    
    return voiceIndex;
}

void VoiceAllocator::setSustainPedal(int value, int channel) {
    bool newState = value >= 64;
    bool oldState = sustainPedalPressed_[channel];
    
    sustainPedalPressed_[channel] = newState;
    
    Logger::debug("VoiceAllocator: Sustain pedal {} on channel {}", newState ? "ON" : "OFF", channel);
    
    // If sustain pedal was released, check if any sustain pedal is still pressed before releasing
    if (oldState && !newState) {
        // Check if ANY sustain pedal is still pressed on any channel (cross-channel support)
        bool anyChannelSustained = false;
        for (int ch = 0; ch < 16; ++ch) {
            if (sustainPedalPressed_[ch]) {
                anyChannelSustained = true;
                break;
            }
        }
        
        // Only release sustained voices if NO sustain pedal is pressed on any channel
        if (!anyChannelSustained) {
            Logger::debug("VoiceAllocator: No sustain pedal pressed on any channel, releasing all sustained voices");
            releaseSustainedVoicesGlobal();
        } else {
            Logger::debug("VoiceAllocator: Sustain pedal still pressed on another channel, keeping sustained voices");
        }
    }
}

void VoiceAllocator::allNotesOff(int channel) {
    Logger::info("VoiceAllocator: All notes off on channel {}", channel);
    
    for (int i = 0; i < maxVoices_; ++i) {
        Voice& voice = voices_[i];
        if (voice.isActive && voice.channel == channel) {
            voice.isReleasing = true;
            voice.releaseTime = std::chrono::steady_clock::now();
        }
    }
}

void VoiceAllocator::allSoundOff(int channel) {
    Logger::info("VoiceAllocator: All sound off on channel {}", channel);
    
    for (int i = 0; i < maxVoices_; ++i) {
        Voice& voice = voices_[i];
        if (voice.isActive && voice.channel == channel) {
            voice.reset();
        }
    }
}

const Voice& VoiceAllocator::getVoice(int voiceIndex) const {
    if (voiceIndex < 0 || voiceIndex >= maxVoices_) {
        static Voice invalidVoice;
        Logger::error("VoiceAllocator: Invalid voice index {}", voiceIndex);
        return invalidVoice;
    }
    return voices_[voiceIndex];
}

Voice& VoiceAllocator::getVoiceRef(int voiceIndex) {
    if (voiceIndex < 0 || voiceIndex >= maxVoices_) {
        static Voice invalidVoice;
        Logger::error("VoiceAllocator: Invalid voice index {}", voiceIndex);
        return invalidVoice;
    }
    return voices_[voiceIndex];
}

int VoiceAllocator::findVoiceForNote(int note, int channel) const {
    for (int i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        if (voice.isActive && voice.note == note && 
            (channel == -1 || voice.channel == channel)) {
            return i;
        }
    }
    return -1;
}

std::vector<int> VoiceAllocator::getActiveVoices() const {
    std::vector<int> activeVoices;
    for (int i = 0; i < maxVoices_; ++i) {
        if (voices_[i].isInUse()) {
            activeVoices.push_back(i);
        }
    }
    return activeVoices;
}

int VoiceAllocator::getActiveVoiceCount() const {
    int count = 0;
    for (int i = 0; i < maxVoices_; ++i) {
        if (voices_[i].isInUse()) {
            count++;
        }
    }
    return count;
}

void VoiceAllocator::markVoiceFinished(int voiceIndex) {
    if (voiceIndex >= 0 && voiceIndex < maxVoices_) {
        voices_[voiceIndex].reset();
        Logger::debug("VoiceAllocator: Voice {} marked as finished", voiceIndex);
    }
}

void VoiceAllocator::markVoiceAsSustained(int voiceIndex, int note, int channel) {
    if (voiceIndex >= 0 && voiceIndex < maxVoices_) {
        Voice& voice = voices_[voiceIndex];
        // Mark voice as sustained but keep it playing
        voice.isActive = false;     // Key is no longer pressed
        voice.isSustained = true;   // But held by sustain pedal
        voice.isReleasing = false;  // Not releasing yet
        Logger::debug("VoiceAllocator: Voice {} marked as sustained for note {} on channel {}", 
                     voiceIndex, note, channel);
    }
}

void VoiceAllocator::setMaxVoices(int maxVoices) {
    if (maxVoices < 1 || maxVoices > 128) {
        Logger::error("VoiceAllocator: Invalid max voices: {}", maxVoices);
        return;
    }
    
    int oldMaxVoices = maxVoices_;
    maxVoices_ = maxVoices;
    
    // Resize voice pool
    voices_.resize(maxVoices_);
    
    // Initialize new voices if we increased the count
    for (int i = oldMaxVoices; i < maxVoices_; ++i) {
        voices_[i].reset();
    }
    
    Logger::info("VoiceAllocator: Max voices changed from {} to {}", oldMaxVoices, maxVoices_);
}

void VoiceAllocator::reset() {
    Logger::info("VoiceAllocator: Resetting all voices");
    
    for (auto& voice : voices_) {
        voice.reset();
    }
    
    std::fill(sustainPedalPressed_.begin(), sustainPedalPressed_.end(), false);
    voiceStealCount_ = 0;
}

bool VoiceAllocator::isSustainPedalPressed(int channel) const {
    if (channel < 0 || channel >= 16) {
        return false;
    }
    return sustainPedalPressed_[channel];
}

void VoiceAllocator::printVoiceState() const {
    Logger::info("=== Voice Allocator State ===");
    Logger::info("Max Voices: {}, Active: {}, Steals: {}", 
                 maxVoices_, getActiveVoiceCount(), voiceStealCount_);
    
    for (int i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        if (voice.isInUse()) {
            Logger::info("Voice {}: Note={}, Vel={}, Ch={}, Active={}, Sustained={}, Releasing={}", 
                        i, voice.note, voice.velocity, voice.channel, 
                        voice.isActive, voice.isSustained, voice.isReleasing);
        }
    }
    
    Logger::info("Sustain Pedals: ");
    for (int ch = 0; ch < 16; ++ch) {
        if (sustainPedalPressed_[ch]) {
            Logger::info("  Channel {}: ON", ch);
        }
    }
    Logger::info("=============================");
}

// =========================
// Private Methods
// =========================

int VoiceAllocator::findAvailableVoice() {
    for (int i = 0; i < maxVoices_; ++i) {
        if (!voices_[i].isInUse()) {
            return i;
        }
    }
    return -1;
}

int VoiceAllocator::stealVoice() {
    switch (stealingMode_) {
        case VoiceStealingMode::OLDEST:
            return findOldestVoice();
        case VoiceStealingMode::LOWEST_VELOCITY:
            return findLowestVelocityVoice();
        case VoiceStealingMode::HIGHEST_NOTE:
            return findHighestNoteVoice();
        case VoiceStealingMode::LOWEST_NOTE:
            return findLowestNoteVoice();
        default:
            return findOldestVoice();
    }
}

int VoiceAllocator::findOldestVoice() const {
    int oldestVoice = -1;
    auto oldestTime = std::chrono::steady_clock::now();
    
    for (int i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        if (voice.isInUse() && !voice.isSustained) {
            if (oldestVoice == -1 || voice.triggerTime < oldestTime) {
                oldestVoice = i;
                oldestTime = voice.triggerTime;
            }
        }
    }
    
    return oldestVoice;
}

int VoiceAllocator::findLowestVelocityVoice() const {
    int lowestVoice = -1;
    int lowestVelocity = 128;
    
    for (int i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        if (voice.isInUse() && !voice.isSustained) {
            if (voice.velocity < lowestVelocity) {
                lowestVoice = i;
                lowestVelocity = voice.velocity;
            }
        }
    }
    
    return lowestVoice;
}

int VoiceAllocator::findHighestNoteVoice() const {
    int highestVoice = -1;
    int highestNote = -1;
    
    for (int i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        if (voice.isInUse() && !voice.isSustained) {
            if (voice.note > highestNote) {
                highestVoice = i;
                highestNote = voice.note;
            }
        }
    }
    
    return highestVoice;
}

int VoiceAllocator::findLowestNoteVoice() const {
    int lowestVoice = -1;
    int lowestNote = 128;
    
    for (int i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        if (voice.isInUse() && !voice.isSustained) {
            if (voice.note < lowestNote) {
                lowestVoice = i;
                lowestNote = voice.note;
            }
        }
    }
    
    return lowestVoice;
}

void VoiceAllocator::releaseSustainedVoices(int channel) {
    Logger::debug("VoiceAllocator: Releasing sustained voices on channel {}", channel);
    
    for (int i = 0; i < maxVoices_; ++i) {
        Voice& voice = voices_[i];
        if (voice.isSustained && voice.channel == channel) {
            voice.isSustained = false;
            voice.isReleasing = true;
            voice.releaseTime = std::chrono::steady_clock::now();
            
            // Release ADSR envelopes
            voice.amplitudeEnvelope.release();
            if (voice.filterEnvelope) {
                voice.filterEnvelope->release();
            }
            if (voice.pitchEnvelope) {
                voice.pitchEnvelope->release();
            }
        }
    }
}

void VoiceAllocator::releaseSustainedVoicesGlobal() {
    Logger::debug("VoiceAllocator: Releasing all sustained voices globally");
    
    for (int i = 0; i < maxVoices_; ++i) {
        Voice& voice = voices_[i];
        if (voice.isSustained) {
            Logger::debug("VoiceAllocator: Releasing sustained voice {} on channel {}", i, voice.channel);
            voice.isSustained = false;
            voice.isReleasing = true;
            voice.releaseTime = std::chrono::steady_clock::now();
            
            // Release ADSR envelopes
            voice.amplitudeEnvelope.release();
            if (voice.filterEnvelope) {
                voice.filterEnvelope->release();
            }
            if (voice.pitchEnvelope) {
                voice.pitchEnvelope->release();
            }
        }
    }
}

// =========================
// ADSR Configuration Methods
// =========================

void VoiceAllocator::setAmplitudeADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime) {
    Logger::debug("VoiceAllocator: Setting amplitude ADSR - A:{:.3f}s D:{:.3f}s S:{:.3f} R:{:.3f}s", 
                 attackTime, decayTime, sustainLevel, releaseTime);
    
    for (auto& voice : voices_) {
        voice.amplitudeEnvelope.setParameters(attackTime, decayTime, sustainLevel, releaseTime);
    }
}

void VoiceAllocator::setAmplitudeADSRCurve(double curve) {
    Logger::debug("VoiceAllocator: Setting amplitude ADSR curve to {:.3f}", curve);
    
    for (auto& voice : voices_) {
        voice.amplitudeEnvelope.setCurve(curve);
    }
}

void VoiceAllocator::enableFilterEnvelopes() {
    Logger::debug("VoiceAllocator: Enabling filter envelopes for all voices");
    
    for (int i = 0; i < maxVoices_; ++i) {
        voices_[i].enableFilterEnvelope(i);
    }
}

void VoiceAllocator::setFilterADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime) {
    Logger::debug("VoiceAllocator: Setting filter ADSR - A:{:.3f}s D:{:.3f}s S:{:.3f} R:{:.3f}s", 
                 attackTime, decayTime, sustainLevel, releaseTime);
    
    for (auto& voice : voices_) {
        if (voice.filterEnvelope) {
            voice.filterEnvelope->setParameters(attackTime, decayTime, sustainLevel, releaseTime);
        }
    }
}

void VoiceAllocator::enablePitchEnvelopes() {
    Logger::debug("VoiceAllocator: Enabling pitch envelopes for all voices");
    
    for (int i = 0; i < maxVoices_; ++i) {
        voices_[i].enablePitchEnvelope(i);
    }
}

void VoiceAllocator::setPitchADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime) {
    Logger::debug("VoiceAllocator: Setting pitch ADSR - A:{:.3f}s D:{:.3f}s S:{:.3f} R:{:.3f}s", 
                 attackTime, decayTime, sustainLevel, releaseTime);
    
    for (auto& voice : voices_) {
        if (voice.pitchEnvelope) {
            voice.pitchEnvelope->setParameters(attackTime, decayTime, sustainLevel, releaseTime);
        }
    }
}

void VoiceAllocator::initializeEnvelopes(double sampleRate) {
    Logger::debug("VoiceAllocator: Initializing all envelopes with sample rate {:.1f} Hz", sampleRate);
    
    for (auto& voice : voices_) {
        voice.initializeEnvelopes(sampleRate);
    }
}
