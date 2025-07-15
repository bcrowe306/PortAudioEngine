#pragma once

#include <portaudio.h>
#include <vector>
#include <string>
#include <memory>
#include "AudioGraph.h"
#include "AudioNode.h"
#include "../../lib/choc/audio/choc_SampleBuffers.h"
#include "../../lib/choc/audio/choc_AudioFileFormat_WAV.h"

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

    // Offline rendering
    struct OfflineRenderParams {
        std::string outputFilePath;
        
        // Render length (choose one)
        int lengthInSamples = 0;           // Direct sample count
        double lengthInSeconds = 0.0;      // Time in seconds
        int lengthInTicks = 0;             // Musical time in ticks
        double tempoBeatsPerMinute = 120.0; // BPM for tick calculation
        int ticksPerQuarterNote = 480;     // TPQN for tick calculation
        
        // Rendering options
        std::shared_ptr<AudioNode> sourceNode = nullptr; // Start from specific node, nullptr = whole graph
        double renderSampleRate = 44100.0; // Sample rate for offline render
        int renderBufferSize = 1024;       // Buffer size for processing chunks
        bool includeInput = false;         // Whether to include input buffers (silent for offline)
    };
    
    // Perform offline render
    bool renderOffline(const OfflineRenderParams& params);
    
    // Helper to calculate samples from different time units
    static int calculateSamplesFromParams(const OfflineRenderParams& params);

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