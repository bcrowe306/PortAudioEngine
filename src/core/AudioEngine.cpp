#include "AudioEngine.h"
#include <stdexcept>
#include <cstring>
#include <chrono>
#include <iostream>
#include <algorithm>

AudioEngine::AudioEngine() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error("Failed to initialize PortAudio");
    }
    enumerateDevices();
    
    // Initialize audio graph system
    audioGraph = std::make_unique<AudioGraph>();
    processor = std::make_unique<AudioGraphProcessor>();
}

AudioEngine::~AudioEngine() {
    stopStream();
    Pa_Terminate();
}

void AudioEngine::enumerateDevices() {
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        throw std::runtime_error("Pa_GetDeviceCount failed");
    }

    inputDevices.clear();
    outputDevices.clear();

    for (int i = 0; i < numDevices; ++i) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        DeviceInfo dev;
        dev.index = i;
        dev.name = info->name ? info->name : "";
        dev.maxInputChannels = info->maxInputChannels;
        dev.maxOutputChannels = info->maxOutputChannels;
        dev.defaultSampleRate = info->defaultSampleRate;

        if (dev.maxInputChannels > 0) {
            inputDevices.push_back(dev);
        }
        if (dev.maxOutputChannels > 0) {
            outputDevices.push_back(dev);
        }
    }
}

void AudioEngine::startStream(int inputDeviceIndex, int outputDeviceIndex, int bufferSize_, double sampleRate_) {
    stopStream();

    bufferSize = bufferSize_;
    sampleRate = sampleRate_;

    PaStreamParameters inputParams = {};
    PaStreamParameters outputParams = {};

    if (inputDeviceIndex >= 0 && inputDeviceIndex < (int)inputDevices.size()) {
        const DeviceInfo& dev = inputDevices[inputDeviceIndex];
        inputParams.device = dev.index;
        // Use configured input channels, but clamp to device max
        // If no input channels configured, default to device max or 2, whichever is smaller
        int desiredInputChannels = (inputChannels > 0) ? inputChannels : std::min(2, dev.maxInputChannels);
        inputParams.channelCount = std::min(desiredInputChannels, dev.maxInputChannels);
        inputParams.sampleFormat = paFloat32;
        inputParams.suggestedLatency = Pa_GetDeviceInfo(dev.index)->defaultLowInputLatency;
        inputParams.hostApiSpecificStreamInfo = nullptr;
        
        // Update actual input channels being used
        inputChannels = inputParams.channelCount;
    } else {
        inputParams.device = paNoDevice;
        inputChannels = 0;  // No input
    }

    if (outputDeviceIndex >= 0 && outputDeviceIndex < (int)outputDevices.size()) {
        const DeviceInfo& dev = outputDevices[outputDeviceIndex];
        outputParams.device = dev.index;
        // Use configured output channels, but clamp to device max
        // If no output channels configured, default to 2 (stereo) or device max, whichever is smaller
        int desiredOutputChannels = (outputChannels > 0) ? outputChannels : std::min(2, dev.maxOutputChannels);
        outputParams.channelCount = std::min(desiredOutputChannels, dev.maxOutputChannels);
        outputParams.sampleFormat = paFloat32;
        outputParams.suggestedLatency = Pa_GetDeviceInfo(dev.index)->defaultLowOutputLatency;
        outputParams.hostApiSpecificStreamInfo = nullptr;
        
        // Update actual output channels being used
        outputChannels = outputParams.channelCount;
    } else {
        outputParams.device = paNoDevice;
        outputChannels = 0;  // No output
    }

    PaError err = Pa_OpenStream(
        &stream,
        inputParams.device != paNoDevice ? &inputParams : nullptr,
        outputParams.device != paNoDevice ? &outputParams : nullptr,
        sampleRate,
        bufferSize,
        paNoFlag,
        &AudioEngine::paCallback,
        this
    );
    if (err != paNoError) {
        throw std::runtime_error("Failed to open PortAudio stream");
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        throw std::runtime_error("Failed to start PortAudio stream");
    }
    
    // Prepare the audio graph system
    prepareAudioGraph();
}

void AudioEngine::startStream(int bufferSize_, double sampleRate_) {
    // Use default devices
    int defaultInputIndex = getDefaultInputDeviceIndex();
    int defaultOutputIndex = getDefaultOutputDeviceIndex();
    
    startStream(defaultInputIndex, defaultOutputIndex, bufferSize_, sampleRate_);
}

