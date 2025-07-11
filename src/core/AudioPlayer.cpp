#include "AudioPlayer.h"
#include "Logger.h"
#include <algorithm>
#include <cstring>

AudioPlayer::AudioPlayer(const std::string& name) : AudioNode(name) {
}

void AudioPlayer::processCallback(
    const float* const* inputBuffers,
    float* const* outputBuffers,
    int numInputChannels,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize
) {
    // Clear all output buffers first
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        std::memset(outputBuffers[ch], 0, numSamples * sizeof(float));
    }
    
    if (!playing.load() || audioData.empty()) {
        return;
    }
    
    size_t currentPos = playPosition.load();
    size_t dataSize = audioData.size();
    
    for (int sample = 0; sample < numSamples; ++sample) {
        if (currentPos >= dataSize) {
            if (loop) {
                currentPos = 0; // Loop back to beginning
            } else {
                playing.store(false); // Stop playing
                break;
            }
        }
        
        if (currentPos < dataSize) {
            float sampleValue = audioData[currentPos] * gain;
            
            // Output to all channels (mono to stereo/multi-channel)
            for (int ch = 0; ch < numOutputChannels; ++ch) {
                outputBuffers[ch][sample] = sampleValue;
            }
            
            currentPos++;
        }
    }
    
    playPosition.store(currentPos);
}

void AudioPlayer::loadData(const std::vector<float>& data) {
    Logger::debug("AudioPlayer::loadData() called with ", data.size(), " samples");
    stop();
    audioData = data;
    reset();
    Logger::debug("  Data loaded, audioData.size() = ", audioData.size());
}

void AudioPlayer::play() {
    Logger::debug("AudioPlayer::play() called - dataSize: ", audioData.size());
    if (!audioData.empty()) {
        playing.store(true);
        Logger::debug("  Playing flag set to true");
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
    if (audioData.empty()) {
        return 0.0;
    }
    return static_cast<double>(playPosition.load()) / static_cast<double>(audioData.size());
}
