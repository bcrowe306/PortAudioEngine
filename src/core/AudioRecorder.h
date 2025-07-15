#pragma once

#include "AudioNode.h"
#include <vector>
#include <atomic>
#include <memory>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>

#include "containers/choc_SingleReaderSingleWriterFIFO.h"
#include "audio/choc_AudioFileFormat_WAV.h"
#include "audio/choc_SampleBuffers.h"

class AudioRecorder : public AudioNode {
public:
    AudioRecorder(const std::string& filename = "", const std::string& name = "AudioRecorder");
    ~AudioRecorder();
    
    // AudioNode interface
    void processCallback(
        choc::buffer::ChannelArrayView<const float> inputBuffers,
        choc::buffer::ChannelArrayView<float> outputBuffers,
        double sampleRate,
        int blockSize
    ) override;
    
    void prepare(const PrepareInfo& info) override;
    
    // Recording controls
    void startRecording(const std::string& filename = "");
    void stopRecording();
    bool isRecording() const { return recording.load(); }
    
    // Get recorded data for playback
    std::vector<float> getRecordedData() const;
    void clearRecordedData();
    
    // Statistics
    size_t getTotalSamplesRecorded() const { return totalSamplesRecorded.load(); }
    double getRecordingDuration() const;

private:
    void writerThreadFunction();
    
    std::unique_ptr<choc::fifo::SingleReaderSingleWriterFIFO<float>> fifo;
    std::atomic<bool> recording{false};
    std::atomic<bool> shouldStopWriter{false};
    std::atomic<size_t> totalSamplesRecorded{0};
    
    std::string currentFilename;
    double currentSampleRate = 44100.0;
    int currentChannels = 1;
    
    // Writer thread for disk I/O
    std::thread writerThread;
    std::mutex filenameMutex;
    
    // CHOC WAV format handler
    choc::audio::WAVAudioFileFormat<true> wavFormat;
    
    // Recorded data storage (for playback testing)
    mutable std::mutex recordedDataMutex;
    std::vector<float> recordedData;
    
    static constexpr size_t FIFO_SIZE = 1024 * 1024; // 1MB buffer
    static constexpr size_t WRITE_CHUNK_SIZE = 4096;
};
