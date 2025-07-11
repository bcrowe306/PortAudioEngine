#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "core/PlayheadNode.h"

void printPosition(const SongPosition& pos) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "BPM: " << pos.bpm 
              << " | Time Sig: " << pos.timeSignatureNumerator << "/" << pos.timeSignatureDenominator
              << " | Bar: " << pos.currentBar 
              << " | Beat: " << pos.currentBeat 
              << " | 16th: " << pos.currentSixteenth
              << " | Beats: " << pos.songPositionInBeats
              << " | Ticks: " << pos.songPositionInTicks
              << " | Samples: " << pos.songPositionInSamples
              << std::endl;
}

int main() {
    std::cout << "Testing PlayheadNode..." << std::endl;
    
    // Create playhead
    auto playhead = std::make_shared<PlayheadNode>();
    
    // Setup audio parameters
    AudioNode::PrepareInfo info;
    info.sampleRate = 44100.0;
    info.maxBufferSize = 512;
    info.numChannels = 2;
    
    playhead->prepare(info);
    
    std::cout << "\nInitial position:" << std::endl;
    printPosition(playhead->getCurrentPosition());
    
    // Test tempo and time signature changes
    std::cout << "\nSetting BPM to 140:" << std::endl;
    playhead->setBpm(140.0);
    printPosition(playhead->getCurrentPosition());
    
    std::cout << "\nSetting time signature to 3/4:" << std::endl;
    playhead->setTimeSignature(3, 4);
    printPosition(playhead->getCurrentPosition());
    
    // Test position jumps
    std::cout << "\nJumping to beat 8.5:" << std::endl;
    playhead->jumpToPosition(8.5);
    printPosition(playhead->getCurrentPosition());
    
    std::cout << "\nJumping to bar 3, beat 2:" << std::endl;
    playhead->jumpToPosition(3, 2);
    printPosition(playhead->getCurrentPosition());
    
    // Test playback simulation
    std::cout << "\nStarting playback simulation..." << std::endl;
    playhead->jumpToPosition(static_cast<int64_t>(0));
    playhead->play();
    
    // Simulate audio processing
    const int bufferSize = 512;
    const int numBuffers = 10; // Process several buffers
    
    for (int i = 0; i < numBuffers; ++i) {
        // Simulate audio callback
        playhead->processCallback(nullptr, nullptr, 0, 0, bufferSize, info.sampleRate, bufferSize);
        
        std::cout << "Buffer " << (i + 1) << ": ";
        printPosition(playhead->getCurrentPosition());
        
        // Small delay to simulate real-time processing
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Test pause/resume
    std::cout << "\nPausing..." << std::endl;
    playhead->pause();
    
    // Process a few more buffers while paused
    for (int i = 0; i < 3; ++i) {
        playhead->processCallback(nullptr, nullptr, 0, 0, bufferSize, info.sampleRate, bufferSize);
        std::cout << "Paused buffer " << (i + 1) << ": ";
        printPosition(playhead->getCurrentPosition());
    }
    
    std::cout << "\nResuming..." << std::endl;
    playhead->play();
    
    // Process a few more buffers
    for (int i = 0; i < 3; ++i) {
        playhead->processCallback(nullptr, nullptr, 0, 0, bufferSize, info.sampleRate, bufferSize);
        std::cout << "Resumed buffer " << (i + 1) << ": ";
        printPosition(playhead->getCurrentPosition());
    }
    
    // Test stop
    std::cout << "\nStopping..." << std::endl;
    playhead->stop();
    printPosition(playhead->getCurrentPosition());
    
    std::cout << "\nPlayheadNode test completed!" << std::endl;
    
    return 0;
}
