#include "AudioEngine.h"
#include "Logger.h"
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
        Logger::debug("Engine recompiling audio graph...");
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
        const float* interleavedInput = static_cast<const float*>(inputBuffer);
        
        // Use actual channel counts
        const int numOutputChannels = engine->outputChannels;
        const int numInputChannels = engine->inputChannels;
        const int numSamples = static_cast<int>(framesPerBuffer);
        
        // Create owned input buffer (ChannelArrayBuffer)
        choc::buffer::ChannelArrayBuffer<float> inputChannelBuffer(
            static_cast<choc::buffer::ChannelCount>(numInputChannels), 
            static_cast<choc::buffer::FrameCount>(numSamples)
        );
        
        // Create owned output buffer (ChannelArrayBuffer)
        choc::buffer::ChannelArrayBuffer<float> outputChannelBuffer(
            static_cast<choc::buffer::ChannelCount>(numOutputChannels), 
            static_cast<choc::buffer::FrameCount>(numSamples)
        );
        
        // Convert interleaved input to deinterleaved input buffer
        if (interleavedInput && numInputChannels > 0) {
            for (int sample = 0; sample < numSamples; ++sample) {
                for (int ch = 0; ch < numInputChannels; ++ch) {
                    inputChannelBuffer.getSample(static_cast<choc::buffer::ChannelCount>(ch), 
                                               static_cast<choc::buffer::FrameCount>(sample)) = 
                        interleavedInput[sample * numInputChannels + ch];
                }
            }
        }
        
        // Process the audio graph with both input and output buffers
        engine->processor->processGraph(
            inputChannelBuffer.getView(),
            outputChannelBuffer.getView(),
            engine->sampleRate,
            numSamples
        );
        
        // Convert back to interleaved format
        for (int sample = 0; sample < numSamples; ++sample) {
            for (int ch = 0; ch < numOutputChannels; ++ch) {
                interleavedOutput[sample * numOutputChannels + ch] = 
                    outputChannelBuffer.getSample(static_cast<choc::buffer::ChannelCount>(ch), 
                                                static_cast<choc::buffer::FrameCount>(sample));
            }
        }
        
    } else if (outputBuffer && engine->outputChannels > 0) {
        // Fallback: just zero the output
        std::memset(outputBuffer, 0, framesPerBuffer * sizeof(float) * engine->outputChannels);
    }
    
    return paContinue;
}

