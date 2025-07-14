#include "core/AudioCore.h"
#include "test.h"
#include <iostream>

class CmajorTest : public AudioNode {
public:

    sine sine_generator;
    CmajorTest(const std::string& name = "CmajorTest") : AudioNode(name) {
        sine_generator.initialise(1, 44100.0);
    }

    void prepare(const PrepareInfo &info) override {
        std::cout << "CmajorTest::prepare() called with sample rate: " << info.sampleRate << std::endl;
        currentPrepareInfo = info;
        prepared = true;
        sine_generator.initialise(1, info.sampleRate);
    }

    void processCallback(
        const float* const* inputBuffers,
        float* const* outputBuffers,
        int numInputChannels,
        int numOutputChannels,
        int numSamples,
        double sampleRate,
        int blockSize
    ) override {
        // if (bypassed || !prepared) return;
        sine_generator.advance(numSamples);
        sine_generator.copyOutputFrames(
            sine_generator.getEndpointHandleForName("out"), outputBuffers[0],
            numSamples);
    }

};