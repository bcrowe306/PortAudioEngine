#pragma once

#include <portaudio.h>
#include <vector>
#include <string>
#include <memory>
#include "AudioGraph.h"

class AudioEngine {
public:
    struct DeviceInfo {
        int index;
        std::string name;
        int maxInputChannels;
        int maxOutputChannels;
        double defaultSampleRate;
    };

    AudioEngine();
    ~AudioEngine();

    const std::vector<DeviceInfo>& getInputDevices() const { return inputDevices; }
    const std::vector<DeviceInfo>& getOutputDevices() const { return outputDevices; }

    void startStream(int inputDeviceIndex, int outputDeviceIndex, int bufferSize, double sampleRate);
    void startStream(int bufferSize, double sampleRate); // Use default devices
    void stopStream();
    bool isStreamActive() const;

    void setBufferSize(int bufferSize);
    void setSampleRate(double sampleRate);
    int getBufferSize() const { return bufferSize; }
    double getSampleRate() const { return sampleRate; }
    
    // Channel configuration
    void setInputChannels(int channels) { inputChannels = channels; }
    void setOutputChannels(int channels) { outputChannels = channels; }
    void setChannels(int input, int output) { inputChannels = input; outputChannels = output; }
    int getInputChannels() const { return inputChannels; }
    int getOutputChannels() const { return outputChannels; }

    // Audio graph integration
    AudioGraph* getAudioGraph() { return audioGraph.get(); }
    AudioGraphProcessor* getProcessor() { return processor.get(); }
    void prepareAudioGraph();

    // Device utilities
    int getDefaultOutputDeviceIndex() const;
    int getDefaultInputDeviceIndex() const;

private:
    static int paCallback(
        const void* inputBuffer,
        void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

    void enumerateDevices();

    // PortAudio members
    PaStream* stream = nullptr;
    std::vector<DeviceInfo> inputDevices;
    std::vector<DeviceInfo> outputDevices;
    int bufferSize = 0;
    double sampleRate = 0.0;
    int inputChannels = 0;   // Current input channel count
    int outputChannels = 2;  // Default to stereo output

    // Audio graph system
    std::unique_ptr<AudioGraph> audioGraph;
    std::unique_ptr<AudioGraphProcessor> processor;
};