#include "core/AudioCore.h"
#include "osc.h"
#include <iostream>
#include "core/Logger.h"

class CmajorTest : public AudioNode {
public:

    OSC osc_generator;
    CmajorTest(const std::string& name = "CmajorTest") : AudioNode(name) {
        
    }

    void prepare(const PrepareInfo &info) override {
        Logger::info("CmajorTest::prepare() called with sample rate: " + std::to_string(info.sampleRate));
        currentPrepareInfo = info;
        prepared = true;
        osc_generator.initialise(1, info.sampleRate);
        float frequency = 440.0f; // Default frequency
        osc_generator.setValue(osc_generator.getEndpointHandleForName("frequency"), &frequency, 0.0f);
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
        
        if (numOutputChannels > 0) {
            // Get a temporary buffer for the first channel
            std::vector<float> tempBuffer(numSamples);
            osc_generator.copyOutputFrames(
                osc_generator.getEndpointHandleForName("out"), tempBuffer.data(),
                static_cast<int>(numSamples));
            
            // Copy to all output channels
            for (choc::buffer::ChannelCount ch = 0; ch < numOutputChannels; ++ch) {
                for (choc::buffer::FrameCount frame = 0; frame < numSamples; ++frame) {
                    outputBuffers.getSample(ch, frame) = tempBuffer[frame];
                }
            }
        }
        osc_generator.advance(static_cast<int>(numSamples));
    }

};