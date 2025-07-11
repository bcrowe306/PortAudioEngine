#include "AudioNode.h"
#include <algorithm>
#include <cstring>

AudioNode::AudioNode(const std::string& name) : name(name) {
}

void AudioNode::prepare(const PrepareInfo& info) {
    currentPrepareInfo = info;
    prepared = true;
}

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