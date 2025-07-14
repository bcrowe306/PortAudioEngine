#include <iostream>
#include <thread>
#include <chrono>
#include "core/AudioCore.h"
#include "audio/choc_AudioFileFormat_WAV.h"
#include "test_cmajor.h"

int main(int, char**){

    std::cout << "Audio Engine Demo - Filter, ADSR, PlayheadNode Integration\n";
    std::cout << "========================================================\n\n";


    std::string fileName = "/Users/bcrowe/Documents/Audio Samples/DECAP - "
                            "Drums That Knock X/Hihats/DECAP hihat chonky.wav";
    std::cout << "Creating AudioEngine...\n";
    AudioEngine audioEngine;
    auto graph = audioEngine.getAudioGraph();
    std::cout << "Starting audio stream...\n";
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


    std::cout << "Creating MidiEngine...\n";
    MidiEngine midiEngine;
    midiEngine.setMidiInputCallback([&](const choc::midi::ShortMessage& message, const std::string& deviceName, 
                                       unsigned int deviceIndex) {
        std::cout << "[" << deviceName << "] " << message.toHexString() << std::endl;
        if (message.isNoteOn()) {
            player->play();
        }
    });
    auto inputDevices = midiEngine.getInputDeviceNames();
    std::cout << "\nEnabling MPC Studio mk2 Public input device...\n";
    for (const auto& deviceName : inputDevices) {
        if (deviceName == "MPC Studio mk2 Public"){
        midiEngine.enableInputDevice("MPC Studio mk2 Public");
        }
        
    }

    std::cin.get(); // Wait for user input to exit
        
        
    return 0;
}
