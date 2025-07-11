#include "AudioRecorder.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <sstream>

// AudioRecorder implementation
AudioRecorder::AudioRecorder(const std::string& filename, const std::string& name)
    : AudioNode(name), currentFilename(filename) {
    fifo = std::make_unique<choc::fifo::SingleReaderSingleWriterFIFO<float>>();
    fifo->reset(FIFO_SIZE);
}

AudioRecorder::~AudioRecorder() {
    stopRecording();
    if (writerThread.joinable()) {
        writerThread.join();
    }
}

void AudioRecorder::prepare(const PrepareInfo& info) {
    AudioNode::prepare(info);
    currentSampleRate = info.sampleRate;
    currentChannels = info.numChannels;
}

void AudioRecorder::processCallback(
    const float* const* inputBuffers,
    float* const* outputBuffers,
    int numInputChannels,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize
) {
    // Pass input through to output (if connected)
    for (int outCh = 0; outCh < numOutputChannels; ++outCh) {
        if (outCh < numInputChannels && inputBuffers[outCh]) {
            // Copy input to output
            std::memcpy(outputBuffers[outCh], inputBuffers[outCh], numSamples * sizeof(float));
        } else {
            // Clear output if no input
            std::memset(outputBuffers[outCh], 0, numSamples * sizeof(float));
        }
    }
    
    // Record input data if recording is active
    if (recording.load() && numInputChannels > 0 && inputBuffers[0]) {
        // For simplicity, record only the first channel
        // Push samples one by one using CHOC FIFO
        for (int i = 0; i < numSamples; ++i) {
            if (fifo->push(inputBuffers[0][i])) {
                totalSamplesRecorded.fetch_add(1);
                
                // Also store in memory for playback testing
                {
                    std::lock_guard<std::mutex> lock(recordedDataMutex);
                    recordedData.push_back(inputBuffers[0][i]);
                }
            } else {
                // FIFO full, could log warning or implement overflow handling
                break;
            }
        }
    }
}

void AudioRecorder::startRecording(const std::string& filename) {
    if (recording.load()) {
        stopRecording();
    }
    
    {
        std::lock_guard<std::mutex> lock(filenameMutex);
        if (!filename.empty()) {
            currentFilename = filename;
        }
        if (currentFilename.empty()) {
            currentFilename = "recording.wav";
        }
    }
    
    // Clear previous data
    clearRecordedData();
    totalSamplesRecorded.store(0);
    
    // Start writer thread
    shouldStopWriter.store(false);
    recording.store(true);
    
    if (writerThread.joinable()) {
        writerThread.join();
    }
    
    writerThread = std::thread(&AudioRecorder::writerThreadFunction, this);
    
    std::cout << "Started recording to: " << currentFilename << std::endl;
}

void AudioRecorder::stopRecording() {
    if (!recording.load()) {
        return;
    }
    
    recording.store(false);
    shouldStopWriter.store(true);
    
    if (writerThread.joinable()) {
        writerThread.join();
    }
    
    std::cout << "Stopped recording. Total samples: " << totalSamplesRecorded.load() << std::endl;
}

std::vector<float> AudioRecorder::getRecordedData() const {
    std::lock_guard<std::mutex> lock(recordedDataMutex);
    return recordedData;
}

void AudioRecorder::clearRecordedData() {
    std::lock_guard<std::mutex> lock(recordedDataMutex);
    recordedData.clear();
}

double AudioRecorder::getRecordingDuration() const {
    return static_cast<double>(totalSamplesRecorded.load()) / currentSampleRate;
}

void AudioRecorder::writerThreadFunction() {
    std::string filename;
    {
        std::lock_guard<std::mutex> lock(filenameMutex);
        filename = currentFilename;
    }
    
    try {
        // Create properties for the WAV file
        choc::audio::AudioFileProperties properties;
        properties.formatName = "WAV";
        properties.sampleRate = currentSampleRate;
        properties.numChannels = currentChannels;
        properties.bitDepth = choc::audio::BitDepth::float32; // Use 32-bit float for high quality
        properties.numFrames = 0; // Will be updated as we write
        
        // Create output stream
        auto outputStream = std::make_shared<std::ofstream>(filename, std::ios::binary);
        if (!outputStream->is_open()) {
            std::cerr << "Failed to open file for recording: " << filename << std::endl;
            return;
        }
        
        // Create WAV writer using CHOC
        auto writer = wavFormat.createWriter(outputStream, std::move(properties));
        if (!writer) {
            std::cerr << "Failed to create WAV writer for: " << filename << std::endl;
            return;
        }
        
        // Create a buffer for writing chunks
        choc::buffer::ChannelArrayBuffer<float> writeBuffer(currentChannels, WRITE_CHUNK_SIZE);
        
        std::vector<float> sampleBuffer;
        sampleBuffer.reserve(WRITE_CHUNK_SIZE);
        
        uint32_t totalSamplesWritten = 0;
        
        while (!shouldStopWriter.load() || fifo->getUsedSlots() > 0) {
            sampleBuffer.clear();
            
            // Collect samples from FIFO
            float sample;
            while (sampleBuffer.size() < WRITE_CHUNK_SIZE && fifo->pop(sample)) {
                sampleBuffer.push_back(sample);
            }
            
            if (sampleBuffer.empty()) {
                // No data available, sleep briefly
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            
            // Resize the write buffer to match the number of samples we have
            auto framesToWrite = static_cast<choc::buffer::FrameCount>(sampleBuffer.size());
            writeBuffer.resize(choc::buffer::Size::create(currentChannels, framesToWrite));
            
            // Fill the write buffer - for mono recording, copy samples to all channels
            for (uint32_t ch = 0; ch < currentChannels; ++ch) {
                auto channelData = writeBuffer.getChannel(ch);
                auto iter = channelData.getIterator(0);
                for (size_t i = 0; i < framesToWrite; ++i) {
                    *iter = sampleBuffer[i];
                    ++iter;
                }
            }
            
            // Write to file using CHOC
            auto view = writeBuffer.getView();
            if (!writer->appendFrames(view)) {
                std::cerr << "Failed to write audio frames to file" << std::endl;
                break;
            }
            
            totalSamplesWritten += framesToWrite;
        }
        
        // Flush and finalize the file
        writer->flush();
        writer.reset(); // This will close the file properly
        
        std::cout << "Recording written to disk: " << totalSamplesWritten << " samples" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during recording: " << e.what() << std::endl;
    }
}
