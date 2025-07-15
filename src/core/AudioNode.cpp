#include "AudioNode.h"
#include <algorithm>
#include <cstring>

AudioNode::AudioNode(const std::string& name) : name(name) {
}

void AudioNode::prepare(const PrepareInfo& info) {
    currentPrepareInfo = info;
    prepared = true;
}

// New choc buffer helper methods
void AudioNode::copyBuffer(choc::buffer::ChannelArrayView<const float> source, choc::buffer::ChannelArrayView<float> destination) {
    CHOC_ASSERT(source.getNumChannels() <= destination.getNumChannels());
    CHOC_ASSERT(source.getNumFrames() == destination.getNumFrames());
    
    auto numChannelsToCopy = std::min(source.getNumChannels(), destination.getNumChannels());
    auto numFrames = source.getNumFrames();
    
    for (choc::buffer::ChannelCount ch = 0; ch < numChannelsToCopy; ++ch) {
        for (choc::buffer::FrameCount frame = 0; frame < numFrames; ++frame) {
            destination.getSample(ch, frame) = source.getSample(ch, frame);
        }
    }
    
    // Clear any extra destination channels
    for (choc::buffer::ChannelCount ch = numChannelsToCopy; ch < destination.getNumChannels(); ++ch) {
        for (choc::buffer::FrameCount frame = 0; frame < numFrames; ++frame) {
            destination.getSample(ch, frame) = 0.0f;
        }
    }
}

void AudioNode::addToBuffer(choc::buffer::ChannelArrayView<const float> source, choc::buffer::ChannelArrayView<float> destination) {
    CHOC_ASSERT(source.getNumChannels() <= destination.getNumChannels());
    CHOC_ASSERT(source.getNumFrames() == destination.getNumFrames());
    
    auto numChannelsToAdd = std::min(source.getNumChannels(), destination.getNumChannels());
    
    for (choc::buffer::ChannelCount ch = 0; ch < numChannelsToAdd; ++ch) {
        for (choc::buffer::FrameCount frame = 0; frame < source.getNumFrames(); ++frame) {
            destination.getSample(ch, frame) += source.getSample(ch, frame);
        }
    }
}

void AudioNode::clearBuffer(choc::buffer::ChannelArrayView<float> buffer) {
    buffer.clear();
}

void AudioNode::scaleBuffer(choc::buffer::ChannelArrayView<float> buffer, float gain) {
    for (choc::buffer::ChannelCount ch = 0; ch < buffer.getNumChannels(); ++ch) {
        for (choc::buffer::FrameCount frame = 0; frame < buffer.getNumFrames(); ++frame) {
            buffer.getSample(ch, frame) *= gain;
        }
    }
}

// Legacy helper methods for single channel operations
void AudioNode::copyBuffer(const float* source, float* destination, int numSamples) {
    std::memcpy(destination, source, numSamples * sizeof(float));
}

void AudioNode::addToBuffer(const float* source, float* destination, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        destination[i] += source[i];
    }
}

void AudioNode::clearBuffer(float* buffer, int numSamples) {
    std::memset(buffer, 0, numSamples * sizeof(float));
}

void AudioNode::scaleBuffer(float* buffer, float gain, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] *= gain;
    }
}