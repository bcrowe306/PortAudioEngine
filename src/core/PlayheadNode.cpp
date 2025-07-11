#include "PlayheadNode.h"
#include <algorithm>

PlayheadNode::PlayheadNode() : AudioNode("Playhead") {
    updateCachedValues();
}

void PlayheadNode::prepare(const PrepareInfo& info) {
    AudioNode::prepare(info);
    updateCachedValues();
}

void PlayheadNode::processCallback(
    const float* const* inputBuffers,
    float* const* outputBuffers,
    int numInputChannels,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize
) {
    // Check for pending position updates (from non-real-time thread)
    if (positionUpdateFlag.load()) {
        songPosition = pendingPosition;
        positionUpdateFlag.store(false);
        updateMusicalPosition();
    }
    
    // Only advance time if playing and not paused
    if (playing.load() && !paused.load()) {
        // Store previous values for change detection
        int64_t prevTick = songPosition.songPositionInTicks;
        int64_t prevSample = songPosition.songPositionInSamples;
        int prevBeat = songPosition.currentBeat;
        int prevBar = songPosition.currentBar;
        
        // Process each sample in the buffer to ensure precise timing
        for (int sample = 0; sample < numSamples; ++sample) {
            // Advance position by one sample
            songPosition.songPositionInSamples++;
            
            // Call sample callback if registered
            if (sampleCallback && lastSample != songPosition.songPositionInSamples) {
                sampleCallback(songPosition, songPosition.songPositionInSamples);
                lastSample = songPosition.songPositionInSamples;
            }
            
            // Update tick position
            int64_t newTick = samplesToTicks(songPosition.songPositionInSamples);
            
            // Check for tick change and call tick callback
            if (tickCallback && newTick != songPosition.songPositionInTicks) {
                songPosition.songPositionInTicks = newTick;
                songPosition.songPositionInBeats = ticksToBeats(songPosition.songPositionInTicks);
                updateMusicalPosition();
                
                if (lastTick != newTick) {
                    tickCallback(songPosition, newTick);
                    lastTick = newTick;
                }
            } else {
                // Update position even if no callback
                songPosition.songPositionInTicks = newTick;
                songPosition.songPositionInBeats = ticksToBeats(songPosition.songPositionInTicks);
                updateMusicalPosition();
            }
            
            // Check for beat change and call beat callback
            if (beatCallback && songPosition.currentBeat != lastBeat) {
                beatCallback(songPosition, songPosition.currentBeat, songPosition.currentBar);
                lastBeat = songPosition.currentBeat;
            }
            
            // Check for bar change and call bar callback
            if (barCallback && songPosition.currentBar != lastBar) {
                barCallback(songPosition, songPosition.currentBar);
                lastBar = songPosition.currentBar;
            }
        }
    }
    
    // This node doesn't produce audio output, just timing information
    // Clear output buffers if any are provided
    
}

void PlayheadNode::play() {
    paused.store(false);
    playing.store(true);
}

void PlayheadNode::stop() {
    playing.store(false);
    paused.store(false);
    // Reset position to beginning
    jumpToPosition(static_cast<int64_t>(0));
}

void PlayheadNode::pause() {
    paused.store(true);
}

void PlayheadNode::jumpToPosition(int64_t ticks) {
    pendingPosition = songPosition;
    pendingPosition.songPositionInTicks = std::max(0LL, ticks);
    pendingPosition.songPositionInSamples = ticksToSamples(pendingPosition.songPositionInTicks);
    pendingPosition.songPositionInBeats = ticksToBeats(pendingPosition.songPositionInTicks);
    
    // Calculate new musical position
    calculateMusicalPosition(pendingPosition.songPositionInTicks, 
                           pendingPosition.currentBar,
                           pendingPosition.currentBeat,
                           pendingPosition.currentSixteenth,
                           pendingPosition.songPositionInBeats);
    
    positionUpdateFlag.store(true);
}

void PlayheadNode::jumpToPosition(double beats) {
    int64_t ticks = beatsToTicks(beats);
    jumpToPosition(ticks);
}

