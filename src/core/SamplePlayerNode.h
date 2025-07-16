#pragma once

#include "AudioNode.h"
#include "Logger.h"
#include "../../lib/choc/audio/choc_AudioFileFormat_WAV.h"
#include "../../lib/choc/audio/choc_SampleBuffers.h"
#include <string>
#include <memory>
#include <atomic>
#include <cmath>

class SamplePlayerNode : public AudioNode {
public:
    // Interpolation modes for sample playback
    enum class InterpolationMode {
        NONE,           // No interpolation (nearest neighbor)
        LINEAR,         // Linear interpolation
        CUBIC           // Cubic interpolation (higher quality)
    };

    // Playback states
    enum class PlaybackState {
        STOPPED,
        PLAYING,
        PAUSED
    };

    explicit SamplePlayerNode(const std::string& name = "SamplePlayer");
    virtual ~SamplePlayerNode() = default;

    // AudioNode interface
    void prepare(const PrepareInfo& info) override;
    void processCallback(choc::buffer::ChannelArrayView<const float> input,
                        choc::buffer::ChannelArrayView<float> output,
                        double sampleRate,
                        int numSamples) override;

    // Sample loading
    bool loadSample(const std::string& filePath);
    bool loadSample(const choc::buffer::ChannelArrayBuffer<float>& buffer, double sampleRate = 44100.0);
    void unloadSample();
    bool hasSample() const { return sampleBuffer_.getNumFrames() > 0; }

    // Playback control
    void play();
    void stop();
    void pause();
    void resume();
    void trigger(); // Start playback from beginning
    void trigger(int midiNote); // Start playback with pitch adjustment for MIDI note
    
    PlaybackState getPlaybackState() const { return playbackState_; }
    bool isPlaying() const { return playbackState_ == PlaybackState::PLAYING; }

    // Sample region settings
    void setStartSample(int startSample);
    void setEndSample(int endSample);
    void setSampleRegion(int startSample, int endSample);
    int getStartSample() const { return startSample_; }
    int getEndSample() const { return endSample_; }
    int getSampleLength() const { return endSample_ - startSample_; }

    // Looping
    void setLoop(bool loop) { loop_ = loop; }
    bool isLooping() const { return loop_; }
    void setLoopStart(int loopStart);
    void setLoopEnd(int loopEnd);
    void setLoopRegion(int loopStart, int loopEnd);
    int getLoopStart() const { return loopStart_; }
    int getLoopEnd() const { return loopEnd_; }

    // Pitch and tuning
    void setBaseNote(int midiNote) { baseNote_ = midiNote; updatePlaybackRate(); }
    int getBaseNote() const { return baseNote_; }
    
    void setTranspose(int semitones) { transpose_ = semitones; updatePlaybackRate(); }
    int getTranspose() const { return transpose_; }
    
    void setDetune(float cents) { detune_ = cents; updatePlaybackRate(); }
    float getDetune() const { return detune_; }
    
    void setCurrentNote(int midiNote) { currentNote_ = midiNote; updatePlaybackRate(); }
    int getCurrentNote() const { return currentNote_; }

    // Playback rate control
    void setPlaybackRate(double rate) { manualPlaybackRate_ = rate; useManualRate_ = true; }
    void clearManualPlaybackRate() { useManualRate_ = false; updatePlaybackRate(); }
    double getPlaybackRate() const { return playbackRate_; }

    // Interpolation
    void setInterpolationMode(InterpolationMode mode) { interpolationMode_ = mode; }
    InterpolationMode getInterpolationMode() const { return interpolationMode_; }

    // Volume and gain
    void setGain(float gain) { gain_ = gain; }
    float getGain() const { return gain_; }
    
    void setVolume(float volume) { volume_ = volume; } // 0.0 to 1.0
    float getVolume() const { return volume_; }
    
    // ADSR envelope integration
    void setAmplitudeEnvelope(class ADSR* envelope) { amplitudeEnvelope_ = envelope; }
    void setFilterEnvelope(class ADSR* envelope) { filterEnvelope_ = envelope; }
    void setPitchEnvelope(class ADSR* envelope) { pitchEnvelope_ = envelope; }

    // Position control
    void setPlayPosition(double position); // 0.0 to 1.0
    double getPlayPosition() const;
    void setPlayPositionSamples(int samples);
    int getPlayPositionSamples() const { return static_cast<int>(playPosition_); }

    // Sample info
    int getTotalSamples() const { return static_cast<int>(sampleBuffer_.getNumFrames()); }
    int getNumChannels() const { return static_cast<int>(sampleBuffer_.getNumChannels()); }
    double getSampleRate() const { return sampleSampleRate_; }
    double getDurationSeconds() const;

    // Analysis
    float getRMSLevel() const { return rmsLevel_; }
    float getPeakLevel() const { return peakLevel_; }

    // Debug and info
    void printSampleInfo() const;

private:
    // Sample data
    choc::buffer::ChannelArrayBuffer<float> sampleBuffer_;
    double sampleSampleRate_ = 44100.0;
    std::string loadedFilePath_;

    // Playback state
    std::atomic<PlaybackState> playbackState_{PlaybackState::STOPPED};
    double playPosition_ = 0.0; // Current playback position in samples
    double playbackRate_ = 1.0; // Current playback rate
    double manualPlaybackRate_ = 1.0; // Manual override rate
    bool useManualRate_ = false;

    // Sample region
    int startSample_ = 0;
    int endSample_ = 0; // 0 means use full sample

    // Looping
    bool loop_ = false;
    int loopStart_ = 0;
    int loopEnd_ = 0; // 0 means use end of sample

    // Pitch and tuning
    int baseNote_ = 60; // Middle C (C4)
    int currentNote_ = 60; // Current MIDI note being played
    int transpose_ = 0; // Transpose in semitones
    float detune_ = 0.0f; // Detune in cents

    // Audio processing
    InterpolationMode interpolationMode_ = InterpolationMode::LINEAR;
    float gain_ = 1.0f;
    float volume_ = 1.0f;

    // Analysis
    std::atomic<float> rmsLevel_{0.0f};
    std::atomic<float> peakLevel_{0.0f};

    // Prepare info
    double engineSampleRate_ = 44100.0;
    int maxBlockSize_ = 1024;
    
    // ADSR envelope pointers (set by PolyphonicSampler from Voice structure)
    class ADSR* amplitudeEnvelope_ = nullptr;
    class ADSR* filterEnvelope_ = nullptr;
    class ADSR* pitchEnvelope_ = nullptr;

    // Private methods
    void updatePlaybackRate();
    float getSampleInterpolated(int channel, double position) const;
    float getSampleLinear(int channel, double position) const;
    float getSampleCubic(int channel, double position) const;
    void handleLooping();
    void updateAnalysis(const choc::buffer::ChannelArrayView<float>& output);
    double noteToFrequencyRatio(int noteA, int noteB) const;
    int clampSamplePosition(int position) const;
    bool isValidSamplePosition(double position) const;
};
