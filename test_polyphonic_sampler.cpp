#include "src/core/PolyphonicSampler.h"
#include "src/core/Logger.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // Initialize logging
    Logger::initialize();
    Logger::info("=== PolyphonicSampler Test ===");
    
    // Create a polyphonic sampler
    auto polySampler = std::make_unique<PolyphonicSampler>("TestPolySampler", 8, VoiceStealingMode::OLDEST);
    
    // Test basic functionality
    Logger::info("Testing PolyphonicSampler basic functionality:");
    
    // Test without loaded sample
    polySampler->printSamplerInfo();
    
    // Try to play without sample (should warn)
    polySampler->noteOn(60, 100);
    
    // Test parameter settings
    polySampler->setGain(0.8f);
    polySampler->setVolume(0.9f);
    polySampler->setInterpolationMode(SamplePlayerNode::InterpolationMode::LINEAR);
    polySampler->setLoop(false);
    polySampler->setBaseNote(60); // Middle C
    polySampler->setTranspose(0);
    polySampler->setDetune(0.0f);
    
    Logger::info("Testing voice allocation:");
    
    // Simulate note playing (without actual sample for this test)
    Logger::info("Playing some 'virtual' notes to test voice allocation...");
    
    // Play a C major chord
    int chordNotes[] = {60, 64, 67}; // C, E, G
    int velocities[] = {100, 90, 80};
    
    for (int i = 0; i < 3; i++) {
        int voiceIndex = polySampler->noteOn(chordNotes[i], velocities[i]);
        Logger::info("Note {} (vel {}) allocated to voice {}", chordNotes[i], velocities[i], voiceIndex);
    }
    
    polySampler->printActiveVoicesInfo();
    
    // Add more notes to test voice allocation
    Logger::info("Adding more notes...");
    polySampler->noteOn(72, 95); // C5
    polySampler->noteOn(76, 85); // E5
    polySampler->noteOn(79, 75); // G5
    
    polySampler->printActiveVoicesInfo();
    
    // Test note off
    Logger::info("Releasing some notes...");
    polySampler->noteOff(60); // Release C4
    polySampler->noteOff(67); // Release G4
    
    polySampler->printActiveVoicesInfo();
    
    // Test sustain pedal
    Logger::info("Testing sustain pedal...");
    polySampler->setSustainPedal(127); // Sustain on
    polySampler->noteOff(64); // Try to release E4 (should be sustained)
    polySampler->printActiveVoicesInfo();
    
    polySampler->setSustainPedal(0); // Sustain off
    polySampler->printActiveVoicesInfo();
    
    // Test all notes off
    Logger::info("Testing all notes off...");
    polySampler->allNotesOff();
    polySampler->printActiveVoicesInfo();
    
    // Test voice stealing
    Logger::info("Testing voice stealing with many notes...");
    polySampler->setVoiceStealingMode(VoiceStealingMode::OLDEST);
    
    // Play more notes than available voices
    for (int note = 60; note < 75; note++) {
        int voiceIndex = polySampler->noteOn(note, 100 - (note - 60) * 5);
        Logger::info("Note {} -> Voice {}", note, voiceIndex);
        
        if (note % 3 == 0) {
            polySampler->printActiveVoicesInfo();
        }
    }
    
    Logger::info("Final voice state:");
    polySampler->printActiveVoicesInfo();
    polySampler->printSamplerInfo();
    
    Logger::info("=== PolyphonicSampler Test Complete ===");
    
    return 0;
}
