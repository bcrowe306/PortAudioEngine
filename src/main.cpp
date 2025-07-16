#include <iostream>
#include <thread>
#include <chrono>
#include "core/AudioCore.h"
#include "core/Logger.h"
#include "core/PolyphonicSampler.h"
#include "audio/choc_AudioFileFormat_WAV.h"
#include "test_cmajor.h"
#include "OfflineRenderExamples.h"

int main(int, char**){

    // Initialize logging first
    Logger::initialize();
    Logger::setLevel(spdlog::level::debug); // Set to debug level for detailed output
    
    Logger::info("Audio Engine Demo - PolyphonicSampler Integration");
    Logger::info("==================================================");

    // Test if sample file exists
    std::string fileName = "/Users/brandoncrowe/Documents/Audio Samples/BVKER - Elevate Beamaker Kit/Tonal Shots/BVKER - Artifacts Keys 09 - C.wav";
    
    Logger::info("Creating AudioEngine...");
    AudioEngine audioEngine;
    auto graph = audioEngine.getAudioGraph();
    Logger::info("Starting audio stream...");
    audioEngine.startStream(256, 44100);

    // Create and configure PolyphonicSampler
    auto polySampler = std::make_shared<PolyphonicSampler>("MainSampler", 16, VoiceStealingMode::OLDEST);
    
    // Try to load the sample file
    if (polySampler->loadSample(fileName)) {
        Logger::info("Sample loaded successfully into polyphonic sampler!");
        
        // Configure the sampler
        polySampler->setGain(0.8f);
        polySampler->setVolume(0.9f);
        polySampler->setInterpolationMode(SamplePlayerNode::InterpolationMode::LINEAR);
        polySampler->setLoop(false);
        polySampler->setBaseNote(60); // Middle C (assuming the sample is recorded at C)
        
        // Configure ADSR envelope
        // Attack: 100ms, Decay: 200ms, Sustain: 70%, Release: 500ms
        polySampler->setAmplitudeADSR(0.01, 0.2, 0.7, .1);
        polySampler->setAmplitudeADSRCurve(1); // Slight exponential curve
        
        Logger::info("ADSR envelope configured: A=100ms, D=200ms, S=70%, R=500ms");
        
        // Add to audio graph
        graph->addNode(polySampler);
        graph->addOutputNode(polySampler);
        
        // Print sampler information
        polySampler->printSamplerInfo();
        
    } else {
        Logger::warn("Could not load sample file. Using fallback AudioPlayer...");
        
        // Fallback to original AudioPlayer if sample loading fails
        auto player = std::make_shared<AudioPlayer>("AudioPlayer");
        choc::audio::WAVAudioFileFormat<false> wavFormat;
        auto reader = wavFormat.createReader(fileName);
        if (reader) {
            auto data = reader->loadFileContent(audioEngine.getSampleRate());
            player->loadData(data.frames);
            graph->addNode(player);
            graph->addOutputNode(player);
            player->play();
        }
    }

    Logger::info("Creating MidiEngine...");
    MidiEngine midiEngine;
    
    // Configure MIDI callback to trigger polyphonic sampler
    midiEngine.setMidiInputCallback(
        [&](const choc::midi::ShortMessage& message, const std::string& deviceName, unsigned int deviceIndex) 
        {
            // Try to get the polyphonic sampler from the graph
            auto nodes = graph->getNodes();
            for (auto& node : nodes) {
                auto polyphonicSampler = std::dynamic_pointer_cast<PolyphonicSampler>(node);
                if (polyphonicSampler) {
                    // Process MIDI message through the sampler
                    int voiceIndex = polyphonicSampler->processMidiMessage(message);
                    
                    if (message.isNoteOn()) {
                        int noteNumber = static_cast<int>(message.getNoteNumber());
                        int velocity = static_cast<int>(message.getVelocity());
                        Logger::debug("MIDI Note ON: {} (vel: {}) -> Voice: {}", 
                                     noteNumber, velocity, voiceIndex);
                        
                        // Print active voices info every few notes
                        static int noteCount = 0;
                        if (++noteCount % 5 == 0) {
                            polyphonicSampler->printActiveVoicesInfo();
                        }
                    } else if (message.isNoteOff()) {
                        int noteNumber = static_cast<int>(message.getNoteNumber());
                        Logger::debug("MIDI Note OFF: {} -> Voice: {}", noteNumber, voiceIndex);
                    } else if (message.isController()) {
                        int controller = static_cast<int>(message.getControllerNumber());
                        int value = static_cast<int>(message.getControllerValue());
                        Logger::debug("MIDI CC: {} = {}", controller, value);
                    }
                    break;
                }
            }
        });
    
    auto inputDevices = midiEngine.getInputDeviceNames();
    for (const auto& deviceName : inputDevices) {
            Logger::info("Enabling: {}", deviceName);
            midiEngine.enableInputDevice(deviceName);
    }

    std::cin.get(); // Wait for user input to exit
        
        
    return 0;
}