void PlayheadNode::jumpToPosition(int bar, int beat) {
    // Convert bar and beat to total beats
    double totalBeats = (bar - 1) * songPosition.timeSignatureNumerator + (beat - 1);
    jumpToPosition(totalBeats);
}

void PlayheadNode::jumpToSample(int64_t samples) {
    int64_t ticks = samplesToTicks(samples);
    jumpToPosition(ticks);
}

void PlayheadNode::setBpm(double newBpm) {
    if (newBpm > 0.0) {
        pendingPosition = songPosition;
        pendingPosition.bpm = newBpm;
        positionUpdateFlag.store(true);
        updateCachedValues();
    }
}

void PlayheadNode::setTimeSignature(int numerator, int denominator) {
    if (numerator > 0 && denominator > 0) {
        pendingPosition = songPosition;
        pendingPosition.timeSignatureNumerator = numerator;
        pendingPosition.timeSignatureDenominator = denominator;
        positionUpdateFlag.store(true);
        updateCachedValues();
    }
}

void PlayheadNode::updateCachedValues() {
    if (currentPrepareInfo.sampleRate > 0.0) {
        // Calculate samples per beat (quarter note)
        samplesPerBeat = (60.0 / songPosition.bpm) * currentPrepareInfo.sampleRate;
        
        // Calculate ticks per sample and samples per tick
        ticksPerBeat = SongPosition::TICKS_PER_QUARTER_NOTE;
        ticksPerSample = ticksPerBeat / samplesPerBeat;
        samplesPerTick = samplesPerBeat / ticksPerBeat;
        
        // Calculate samples per bar
        double beatsPerBar = (4.0 / songPosition.timeSignatureDenominator) * songPosition.timeSignatureNumerator;
        samplesPerBar = samplesPerBeat * beatsPerBar;
    }
}

void PlayheadNode::updateMusicalPosition() {
    calculateMusicalPosition(songPosition.songPositionInTicks,
                           songPosition.currentBar,
                           songPosition.currentBeat,
                           songPosition.currentSixteenth,
                           songPosition.songPositionInBeats);
}

void PlayheadNode::calculateMusicalPosition(int64_t ticks, int& bar, int& beat, int& sixteenth, double& beatTime) {
    // Convert ticks to beats
    beatTime = ticksToBeats(ticks);
    
    // Calculate beats per bar based on time signature
    double beatsPerBar = (4.0 / songPosition.timeSignatureDenominator) * songPosition.timeSignatureNumerator;
    
    // Calculate bar and beat within bar
    double totalBars = beatTime / beatsPerBar;
    bar = static_cast<int>(std::floor(totalBars)) + 1; // 1-based
    
    double beatInBar = std::fmod(beatTime, beatsPerBar);
    beat = static_cast<int>(std::floor(beatInBar)) + 1; // 1-based
    
    // Calculate sixteenth note position
    // In 4/4 time, there are 16 sixteenth notes per bar
    // In other time signatures, scale accordingly
    double sixteenthsPerBar = beatsPerBar * 4.0; // 4 sixteenths per beat
    double sixteenthInBar = std::fmod(beatTime * 4.0, sixteenthsPerBar);
    sixteenth = static_cast<int>(std::floor(sixteenthInBar)) + 1; // 1-based
    
    // Clamp to valid ranges
    beat = std::max(1, std::min(beat, songPosition.timeSignatureNumerator));
    sixteenth = std::max(1, std::min(sixteenth, static_cast<int>(sixteenthsPerBar)));
}

double PlayheadNode::ticksToBeats(int64_t ticks) const {
    return static_cast<double>(ticks) / SongPosition::TICKS_PER_QUARTER_NOTE;
}

int64_t PlayheadNode::beatsToTicks(double beats) const {
    return static_cast<int64_t>(beats * SongPosition::TICKS_PER_QUARTER_NOTE);
}

int64_t PlayheadNode::samplesToTicks(int64_t samples) const {
    if (samplesPerTick > 0.0) {
        return static_cast<int64_t>(samples / samplesPerTick);
    }
    return 0;
}

int64_t PlayheadNode::ticksToSamples(int64_t ticks) const {
    return static_cast<int64_t>(ticks * samplesPerTick);
}
