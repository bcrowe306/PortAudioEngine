#include <iostream>
#include <thread>
#include <chrono>
#include "core/AudioCore.h"




void demonstrateMIDI() {
    std::cout << "\n=== MIDI DEVICE DEMO ===\n";
    std::cout << "Enabling all connected MIDI devices and printing incoming messages\n\n";
    
    try {
        MidiEngine midiEngine;
        
        // List available devices
        auto inputDevices = midiEngine.getInputDeviceNames();
        auto outputDevices = midiEngine.getOutputDeviceNames();
        
        std::cout << "Found " << inputDevices.size() << " input device(s):\n";
        for (size_t i = 0; i < inputDevices.size(); ++i) {
            std::cout << "  " << i << ": " << inputDevices[i] << "\n";
        }
        
        std::cout << "Found " << outputDevices.size() << " output device(s):\n";
        for (size_t i = 0; i < outputDevices.size(); ++i) {
            std::cout << "  " << i << ": " << outputDevices[i] << "\n";
        }
        
        if (inputDevices.empty()) {
            std::cout << "\nNo MIDI input devices found. Connect a MIDI device and try again.\n";
            return;
        }
        
        // Set up MIDI input callback to print all incoming messages
        midiEngine.setMidiInputCallback([](const choc::midi::ShortMessage& message, 
                                          const std::string& deviceName, 
                                          unsigned int deviceIndex) {
            std::cout << "[" << deviceName << "] ";
            
            if (message.isNoteOn()) {
                std::cout << "Note ON: " << static_cast<int>(message.getNoteNumber()) 
                         << " velocity " << static_cast<int>(message.getVelocity());
            } else if (message.isNoteOff()) {
                std::cout << "Note OFF: " << static_cast<int>(message.getNoteNumber());
            } else if (message.isController()) {
                std::cout << "CC " << static_cast<int>(message.getControllerNumber()) 
                         << " = " << static_cast<int>(message.getControllerValue());
            } else if (message.isPitchWheel()) {
                std::cout << "Pitch Bend = " << message.getPitchWheelValue();
            } else if (message.isAftertouch()) {
                std::cout << "Aftertouch: " << static_cast<int>(message.getAfterTouchValue());
            } else if (message.isProgramChange()) {
                std::cout << "Program Change: " << static_cast<int>(message.getProgramChangeNumber());
            } else {
                std::cout << "MIDI message (";
                for (size_t i = 0; i < message.length(); ++i) {
                    std::cout << std::hex << static_cast<int>(message.data()[i]);
                    if (i < message.length() - 1) std::cout << " ";
                }
                std::cout << std::dec << ")";
            }
            std::cout << "\n";
        });
        
        // Enable all input devices
        std::cout << "\nEnabling all input devices...\n";
        for (const auto& deviceName : inputDevices) {
          if (deviceName == "MPC Studio mk2 Public"){
            midiEngine.enableInputDevice("MPC Studio mk2 Public");
          }
            
        }
        
       
        
        
        // Listen for MIDI messages indefinitely
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
    } catch (const std::exception& e) {
        std::cout << "MIDI Engine error: " << e.what() << "\n";
    }
}



int main(int, char**){
    std::cout << "Audio Engine Demo - Filter, ADSR, PlayheadNode Integration\n";
    std::cout << "========================================================\n\n";

    try {
        std::cout << "Creating AudioEngine...\n";
        AudioEngine audioEngine;
        
        std::cout << "Starting audio stream...\n";
        audioEngine.startStream(256, 44100);
        
        // First demonstrate MIDI functionality  
        demonstrateMIDI();
        
        std::cout << "\n" << std::string(60, '=') << "\n";
        
    }

    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
