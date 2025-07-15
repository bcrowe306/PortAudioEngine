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
        choc::buffer::ChannelArrayView<const float> inputBuffers,
        choc::buffer::ChannelArrayView<float> outputBuffers,
        double sampleRate,
        int blockSize
    ) override {
        auto numSamples = outputBuffers.getNumFrames();
        auto numOutputChannels = outputBuffers.getNumChannels();
        
        // if (bypassed || !prepared) return;
        sine_generator.advance(static_cast<int>(numSamples));
        
        if (numOutputChannels > 0) {
            // Get a temporary buffer for the first channel
            std::vector<float> tempBuffer(numSamples);
            sine_generator.copyOutputFrames(
                sine_generator.getEndpointHandleForName("out"), tempBuffer.data(),
                static_cast<int>(numSamples));
            
            // Copy to all output channels
            for (choc::buffer::ChannelCount ch = 0; ch < numOutputChannels; ++ch) {
                for (choc::buffer::FrameCount frame = 0; frame < numSamples; ++frame) {
                    outputBuffers.getSample(ch, frame) = tempBuffer[frame];
                }
            }
        }
    }

};