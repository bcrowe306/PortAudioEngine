#pragma once

#include "AudioNode.h"
#include <atomic>
#include <array>
#include <vector>
#include <complex>
#include <mutex>

class AnalyzerNode : public AudioNode {
public:
    struct SpectrumData {
        std::vector<float> magnitudes;  // Frequency bin magnitudes (dB)
        std::vector<float> frequencies; // Frequency bin centers (Hz)
        double sampleRate = 44100.0;
        int fftSize = 0;
    };

    AnalyzerNode(const std::string& name = "AnalyzerNode", int fftSize = 2048);
    ~AnalyzerNode() override = default;

    void prepare(const PrepareInfo& info) override;
    
    void processCallback(
        const float* const* inputBuffers,
        float* const* outputBuffers,
        int numInputChannels,
        int numOutputChannels,
        int numSamples,
        double sampleRate,
        int blockSize
    ) override;
    
    // Thread-safe spectrum reading (call from UI thread)
    SpectrumData getCurrentSpectrum();
    
    // Configuration
    void setFFTSize(int newSize);
    int getFFTSize() const { return fftSize; }
    
    void setWindowType(int type) { windowType = type; }
    int getWindowType() const { return windowType; }
    
    // Window types
    enum WindowType {
        RECTANGULAR = 0,
        HANNING = 1,
        HAMMING = 2,
        BLACKMAN = 3
    };

private:
    int fftSize;
    int windowType = HANNING;
    
    // Input buffer for accumulating samples
    std::vector<float> inputBuffer;
    int bufferWriteIndex = 0;
    
    // FFT working buffers
    std::vector<std::complex<float>> fftInput;
    std::vector<std::complex<float>> fftOutput;
    std::vector<float> windowFunction;
    
    // Spectrum data (thread-safe)
    mutable std::mutex spectrumMutex;
    SpectrumData latestSpectrum;
    bool spectrumReady = false;
    
    // Smoothing
    std::vector<float> smoothedMagnitudes;
    static constexpr float SMOOTHING_FACTOR = 0.8f;
    
    // Helper methods
    void initializeWindow();
    void performFFT();
    void calculateMagnitudes();
    void applyWindow(std::vector<float>& buffer);
    
    // Simple FFT implementation
    void fft(std::vector<std::complex<float>>& data);
    void fftRecursive(std::vector<std::complex<float>>& data, int n);
    
    // Convert linear magnitude to dB
    float magnitudeToDb(float magnitude);
    
    // Check if number is power of 2
    bool isPowerOfTwo(int n);
};