void AudioEngine::stopStream() {
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
    }
}

bool AudioEngine::isStreamActive() const {
    return stream && Pa_IsStreamActive(stream) == 1;
}

void AudioEngine::setBufferSize(int bufferSize_) {
    bufferSize = bufferSize_;
}

void AudioEngine::setSampleRate(double sampleRate_) {
    sampleRate = sampleRate_;
}

// TODO:Fix Implement audio graph preparation
void AudioEngine::prepareAudioGraph() {
    if (audioGraph && sampleRate > 0 && bufferSize > 0) {
        AudioNode::PrepareInfo info;
        info.sampleRate = sampleRate;
        info.maxBufferSize = bufferSize;
        info.numChannels = outputChannels; // Use actual output channel count
        
        audioGraph->prepare(info);
    }
}

int AudioEngine::getDefaultOutputDeviceIndex() const {
    PaDeviceIndex defaultDevice = Pa_GetDefaultOutputDevice();
    if (defaultDevice == paNoDevice) {
        // Fallback to first available output device
        return outputDevices.empty() ? -1 : 0;
    }
    
    // Find the device in our output devices list
    for (size_t i = 0; i < outputDevices.size(); ++i) {
        if (outputDevices[i].index == defaultDevice) {
            return static_cast<int>(i);
        }
    }
    
    // If default device not found in our list, use first available
    return outputDevices.empty() ? -1 : 0;
}

int AudioEngine::getDefaultInputDeviceIndex() const {
    PaDeviceIndex defaultDevice = Pa_GetDefaultInputDevice();
    if (defaultDevice == paNoDevice) {
        // Return -1 to indicate no input device (output-only)
        return -1;
    }
    
    // Find the device in our input devices list
    for (size_t i = 0; i < inputDevices.size(); ++i) {
        if (inputDevices[i].index == defaultDevice) {
            return static_cast<int>(i);
        }
    }
    
    // If default device not found in our list, return -1 (no input)
    return -1;
}

int AudioEngine::paCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    
    // Check if graph needs recompilation and update processor atomically
    // We do this in the audio thread but try to minimize blocking
    if (engine->audioGraph && engine->audioGraph->needsRecompile()) {
        std::cout << "Engine recompiling audio graph..." << std::endl;
        // Try to get current compiled graph first (lock-free)
        engine->prepareAudioGraph();
        auto compiledGraph = engine->audioGraph->getCompiledGraph();
        if (engine->processor && compiledGraph) {
            engine->processor->setCompiledGraph(compiledGraph);
        }
    }
    
    if (outputBuffer && engine->processor && engine->outputChannels > 0) {
        // Convert PortAudio interleaved buffer to separate channel pointers
        float* interleavedOutput = static_cast<float*>(outputBuffer);
        
        // Use actual output channel count
        const int numChannels = engine->outputChannels;
        const int numSamples = static_cast<int>(framesPerBuffer);
        
        // Create temporary deinterleaved buffers
        std::vector<std::vector<float>> channelBuffers(numChannels);
        std::vector<float*> channelPtrs(numChannels);
        
        for (int ch = 0; ch < numChannels; ++ch) {
            channelBuffers[ch].resize(numSamples);
            channelPtrs[ch] = channelBuffers[ch].data();
        }
        
        // Process the audio graph
        engine->processor->processGraph(
            channelPtrs.data(),
            numChannels,
            numSamples,
            engine->sampleRate,
            numSamples
        );
        
        // Convert back to interleaved format
        for (int sample = 0; sample < numSamples; ++sample) {
            for (int ch = 0; ch < numChannels; ++ch) {
                interleavedOutput[sample * numChannels + ch] = channelBuffers[ch][sample];
            }
        }
        
        // Debug: Check if we have audio
        static int audioDebugCount = 0;
        if (audioDebugCount < 2) {
            std::cout << "Audio callback - channels: " << numChannels 
                      << ", first sample: " << channelBuffers[0][0];
            if (numChannels > 1) {
                std::cout << ", " << channelBuffers[1][0];
            }
            std::cout << std::endl;
            audioDebugCount++;
        }
    } else if (outputBuffer && engine->outputChannels > 0) {
        // Fallback: just zero the output
        std::memset(outputBuffer, 0, framesPerBuffer * sizeof(float) * engine->outputChannels);
    }
    
    return paContinue;
}
