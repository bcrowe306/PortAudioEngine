#include <iostream>
#include <thread>
#include <chrono>
#include "core/AudioCore.h"
#include "core/Logger.h"
#include "audio/choc_AudioFileFormat_WAV.h"
#include "test_cmajor.h"
#include "OfflineRenderExamples.h"

int main(int, char**){

    // Initialize logging first
    Logger::initialize();
    Logger::setLevel(spdlog::level::debug); // Set to debug level for detailed output
    
    Logger::info("Audio Engine Demo - Filter, ADSR, PlayheadNode Integration");
    Logger::info("========================================================");


    std::string fileName = "/Users/brandoncrowe/Documents/Audio Samples/DECAP - "
                            "Drums That Knock X/Hihats/DECAP hihat chonky.wav";
    Logger::info("Creating AudioEngine...");
    AudioEngine audioEngine;
    auto graph = audioEngine.getAudioGraph();
    Logger::info("Starting audio stream...");
    audioEngine.startStream(256, 44100);

    auto player = std::make_shared<AudioPlayer>("AudioPlayer");
    auto sine = std::make_shared<CmajorTest>("SineGenerator");
    choc::audio::WAVAudioFileFormat<false> wavFormat;
    choc::buffer::ChannelArrayBuffer<float> audioBuffer;
    auto reader = wavFormat.createReader(fileName);
    auto data = reader->loadFileContent(audioEngine.getSampleRate());

    player->loadData(data.frames);

    graph->addNode(player);
    graph->addOutputNode(player);
    graph->addNode(sine);
    graph->addOutputNode(sine);
    
    player->play();

    Logger::info("Creating MidiEngine...");
    MidiEngine midiEngine;
    midiEngine.setMidiInputCallback([&](const choc::midi::ShortMessage& message, const std::string& deviceName, 
                                       unsigned int deviceIndex) {
        Logger::info("[{}] {}", deviceName, message.toHexString());
        if (message.isNoteOn()) {
            player->play();
        }
    });
    auto inputDevices = midiEngine.getInputDeviceNames();
    Logger::info("Enabling MPC Studio mk2 Public input device...");
    for (const auto& deviceName : inputDevices) {
        if (deviceName == "MPC Studio mk2 Public"){
        midiEngine.enableInputDevice("MPC Studio mk2 Public");
        }
        
    }

    std::cin.get(); // Wait for user input to exit
        
        
    return 0;
}
