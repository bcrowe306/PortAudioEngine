#include "src/core/SamplePlayerNode.h"
#include "src/core/Logger.h"
#include <iostream>

int main() {
    // Initialize logging
    Logger::initialize();
    Logger::info("=== SamplePlayerNode Test ===");
    
    // Create a sample player node
    auto samplePlayer = std::make_unique<SamplePlayerNode>("TestSampler");
    
    // Test basic functionality
    Logger::info("Testing SamplePlayerNode basic functionality:");
    
    // Test without loaded sample
    samplePlayer->printSampleInfo();
    
    // Try to play without sample (should warn)
    samplePlayer->play();
    
    // Test parameter settings
    samplePlayer->setGain(0.8f);
    samplePlayer->setVolume(0.9f);
    samplePlayer->setInterpolationMode(SamplePlayerNode::InterpolationMode::LINEAR);
    samplePlayer->setLoop(true);
    samplePlayer->setBaseNote(60); // Middle C
    samplePlayer->setTranspose(7);  // Transpose up a fifth
    samplePlayer->setDetune(10.0f); // 10 cents sharp
    
    Logger::info("Gain: {:.2f}, Volume: {:.2f}", samplePlayer->getGain(), samplePlayer->getVolume());
    Logger::info("Loop: {}, Base Note: {}, Transpose: {}, Detune: {:.1f}c", 
                samplePlayer->isLooping(), samplePlayer->getBaseNote(), 
                samplePlayer->getTranspose(), samplePlayer->getDetune());
    
    // Test MIDI note conversion
    Logger::info("Testing MIDI note triggering:");
    samplePlayer->setCurrentNote(67); // G4
    Logger::info("Current note set to {} (G4)", samplePlayer->getCurrentNote());
    
    // Test sample region settings
    samplePlayer->setSampleRegion(1000, 5000);
    samplePlayer->setLoopRegion(2000, 4000);
    Logger::info("Sample region: {} - {}, Loop region: {} - {}", 
                samplePlayer->getStartSample(), samplePlayer->getEndSample(),
                samplePlayer->getLoopStart(), samplePlayer->getLoopEnd());
    
    // Test playback state
    Logger::info("Testing playback states:");
    Logger::info("Initial state: {}", static_cast<int>(samplePlayer->getPlaybackState()));
    
    samplePlayer->trigger(60); // Trigger with middle C
    Logger::info("After trigger: {}", static_cast<int>(samplePlayer->getPlaybackState()));
    
    samplePlayer->pause();
    Logger::info("After pause: {}", static_cast<int>(samplePlayer->getPlaybackState()));
    
    samplePlayer->resume();
    Logger::info("After resume: {}", static_cast<int>(samplePlayer->getPlaybackState()));
    
    samplePlayer->stop();
    Logger::info("After stop: {}", static_cast<int>(samplePlayer->getPlaybackState()));
    
    // Test audio levels
    Logger::info("Peak level: {:.3f}, RMS level: {:.3f}", 
                samplePlayer->getPeakLevel(), samplePlayer->getRMSLevel());
    
    Logger::info("=== SamplePlayerNode Test Complete ===");
    
    return 0;
}
