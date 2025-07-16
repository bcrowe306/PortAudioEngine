#include "SamplePlayerNode.h"
#include "ADSR.h"
#include <algorithm>
#include <cstring>

SamplePlayerNode::SamplePlayerNode(const std::string& name)
    : AudioNode(name)
{
    Logger::info("SamplePlayerNode '{}' created", name);
}

void SamplePlayerNode::prepare(const PrepareInfo& info) {
    engineSampleRate_ = info.sampleRate;
    maxBlockSize_ = info.maxBufferSize;
    
    Logger::debug("SamplePlayerNode '{}' prepared: SR={} Hz, MaxBlock={}", 
                 getName(), engineSampleRate_, maxBlockSize_);
    
    // Update playback rate in case sample rate changed
    updatePlaybackRate();
}

void SamplePlayerNode::processCallback(choc::buffer::ChannelArrayView<const float> input,
                                      choc::buffer::ChannelArrayView<float> output,
                                      double sampleRate,
                                      int numSamples) {
    // Clear output first
    output.clear();
    
    // Check if we have a sample loaded and are playing
    if (!hasSample()) {
        if (playbackState_ == PlaybackState::PLAYING) {
            Logger::debug("SamplePlayerNode '{}': Stopping - no sample loaded", getName());
            stop();
        }
        return;
    }
    
    if (playbackState_ != PlaybackState::PLAYING) {
        return;
    }
    
    const int outputChannels = static_cast<int>(output.getNumChannels());
    const int sampleChannels = static_cast<int>(sampleBuffer_.getNumChannels());
    const int totalSamples = getTotalSamples();
    
    // Calculate effective sample region
    int effectiveStart = startSample_;
    int effectiveEnd = (endSample_ > 0) ? std::min(endSample_, totalSamples) : totalSamples;
    
    // Logger::debug("SamplePlayerNode '{}': Processing - pos: {}, start: {}, end: {}, total: {}", 
    //              getName(), static_cast<int>(playPosition_), effectiveStart, effectiveEnd, totalSamples);
    
    if (effectiveStart >= effectiveEnd) {
        // Logger::debug("SamplePlayerNode '{}': Stopping - invalid region: {} >= {}", 
        //              getName(), effectiveStart, effectiveEnd);
        stop();
        return;
    }
    
    // Process samples
    for (int i = 0; i < numSamples; ++i) {
        // Check if we've reached the end
        if (playPosition_ >= effectiveEnd) {
            if (loop_) {
                handleLooping();
            } else {
                stop();
                break;
            }
        }
        
        // Check if we're still in valid range
        if (playPosition_ < effectiveStart || playPosition_ >= effectiveEnd) {
            stop();
            break;
        }
        
        // Get interpolated sample for each output channel
        for (int ch = 0; ch < outputChannels; ++ch) {
            float sample = 0.0f;
            
            if (sampleChannels == 1) {
                // Mono sample - use for all output channels
                sample = getSampleInterpolated(0, playPosition_);
            } else if (ch < sampleChannels) {
                // Multi-channel sample - use corresponding channel
                sample = getSampleInterpolated(ch, playPosition_);
            } else {
                // More output channels than sample channels - repeat last sample channel
                sample = getSampleInterpolated(sampleChannels - 1, playPosition_);
            }
            
            // Apply gain and volume
            sample *= gain_ * volume_;
            
            // Apply amplitude envelope if available
            if (amplitudeEnvelope_) {
                sample *= static_cast<float>(amplitudeEnvelope_->getCurrentValue());
            }
            
            output.getSample(static_cast<choc::buffer::ChannelCount>(ch), 
                           static_cast<choc::buffer::FrameCount>(i)) = sample;
        }
        
        // Advance playback position
        playPosition_ += playbackRate_;
    }
    
    // Update audio analysis
    updateAnalysis(output);
}

bool SamplePlayerNode::loadSample(const std::string& filePath) {
    try {
        choc::audio::WAVAudioFileFormat<false> wavFormat;
        auto reader = wavFormat.createReader(filePath);
        
        if (!reader) {
            Logger::error("SamplePlayerNode '{}': Failed to create reader for file: {}", 
                         getName(), filePath);
            return false;
        }
        
        // Load the entire file
        auto data = reader->loadFileContent();
        
        if (data.frames.getNumFrames() == 0) {
            Logger::error("SamplePlayerNode '{}': No audio data in file: {}", 
                         getName(), filePath);
            return false;
        }
        
        // Store the sample data
        sampleBuffer_ = std::move(data.frames);
        sampleSampleRate_ = data.sampleRate;
        loadedFilePath_ = filePath;
        
        // Initialize sample region to full sample
        startSample_ = 0;
        endSample_ = getTotalSamples();
        
        // Initialize loop region to full sample
        loopStart_ = startSample_;
        loopEnd_ = endSample_;
        
        // Reset playback position
        playPosition_ = startSample_;
        
        // Update playback rate for current note
        updatePlaybackRate();
        
        Logger::info("SamplePlayerNode '{}': Loaded sample '{}' - {} channels, {} samples, {:.1f} Hz", 
                    getName(), filePath, getNumChannels(), getTotalSamples(), sampleSampleRate_);
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("SamplePlayerNode '{}': Exception loading file '{}': {}", 
                     getName(), filePath, e.what());
        return false;
    }
}