// Offline rendering implementation
bool AudioEngine::renderOffline(const OfflineRenderParams& params) {
    if (params.outputFilePath.empty()) {
        Logger::error("Output file path is required for offline rendering");
        return false;
    }
    
    // Calculate total samples to render
    int totalSamples = calculateSamplesFromParams(params);
    if (totalSamples <= 0) {
        Logger::error("Invalid render length specified");
        return false;
    }
    
    Logger::info("Starting offline render: {} samples at {} Hz", totalSamples, params.renderSampleRate);
    
    // Prepare audio graph for offline rendering
    if (!audioGraph) {
        Logger::error("No audio graph available for rendering");
        return false;
    }
    
    // Store current state to restore later
    double originalSampleRate = sampleRate;
    int originalBufferSize = bufferSize;
    int originalOutputChannels = outputChannels;
    int originalInputChannels = inputChannels;
    
    // Set up for offline rendering
    sampleRate = params.renderSampleRate;
    bufferSize = params.renderBufferSize;
    // Keep existing channel configuration or use defaults
    if (outputChannels == 0) outputChannels = 2; // Default to stereo
    
    // Prepare the audio graph with offline settings
    AudioNode::PrepareInfo prepareInfo;
    prepareInfo.sampleRate = sampleRate;
    prepareInfo.maxBufferSize = bufferSize;
    prepareInfo.numChannels = outputChannels;
    
    audioGraph->prepare(prepareInfo);
    
    // Create processor for offline rendering
    auto offlineProcessor = std::make_unique<AudioGraphProcessor>();
    auto compiledGraph = audioGraph->getCompiledGraph();
    if (!compiledGraph) {
        Logger::error("Failed to compile audio graph for offline rendering");
        goto cleanup;
    }
    offlineProcessor->setCompiledGraph(compiledGraph);
    
    try {
        // Create output buffer for entire render
        choc::buffer::ChannelArrayBuffer<float> fullOutputBuffer(
            static_cast<choc::buffer::ChannelCount>(outputChannels),
            static_cast<choc::buffer::FrameCount>(totalSamples)
        );
        
        // Create input buffer (silent for offline rendering unless specified)
        choc::buffer::ChannelArrayBuffer<float> inputBuffer(
            static_cast<choc::buffer::ChannelCount>(params.includeInput ? inputChannels : 0),
            static_cast<choc::buffer::FrameCount>(bufferSize)
        );
        if (params.includeInput) {
            inputBuffer.clear(); // Fill with silence
        }
        
        // Create empty input buffer for when no input is needed
        choc::buffer::ChannelArrayBuffer<float> emptyInputBuffer(
            static_cast<choc::buffer::ChannelCount>(0),
            static_cast<choc::buffer::FrameCount>(bufferSize)
        );
        
        // Process in chunks
        int samplesRendered = 0;
        while (samplesRendered < totalSamples) {
            int samplesThisChunk = std::min(bufferSize, totalSamples - samplesRendered);
            
            // Create output buffer view for this chunk
            auto outputChunkView = fullOutputBuffer.getView().getFrameRange(
                {static_cast<choc::buffer::FrameCount>(samplesRendered), 
                 static_cast<choc::buffer::FrameCount>(samplesRendered + samplesThisChunk)}
            );
            
            // Create input buffer view for this chunk (if using input)
            choc::buffer::ChannelArrayView<const float> inputChunkView;
            if (params.includeInput && inputChannels > 0) {
                // Resize input buffer if needed for this chunk
                if (inputBuffer.getNumFrames() != static_cast<choc::buffer::FrameCount>(samplesThisChunk)) {
                    inputBuffer.resize({static_cast<choc::buffer::ChannelCount>(inputChannels), 
                                       static_cast<choc::buffer::FrameCount>(samplesThisChunk)});
                    inputBuffer.clear();
                }
                inputChunkView = inputBuffer.getView();
            } else {
                // Use the empty input buffer, resizing if needed
                if (emptyInputBuffer.getNumFrames() != static_cast<choc::buffer::FrameCount>(samplesThisChunk)) {
                    emptyInputBuffer.resize({static_cast<choc::buffer::ChannelCount>(0),
                                           static_cast<choc::buffer::FrameCount>(samplesThisChunk)});
                }
                inputChunkView = emptyInputBuffer.getView();
            }
            
            // Process this chunk
            if (params.sourceNode) {
                // Render from specific node
                params.sourceNode->processCallback(
                    inputChunkView,
                    outputChunkView,
                    sampleRate,
                    samplesThisChunk
                );
            } else {
                // Render entire graph
                offlineProcessor->processGraph(
                    inputChunkView,
                    outputChunkView,
                    sampleRate,
                    samplesThisChunk
                );
            }
            
            samplesRendered += samplesThisChunk;
            
            // Progress indicator
            if (samplesRendered % (totalSamples / 10) == 0) {
                int progress = (samplesRendered * 100) / totalSamples;
                Logger::debug("Rendering progress: {}%", progress);
            }
        }
        
        // Write to WAV file
        choc::audio::WAVAudioFileFormat<true> wavFormat; // Set to true to enable writing
        
        choc::audio::AudioFileProperties fileProps;
        fileProps.sampleRate = params.renderSampleRate;
        fileProps.numChannels = outputChannels;
        fileProps.bitDepth = choc::audio::BitDepth::float32; // Use 32-bit float
        
        auto writer = wavFormat.createWriter(params.outputFilePath, fileProps);
        if (!writer) {
            Logger::error("Failed to create WAV writer for: {}", params.outputFilePath);
            goto cleanup;
        }
        
        if (!writer->appendFrames(fullOutputBuffer.getView())) {
            Logger::error("Failed to write audio data to file");
            goto cleanup;
        }
        
        Logger::info("Offline render completed successfully: {}", params.outputFilePath);
        Logger::info("Rendered {} samples ({:.2f} seconds)", totalSamples, (totalSamples / params.renderSampleRate));
        
        // Restore original state
        sampleRate = originalSampleRate;
        bufferSize = originalBufferSize;
        outputChannels = originalOutputChannels;
        inputChannels = originalInputChannels;
        
        // Re-prepare with original settings if we had a real-time session
        if (originalSampleRate > 0 && originalBufferSize > 0) {
            prepareAudioGraph();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Error during offline rendering: {}", e.what());
        goto cleanup;
    }
    
cleanup:
    // Restore original state
    sampleRate = originalSampleRate;
    bufferSize = originalBufferSize;
    outputChannels = originalOutputChannels;
    inputChannels = originalInputChannels;
    
    // Re-prepare with original settings if we had a real-time session
    if (originalSampleRate > 0 && originalBufferSize > 0) {
        prepareAudioGraph();
    }
    
    return false;
}

int AudioEngine::calculateSamplesFromParams(const OfflineRenderParams& params) {
    // Priority: samples > seconds > ticks
    if (params.lengthInSamples > 0) {
        return params.lengthInSamples;
    }
    
    if (params.lengthInSeconds > 0.0) {
        return static_cast<int>(params.lengthInSeconds * params.renderSampleRate);
    }
    
    if (params.lengthInTicks > 0) {
        // Convert ticks to seconds: ticks / (TPQN * BPM / 60)
        double secondsPerTick = 60.0 / (params.tempoBeatsPerMinute * params.ticksPerQuarterNote);
        double totalSeconds = params.lengthInTicks * secondsPerTick;
        return static_cast<int>(totalSeconds * params.renderSampleRate);
    }
    
    return 0; // Invalid parameters
}
