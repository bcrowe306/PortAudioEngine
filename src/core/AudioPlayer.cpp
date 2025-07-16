#include "AudioPlayer.h"
#include "Logger.h"
#include <algorithm>
#include <cstring>
#include <iostream>

AudioPlayer::AudioPlayer(const std::string& name) : AudioNode(name) {
}

void AudioPlayer::processCallback(
    choc::buffer::ChannelArrayView<const float> inputBuffers,
    choc::buffer::ChannelArrayView<float> outputBuffers,
    double sampleRate,
    int blockSize) 
{
    auto numOutputChannels = outputBuffers.getNumChannels();
    auto numSamples = outputBuffers.getNumFrames();
    
    // Clear all output buffers first
    outputBuffers.clear();
    
    if (!playing.load() || audioData.getSize().isEmpty()) {
        return;
    }
    
    size_t currentPos = playPosition.load();
    size_t dataSize = audioData.getNumFrames();
    auto audioChannels = audioData.getNumChannels();
    
    for (choc::buffer::FrameCount sample = 0; sample < numSamples; ++sample) {
        if (currentPos >= endPosition.load()) {
            if (loop) {
                currentPos = startPosition.load(); // Loop back to start position
            } else {
                playing.store(false); // Stop playing
                currentPos = startPosition.load(); // Reset position
                break;
            }
        }
        
        if (currentPos < dataSize) {
            // Process each output channel
            for (choc::buffer::ChannelCount outCh = 0; outCh < numOutputChannels; ++outCh) {
                float sampleValue = 0.0f;
                
                if (audioChannels == 1) {
                    // Mono: use the same sample for all output channels
                    sampleValue = audioData.getSample(0, static_cast<choc::buffer::FrameCount>(currentPos));
                } else {
                    // Multi-channel: map audio channels to output channels
                    auto audioCh = outCh % audioChannels; // Wrap around if more output channels than audio channels
                    sampleValue = audioData.getSample(audioCh, static_cast<choc::buffer::FrameCount>(currentPos));
                }
                
                outputBuffers.getSample(outCh, sample) = sampleValue * gain;
            }
            
            currentPos++;
        }
    }
    
    playPosition.store(currentPos);
}

void AudioPlayer::loadData(const std::vector<float>& monoData) {
    Logger::debug("AudioPlayer::loadData() called with ", monoData.size(), " mono samples");
    stop();
    
    // Create a mono buffer from the vector data
    audioData = choc::buffer::ChannelArrayBuffer<float>(1, static_cast<choc::buffer::FrameCount>(monoData.size()));
    startPosition.store(0);
    endPosition.store(monoData.size());
    // Copy data into the buffer
    auto channelView = audioData.getChannel(0);
    for (size_t i = 0; i < monoData.size(); ++i) {
        channelView.getSample(0, static_cast<choc::buffer::FrameCount>(i)) = monoData[i];
    }
    
    reset();
    Logger::debug("  Data loaded, audioData frames = ", audioData.getNumFrames(), 
                  ", channels = ", audioData.getNumChannels());
}

void AudioPlayer::loadData(const choc::buffer::ChannelArrayBuffer<float>& audioBuffer) {
    Logger::debug("AudioPlayer::loadData() called with ChannelArrayBuffer - channels: ", 
                  audioBuffer.getNumChannels(), ", frames: ", audioBuffer.getNumFrames());
    stop();
    
    // Create a copy of the buffer
    audioData = choc::buffer::ChannelArrayBuffer<float>(audioBuffer);
    startPosition.store(0);
    endPosition.store(audioData.getNumFrames());
    
    reset();
    Logger::debug("  Data loaded, audioData frames = ", audioData.getNumFrames(), 
                  ", channels = ", audioData.getNumChannels());
}

void AudioPlayer::loadData(const float* const* channelData, int numChannels, int numFrames) {
    Logger::debug("AudioPlayer::loadData() called with channel array - channels: ", 
                  numChannels, ", frames: ", numFrames);
    stop();
    
    // Create buffer with the specified size
    audioData = choc::buffer::ChannelArrayBuffer<float>(static_cast<choc::buffer::ChannelCount>(numChannels), 
                                                        static_cast<choc::buffer::FrameCount>(numFrames));
    
    startPosition.store(0);
    endPosition.store(numFrames);
    // Copy data from channel arrays
    for (int ch = 0; ch < numChannels; ++ch) {
        auto channelView = audioData.getChannel(static_cast<choc::buffer::ChannelCount>(ch));
        for (int frame = 0; frame < numFrames; ++frame) {
            channelView.getSample(0, static_cast<choc::buffer::FrameCount>(frame)) = channelData[ch][frame];
        }
    }
    
    reset();
    Logger::debug("  Data loaded, audioData frames = ", audioData.getNumFrames(), 
                  ", channels = ", audioData.getNumChannels());
}

void AudioPlayer::play() {
    // Logger::debug("AudioPlayer::play() called - dataSize: ", audioData.getNumFrames());
    if (!audioData.getSize().isEmpty()) {
        playing.store(true);
        playPosition.store(startPosition.load()); // Reset position to start
        // Logger::debug("  Playing flag set to true");
    } else {
        Logger::warn("AudioPlayer::play() called but no data to play!");
    }
}

void AudioPlayer::stop() {
    playing.store(false);
}

void AudioPlayer::reset() {
    playPosition.store(0);
}

double AudioPlayer::getPlaybackProgress() const {
    if (audioData.getSize().isEmpty()) {
        return 0.0;
    }
    return static_cast<double>(playPosition.load()) / static_cast<double>(audioData.getNumFrames());
}