bool SamplePlayerNode::loadSample(const choc::buffer::ChannelArrayBuffer<float>& buffer, double sampleRate) {
    if (buffer.getNumFrames() == 0) {
        Logger::error("SamplePlayerNode '{}': Cannot load empty buffer", getName());
        return false;
    }
    
    // Copy the buffer
    sampleBuffer_ = buffer;
    sampleSampleRate_ = sampleRate;
    loadedFilePath_ = "<buffer>";
    
    // Initialize sample region to full sample
    startSample_ = 0;
    endSample_ = getTotalSamples();
    
    // Initialize loop region to full sample
    loopStart_ = startSample_;
    loopEnd_ = endSample_;
    
    // Reset playback position
    playPosition_ = startSample_;
    
    // Update playback rate for current note
    updatePlaybackRate();
    
    Logger::info("SamplePlayerNode '{}': Loaded buffer - {} channels, {} samples, {:.1f} Hz", 
                getName(), getNumChannels(), getTotalSamples(), sampleSampleRate_);
    
    return true;
}

void SamplePlayerNode::unloadSample() {
    stop();
    sampleBuffer_ = choc::buffer::ChannelArrayBuffer<float>();
    sampleSampleRate_ = 44100.0;
    loadedFilePath_.clear();
    
    Logger::info("SamplePlayerNode '{}': Sample unloaded", getName());
}

void SamplePlayerNode::play() {
    if (!hasSample()) {
        Logger::warn("SamplePlayerNode '{}': Cannot play - no sample loaded", getName());
        return;
    }
    
    playbackState_ = PlaybackState::PLAYING;
    Logger::debug("SamplePlayerNode '{}': Started playing", getName());
}

void SamplePlayerNode::stop() {
    playbackState_ = PlaybackState::STOPPED;
    playPosition_ = startSample_;
    Logger::debug("SamplePlayerNode '{}': Stopped", getName());
}

void SamplePlayerNode::pause() {
    if (playbackState_ == PlaybackState::PLAYING) {
        playbackState_ = PlaybackState::PAUSED;
        Logger::debug("SamplePlayerNode '{}': Paused", getName());
    }
}

void SamplePlayerNode::resume() {
    if (playbackState_ == PlaybackState::PAUSED) {
        playbackState_ = PlaybackState::PLAYING;
        Logger::debug("SamplePlayerNode '{}': Resumed", getName());
    }
}

void SamplePlayerNode::trigger() {
    if (!hasSample()) {
        Logger::warn("SamplePlayerNode '{}': Cannot trigger - no sample loaded", getName());
        return;
    }
    
    playPosition_ = startSample_;
    playbackState_ = PlaybackState::PLAYING;
    Logger::debug("SamplePlayerNode '{}': Triggered - pos: {}, region: {}-{}, total: {}", 
                 getName(), static_cast<int>(playPosition_), startSample_, endSample_, getTotalSamples());
}

void SamplePlayerNode::trigger(int midiNote) {
    setCurrentNote(midiNote);
    trigger();
    Logger::debug("SamplePlayerNode '{}': Triggered with MIDI note {} (region: {}-{}, total: {})", 
                 getName(), midiNote, startSample_, endSample_, getTotalSamples());
}

void SamplePlayerNode::setStartSample(int startSample) {
    startSample_ = std::max(0, std::min(startSample, getTotalSamples() - 1));
    
    // Ensure end sample is after start sample
    if (endSample_ <= startSample_) {
        endSample_ = getTotalSamples();
    }
    
    Logger::debug("SamplePlayerNode '{}': Start sample set to {}", getName(), startSample_);
}

void SamplePlayerNode::setEndSample(int endSample) {
    if (endSample <= 0) {
        endSample_ = getTotalSamples();
    } else {
        endSample_ = std::max(startSample_ + 1, std::min(endSample, getTotalSamples()));
    }
    
    Logger::debug("SamplePlayerNode '{}': End sample set to {}", getName(), endSample_);
}

void SamplePlayerNode::setSampleRegion(int startSample, int endSample) {
    setStartSample(startSample);
    setEndSample(endSample);
}

