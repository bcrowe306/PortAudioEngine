#include "AnalyzerNode.h"
#include <cmath>
#include <algorithm>
#include <cassert>

AnalyzerNode::AnalyzerNode(const std::string& name, int fftSize) 
    : AudioNode(name), fftSize(fftSize) {
    
    // Ensure FFT size is power of 2
    if (!isPowerOfTwo(fftSize)) {
        // Round up to nearest power of 2
        int powerOf2 = 1;
        while (powerOf2 < fftSize) {
            powerOf2 *= 2;
        }
        this->fftSize = powerOf2;
    }
    
    // Initialize buffers
    inputBuffer.resize(this->fftSize, 0.0f);
    fftInput.resize(this->fftSize);
    fftOutput.resize(this->fftSize);
    windowFunction.resize(this->fftSize);
    smoothedMagnitudes.resize(this->fftSize / 2, 0.0f);
    
    initializeWindow();
}

void AnalyzerNode::prepare(const PrepareInfo& info) {
    AudioNode::prepare(info);
    
    // Initialize spectrum data structure
    std::lock_guard<std::mutex> lock(spectrumMutex);
    latestSpectrum.magnitudes.resize(fftSize / 2);
    latestSpectrum.frequencies.resize(fftSize / 2);
    latestSpectrum.sampleRate = info.sampleRate;
    latestSpectrum.fftSize = fftSize;
    
    // Calculate frequency bin centers
    for (int i = 0; i < fftSize / 2; ++i) {
        latestSpectrum.frequencies[i] = static_cast<float>(i * info.sampleRate / fftSize);
    }
    
    spectrumReady = false;
}

void AnalyzerNode::processCallback(
    choc::buffer::ChannelArrayView<const float> inputBuffers,
    choc::buffer::ChannelArrayView<float> outputBuffers,
    double sampleRate,
    int blockSize
) {
    auto numInputChannels = inputBuffers.getNumChannels();
    auto numOutputChannels = outputBuffers.getNumChannels();
    auto numSamples = outputBuffers.getNumFrames();
    
    // Pass through audio (this is an analyzer, not an effect)
    copyBuffer(inputBuffers, outputBuffers);
    
    // Accumulate samples for FFT analysis (mix to mono if stereo)
    for (choc::buffer::FrameCount i = 0; i < numSamples; ++i) {
        float sample = 0.0f;
        if (numInputChannels >= 1) {
            sample += inputBuffers.getSample(0, i);
        }
        if (numInputChannels >= 2) {
            sample += inputBuffers.getSample(1, i);
            sample *= 0.5f; // Average stereo channels
        }
        
        inputBuffer[bufferWriteIndex] = sample;
        bufferWriteIndex = (bufferWriteIndex + 1) % fftSize;
        
        // When buffer is full, perform FFT
        if (bufferWriteIndex == 0) {
            performFFT();
        }
    }
}

void AnalyzerNode::performFFT() {
    // Copy input buffer to FFT input (with proper ordering)
    for (int i = 0; i < fftSize; ++i) {
        int index = (bufferWriteIndex + i) % fftSize;
        fftInput[i] = std::complex<float>(inputBuffer[index], 0.0f);
    }
    
    // Apply window function
    for (int i = 0; i < fftSize; ++i) {
        fftInput[i] *= windowFunction[i];
    }
    
    // Perform FFT
    fftOutput = fftInput;
    fft(fftOutput);
    
    // Calculate magnitudes and update spectrum
    calculateMagnitudes();
}

void AnalyzerNode::calculateMagnitudes() {
    std::lock_guard<std::mutex> lock(spectrumMutex);
    
    for (int i = 0; i < fftSize / 2; ++i) {
        float magnitude = std::abs(fftOutput[i]);
        float magnitudeDb = magnitudeToDb(magnitude);
        
        // Apply smoothing
        smoothedMagnitudes[i] = SMOOTHING_FACTOR * smoothedMagnitudes[i] + 
                               (1.0f - SMOOTHING_FACTOR) * magnitudeDb;
        
        latestSpectrum.magnitudes[i] = smoothedMagnitudes[i];
    }
    
    spectrumReady = true;
}

AnalyzerNode::SpectrumData AnalyzerNode::getCurrentSpectrum() {
    std::lock_guard<std::mutex> lock(spectrumMutex);
    return latestSpectrum;
}

void AnalyzerNode::setFFTSize(int newSize) {
    if (!isPowerOfTwo(newSize)) {
        // Round up to nearest power of 2
        int powerOf2 = 1;
        while (powerOf2 < newSize) {
            powerOf2 *= 2;
        }
        newSize = powerOf2;
    }
    
    if (newSize != fftSize) {
        fftSize = newSize;
        
        // Resize buffers
        inputBuffer.resize(fftSize, 0.0f);
        fftInput.resize(fftSize);
        fftOutput.resize(fftSize);
        windowFunction.resize(fftSize);
        smoothedMagnitudes.resize(fftSize / 2, 0.0f);
        
        bufferWriteIndex = 0;
        initializeWindow();
        
        // Re-prepare if already prepared
        if (isPrepared()) {
            prepare(currentPrepareInfo);
        }
    }
}

void AnalyzerNode::initializeWindow() {
    for (int i = 0; i < fftSize; ++i) {
        float n = static_cast<float>(i);
        float N = static_cast<float>(fftSize);
        
        switch (windowType) {
            case RECTANGULAR:
                windowFunction[i] = 1.0f;
                break;
                
            case HANNING:
                windowFunction[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * n / (N - 1.0f)));
                break;
                
            case HAMMING:
                windowFunction[i] = 0.54f - 0.46f * std::cos(2.0f * M_PI * n / (N - 1.0f));
                break;
                
            case BLACKMAN:
                windowFunction[i] = 0.42f - 0.5f * std::cos(2.0f * M_PI * n / (N - 1.0f)) +
                                  0.08f * std::cos(4.0f * M_PI * n / (N - 1.0f));
                break;
                
            default:
                windowFunction[i] = 1.0f;
                break;
        }
    }
}

void AnalyzerNode::fft(std::vector<std::complex<float>>& data) {
    int n = static_cast<int>(data.size());
    
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
    
    // Cooley-Tukey FFT
    for (int len = 1; len < n; len <<= 1) {
        float angle = -M_PI / len;
        std::complex<float> wlen(std::cos(angle), std::sin(angle));
        
        for (int i = 0; i < n; i += len << 1) {
            std::complex<float> w(1);
            
            for (int j = 0; j < len; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len] * w;
                
                data[i + j] = u + v;
                data[i + j + len] = u - v;
                
                w *= wlen;
            }
        }
    }
}

float AnalyzerNode::magnitudeToDb(float magnitude) {
    const float minDb = -120.0f;
    if (magnitude <= 0.0f) {
        return minDb;
    }
    
    float db = 20.0f * std::log10(magnitude);
    return std::max(db, minDb);
}

bool AnalyzerNode::isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}