void SamplePlayerNode::setLoopStart(int loopStart) {
    loopStart_ = std::max(startSample_, std::min(loopStart, endSample_ - 1));
    
    // Ensure loop end is after loop start
    if (loopEnd_ <= loopStart_) {
        loopEnd_ = endSample_;
    }
    
    Logger::debug("SamplePlayerNode '{}': Loop start set to {}", getName(), loopStart_);
}

void SamplePlayerNode::setLoopEnd(int loopEnd) {
    if (loopEnd <= 0) {
        loopEnd_ = endSample_;
    } else {
        loopEnd_ = std::max(loopStart_ + 1, std::min(loopEnd, endSample_));
    }
    
    Logger::debug("SamplePlayerNode '{}': Loop end set to {}", getName(), loopEnd_);
}

void SamplePlayerNode::setLoopRegion(int loopStart, int loopEnd) {
    setLoopStart(loopStart);
    setLoopEnd(loopEnd);
}

void SamplePlayerNode::setPlayPosition(double position) {
    position = std::max(0.0, std::min(position, 1.0));
    int effectiveEnd = (endSample_ > 0) ? endSample_ : getTotalSamples();
    playPosition_ = startSample_ + position * (effectiveEnd - startSample_);
}

double SamplePlayerNode::getPlayPosition() const {
    if (!hasSample()) return 0.0;
    
    int effectiveEnd = (endSample_ > 0) ? endSample_ : getTotalSamples();
    if (effectiveEnd <= startSample_) return 0.0;
    
    return (playPosition_ - startSample_) / (effectiveEnd - startSample_);
}

void SamplePlayerNode::setPlayPositionSamples(int samples) {
    playPosition_ = clampSamplePosition(samples);
}

double SamplePlayerNode::getDurationSeconds() const {
    if (!hasSample() || sampleSampleRate_ <= 0.0) return 0.0;
    
    int effectiveEnd = (endSample_ > 0) ? endSample_ : getTotalSamples();
    return (effectiveEnd - startSample_) / sampleSampleRate_;
}

void SamplePlayerNode::printSampleInfo() const {
    if (!hasSample()) {
        Logger::info("SamplePlayerNode '{}': No sample loaded", getName());
        return;
    }
    
    Logger::info("=== SamplePlayerNode '{}' Info ===", getName());
    Logger::info("File: {}", loadedFilePath_);
    Logger::info("Channels: {}, Samples: {}, Duration: {:.2f}s", 
                getNumChannels(), getTotalSamples(), getDurationSeconds());
    Logger::info("Sample Rate: {:.1f} Hz, Engine Rate: {:.1f} Hz", 
                sampleSampleRate_, engineSampleRate_);
    Logger::info("Sample Region: {} - {} ({} samples)", 
                startSample_, endSample_, getSampleLength());
    Logger::info("Loop: {}, Loop Region: {} - {} ({} samples)", 
                loop_ ? "ON" : "OFF", loopStart_, loopEnd_, loopEnd_ - loopStart_);
    Logger::info("Base Note: {}, Current Note: {}, Transpose: {}, Detune: {:.1f}c", 
                baseNote_, currentNote_, transpose_, detune_);
    Logger::info("Playback Rate: {:.3f}, Position: {:.2f}%", 
                playbackRate_, getPlayPosition() * 100.0);
    Logger::info("State: {}, Gain: {:.2f}, Volume: {:.2f}", 
                (playbackState_ == PlaybackState::PLAYING ? "PLAYING" : 
                 playbackState_ == PlaybackState::PAUSED ? "PAUSED" : "STOPPED"),
                gain_, volume_);
    Logger::info("Interpolation: {}", 
                (interpolationMode_ == InterpolationMode::NONE ? "NONE" :
                 interpolationMode_ == InterpolationMode::LINEAR ? "LINEAR" : "CUBIC"));
    Logger::info("=====================================");
}

// =========================
// Private Methods
// =========================

void SamplePlayerNode::updatePlaybackRate() {
    if (useManualRate_) {
        playbackRate_ = manualPlaybackRate_;
    } else {
        // Calculate playback rate based on note difference
        double noteRatio = noteToFrequencyRatio(baseNote_ + transpose_, currentNote_);
        
        // Apply detune (cents)
        double detuneRatio = std::pow(2.0, detune_ / 1200.0);
        
        // Apply sample rate ratio
        double sampleRateRatio = sampleSampleRate_ / engineSampleRate_;
        
        playbackRate_ = noteRatio * detuneRatio * sampleRateRatio;
    }
    
    Logger::debug("SamplePlayerNode '{}': Playback rate updated to {:.3f}", getName(), playbackRate_);
}

float SamplePlayerNode::getSampleInterpolated(int channel, double position) const {
    if (!isValidSamplePosition(position) || channel < 0 || channel >= getNumChannels()) {
        return 0.0f;
    }
    
    switch (interpolationMode_) {
        case InterpolationMode::NONE:
            return sampleBuffer_.getSample(static_cast<choc::buffer::ChannelCount>(channel), 
                                         static_cast<choc::buffer::FrameCount>(static_cast<int>(position)));
        case InterpolationMode::LINEAR:
            return getSampleLinear(channel, position);
        case InterpolationMode::CUBIC:
            return getSampleCubic(channel, position);
        default:
            return getSampleLinear(channel, position);
    }
}

float SamplePlayerNode::getSampleLinear(int channel, double position) const {
    int index = static_cast<int>(position);
    double fraction = position - index;
    
    // Clamp indices
    int index1 = clampSamplePosition(index);
    int index2 = clampSamplePosition(index + 1);
    
    float sample1 = sampleBuffer_.getSample(static_cast<choc::buffer::ChannelCount>(channel), 
                                          static_cast<choc::buffer::FrameCount>(index1));
    float sample2 = sampleBuffer_.getSample(static_cast<choc::buffer::ChannelCount>(channel), 
                                          static_cast<choc::buffer::FrameCount>(index2));
    
    return sample1 + static_cast<float>(fraction) * (sample2 - sample1);
}

float SamplePlayerNode::getSampleCubic(int channel, double position) const {
    int index = static_cast<int>(position);
    double fraction = position - index;
    
    // Get 4 sample points for cubic interpolation
    int index0 = clampSamplePosition(index - 1);
    int index1 = clampSamplePosition(index);
    int index2 = clampSamplePosition(index + 1);
    int index3 = clampSamplePosition(index + 2);
    
    float y0 = sampleBuffer_.getSample(static_cast<choc::buffer::ChannelCount>(channel), 
                                     static_cast<choc::buffer::FrameCount>(index0));
    float y1 = sampleBuffer_.getSample(static_cast<choc::buffer::ChannelCount>(channel), 
                                     static_cast<choc::buffer::FrameCount>(index1));
    float y2 = sampleBuffer_.getSample(static_cast<choc::buffer::ChannelCount>(channel), 
                                     static_cast<choc::buffer::FrameCount>(index2));
    float y3 = sampleBuffer_.getSample(static_cast<choc::buffer::ChannelCount>(channel), 
                                     static_cast<choc::buffer::FrameCount>(index3));
    
    // Cubic interpolation (Catmull-Rom)
    float a = static_cast<float>(fraction);
    float a2 = a * a;
    float a3 = a2 * a;
    
    return y1 + 0.5f * a * (y2 - y0) + 
           0.5f * a2 * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3) + 
           0.5f * a3 * (-y0 + 3.0f * y1 - 3.0f * y2 + y3);
}

void SamplePlayerNode::handleLooping() {
    if (!loop_) return;
    
    int effectiveLoopStart = std::max(loopStart_, startSample_);
    int effectiveLoopEnd = (loopEnd_ > 0) ? std::min(loopEnd_, endSample_) : endSample_;
    
    if (effectiveLoopEnd <= effectiveLoopStart) {
        // Invalid loop region, stop playing
        stop();
        return;
    }
    
    // Wrap around to loop start
    while (playPosition_ >= effectiveLoopEnd) {
        playPosition_ -= (effectiveLoopEnd - effectiveLoopStart);
    }
    
    // Ensure we're not before loop start
    if (playPosition_ < effectiveLoopStart) {
        playPosition_ = effectiveLoopStart;
    }
}

void SamplePlayerNode::updateAnalysis(const choc::buffer::ChannelArrayView<float>& output) {
    if (output.getNumFrames() == 0) return;
    
    float peak = 0.0f;
    float rms = 0.0f;
    int totalSamples = 0;
    
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
    
    // Simple exponential smoothing
    const float alpha = 0.1f;
    peakLevel_ = peak * alpha + peakLevel_ * (1.0f - alpha);
    rmsLevel_ = rms * alpha + rmsLevel_ * (1.0f - alpha);
}

double SamplePlayerNode::noteToFrequencyRatio(int noteA, int noteB) const {
    // Calculate frequency ratio between two MIDI notes
    return std::pow(2.0, (noteB - noteA) / 12.0);
}

int SamplePlayerNode::clampSamplePosition(int position) const {
    return std::max(0, std::min(position, getTotalSamples() - 1));
}

bool SamplePlayerNode::isValidSamplePosition(double position) const {
    return position >= 0.0 && position < getTotalSamples();
}
