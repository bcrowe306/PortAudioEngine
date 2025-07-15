#include "MidiEngine.h"
#include "Logger.h"
#include <algorithm>
#include <stdexcept>
#include "Spinlock.h"

MidiEngine::MidiEngine() {
    Logger::info("MidiEngine initializing...");
    
    try {
        // Scan for available devices on startup
        scanDevices();
        initialized.store(true);
        
        Logger::info("MidiEngine initialized successfully");
        Logger::info("Found {} input devices and {} output devices", 
                    inputDevices.size(), outputDevices.size());
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize MidiEngine: {}", e.what());
        throw;
    }
}

MidiEngine::~MidiEngine() {
    Logger::info("MidiEngine shutting down...");
    
    // Disable all devices (this closes connections)
    {
      

      for (auto &device : inputDevices) {
        if (device->enabled && device->rtMidiIn) {
          device->rtMidiIn->closePort();
          device->enabled = false;
        }
        }
        
        for (auto& device : outputDevices) {
            if (device->enabled && device->rtMidiOut) {
                device->rtMidiOut->closePort();
                device->enabled = false;
            }
        }
    }
    
    // Clear control surfaces
    clearControlSurfaces();
    
    // Clear callback
    clearMidiInputCallback();
    
    Logger::info("MidiEngine shutdown complete");
}

void MidiEngine::scanDevices() {
    
    
    Logger::debug("Scanning for MIDI devices...");
    
    // Clear existing devices
    inputDevices.clear();
    outputDevices.clear();
    callbackData.clear();
    
    try {
        // Scan input devices
        {
            RtMidiIn tempMidiIn;
            unsigned int inputCount = tempMidiIn.getPortCount();
            
            Logger::debug("Found ", inputCount, " MIDI input ports");
            
            for (unsigned int i = 0; i < inputCount; ++i) {
                try {
                    std::string deviceName = tempMidiIn.getPortName(i);
                    auto device = std::make_unique<InputDevice>(deviceName, i);
                    
                    Logger::debug("Input device ", i, ": '", deviceName, "'");
                    inputDevices.push_back(std::move(device));
                } catch (const std::exception& e) {
                    Logger::warn("Failed to get input device ", i, " info: ", e.what());
                }
            }
        }
        
        // Scan output devices
        {
            RtMidiOut tempMidiOut;
            unsigned int outputCount = tempMidiOut.getPortCount();
            
            Logger::debug("Found ", outputCount, " MIDI output ports");
            
            for (unsigned int i = 0; i < outputCount; ++i) {
                try {
                    std::string deviceName = tempMidiOut.getPortName(i);
                    auto device = std::make_unique<OutputDevice>(deviceName, i);
                    
                    Logger::debug("Output device ", i, ": '", deviceName, "'");
                    outputDevices.push_back(std::move(device));
                } catch (const std::exception& e) {
                    Logger::warn("Failed to get output device ", i, " info: ", e.what());
                }
            }
        }
        
    } catch (const std::exception& e) {
        Logger::error("Error during MIDI device scan: ", e.what());
        throw;
    }
}

bool MidiEngine::enableInputDevice(const std::string& deviceName) {
    
    
    auto* device = findInputDevice(deviceName);
    if (!device) {
        Logger::warn("Input device '", deviceName, "' not found");
        return false;
    }
    
    return enableInputDevice(device->index);
}

bool MidiEngine::enableInputDevice(unsigned int deviceIndex) {
    
    
    if (deviceIndex >= inputDevices.size()) {
        Logger::warn("Input device index ", deviceIndex, " out of range");
        return false;
    }
    
    auto& device = inputDevices[deviceIndex];
    
    if (device->enabled) {
        Logger::debug("Input device '", device->name, "' already enabled");
        return true;
    }
    
    try {
        // Create RtMidiIn instance
        device->rtMidiIn = std::make_unique<RtMidiIn>();
        
        // Create callback data
        auto callbackDataPtr = std::make_unique<MidiInputCallbackData>(this, device->name, device->index);
        
        // Set up callback
        device->rtMidiIn->setCallback(&MidiEngine::rtMidiInputCallback, callbackDataPtr.get());
        
        // Open the port
        device->rtMidiIn->openPort(device->index, device->name);
        
        // Don't ignore any message types for now (let control surfaces handle filtering)
        device->rtMidiIn->ignoreTypes(false, false, false);
        
        device->enabled = true;
        callbackData.push_back(std::move(callbackDataPtr));
        
        Logger::info("Enabled MIDI input device '", device->name, "'");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to enable input device '", device->name, "': ", e.what());
        device->rtMidiIn.reset();
        device->enabled = false;
        return false;
    }
}

bool MidiEngine::disableInputDevice(const std::string& deviceName) {
    
    
    auto* device = findInputDevice(deviceName);
    if (!device) {
        Logger::warn("Input device '", deviceName, "' not found");
        return false;
    }
    
    return disableInputDevice(device->index);
}

bool MidiEngine::disableInputDevice(unsigned int deviceIndex) {
    
    
    if (deviceIndex >= inputDevices.size()) {
        Logger::warn("Input device index ", deviceIndex, " out of range");
        return false;
    }
    
    auto& device = inputDevices[deviceIndex];
    
    if (!device->enabled) {
        Logger::debug("Input device '", device->name, "' already disabled");
        return true;
    }
    
    try {
        if (device->rtMidiIn) {
            device->rtMidiIn->closePort();
            device->rtMidiIn.reset();
        }
        
        device->enabled = false;
        
        // Remove callback data
        callbackData.erase(
            std::remove_if(callbackData.begin(), callbackData.end(),
                [deviceIndex](const std::unique_ptr<MidiInputCallbackData>& data) {
                    return data->deviceIndex == deviceIndex;
                }),
            callbackData.end());
        
        Logger::info("Disabled MIDI input device '", device->name, "'");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to disable input device '", device->name, "': ", e.what());
        return false;
    }
}

bool MidiEngine::enableOutputDevice(const std::string& deviceName) {
    
    
    auto* device = findOutputDevice(deviceName);
    if (!device) {
        Logger::warn("Output device '", deviceName, "' not found");
        return false;
    }
    
    return enableOutputDevice(device->index);
}

bool MidiEngine::enableOutputDevice(unsigned int deviceIndex) {
    
    
    if (deviceIndex >= outputDevices.size()) {
        Logger::warn("Output device index ", deviceIndex, " out of range");
        return false;
    }
    
    auto& device = outputDevices[deviceIndex];
    
    if (device->enabled) {
        Logger::debug("Output device '", device->name, "' already enabled");
        return true;
    }
    
    try {
        // Create RtMidiOut instance
        device->rtMidiOut = std::make_unique<RtMidiOut>();
        
        // Open the port
        device->rtMidiOut->openPort(device->index, device->name);
        
        device->enabled = true;
        
        Logger::info("Enabled MIDI output device '", device->name, "'");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to enable output device '", device->name, "': ", e.what());
        device->rtMidiOut.reset();
        device->enabled = false;
        return false;
    }
}

bool MidiEngine::disableOutputDevice(const std::string& deviceName) {
    
    
    auto* device = findOutputDevice(deviceName);
    if (!device) {
        Logger::warn("Output device '", deviceName, "' not found");
        return false;
    }
    
    return disableOutputDevice(device->index);
}

bool MidiEngine::disableOutputDevice(unsigned int deviceIndex) {
    
    
    if (deviceIndex >= outputDevices.size()) {
        Logger::warn("Output device index ", deviceIndex, " out of range");
        return false;
    }
    
    auto& device = outputDevices[deviceIndex];
    
    if (!device->enabled) {
        Logger::debug("Output device '", device->name, "' already disabled");
        return true;
    }
    
    try {
        if (device->rtMidiOut) {
            device->rtMidiOut->closePort();
            device->rtMidiOut.reset();
        }
        
        device->enabled = false;
        
        Logger::info("Disabled MIDI output device '", device->name, "'");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to disable output device '", device->name, "': ", e.what());
        return false;
    }
}

void MidiEngine::setMidiInputCallback(MidiInputCallback callback) {
    SpinLockGuard lock(callbackLock);
    userMidiCallback = std::move(callback);
    Logger::debug("MIDI input callback set");
}

void MidiEngine::clearMidiInputCallback() {
    SpinLockGuard lock(callbackLock);
    userMidiCallback = nullptr;
    Logger::debug("MIDI input callback cleared");
}

bool MidiEngine::sendMidiMessage(const choc::midi::ShortMessage& message, const std::string& deviceName) {
    
    
    auto* device = findOutputDevice(deviceName);
    if (!device || !device->enabled || !device->rtMidiOut) {
        Logger::warn("Output device '", deviceName, "' not available for sending");
        return false;
    }
    
    return sendMidiMessage(message, device->index);
}

bool MidiEngine::sendMidiMessage(const choc::midi::ShortMessage& message, unsigned int deviceIndex) {
    
    
    if (deviceIndex >= outputDevices.size()) {
        Logger::warn("Output device index ", deviceIndex, " out of range");
        return false;
    }
    
    auto& device = outputDevices[deviceIndex];
    
    if (!device->enabled || !device->rtMidiOut) {
        Logger::warn("Output device '", device->name, "' not enabled for sending");
        return false;
    }
    
    try {
        // Convert CHOC message to RtMidi format
        std::vector<unsigned char> rtMidiMessage;
        
        // Get the raw MIDI data from CHOC message
        auto messageData = message.data();
        for (size_t i = 0; i < message.length(); ++i) {
            rtMidiMessage.push_back(messageData[i]);
        }
        
        // Send the message
        device->rtMidiOut->sendMessage(&rtMidiMessage);
        
        Logger::debug("Sent MIDI message to device '", device->name, "' - ", 
                     rtMidiMessage.size(), " bytes");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to send MIDI message to device '", device->name, "': ", e.what());
        return false;
    }
}

bool MidiEngine::broadcastMidiMessage(const choc::midi::ShortMessage& message) {
    
    
    bool success = true;
    
    for (const auto& device : outputDevices) {
        if (device->enabled && device->rtMidiOut) {
            if (!sendMidiMessage(message, device->index)) {
                success = false;
            }
        }
    }
    
    return success;
}

void MidiEngine::registerControlSurface(std::shared_ptr<ControlSurface> surface) {
    if (!surface) {
        Logger::warn("Attempted to register null control surface");
        return;
    }
    
    SpinLockGuard lock(controlSurfaceLock);
    
    // Check if already registered
    auto it = std::find(controlSurfaces.begin(), controlSurfaces.end(), surface);
    if (it != controlSurfaces.end()) {
        Logger::warn("Control surface '", surface->getName(), "' is already registered");
        return;
    }
    
    controlSurfaces.push_back(surface);
    surface->onRegistered();
    
    Logger::info("Registered control surface '", surface->getName(), "'");
}

void MidiEngine::unregisterControlSurface(std::shared_ptr<ControlSurface> surface) {
    if (!surface) {
        Logger::warn("Attempted to unregister null control surface");
        return;
    }
    
    SpinLockGuard lock(controlSurfaceLock);
    
    auto it = std::find(controlSurfaces.begin(), controlSurfaces.end(), surface);
    if (it != controlSurfaces.end()) {
        surface->onUnregistered();
        controlSurfaces.erase(it);
        Logger::info("Unregistered control surface '", surface->getName(), "'");
    } else {
        Logger::warn("Control surface '", surface->getName(), "' was not registered");
    }
}

void MidiEngine::clearControlSurfaces() {
    SpinLockGuard lock(controlSurfaceLock);
    
    for (auto& surface : controlSurfaces) {
        surface->onUnregistered();
    }
    
    controlSurfaces.clear();
    Logger::debug("Cleared all control surfaces");
}

std::vector<std::string> MidiEngine::getInputDeviceNames() const {
    
    
    std::vector<std::string> names;
    for (const auto& device : inputDevices) {
        names.push_back(device->name);
    }
    return names;
}

std::vector<std::string> MidiEngine::getOutputDeviceNames() const {
    
    
    std::vector<std::string> names;
    for (const auto& device : outputDevices) {
        names.push_back(device->name);
    }
    return names;
}

std::vector<std::string> MidiEngine::getEnabledInputDeviceNames() const {
    
    
    std::vector<std::string> names;
    for (const auto& device : inputDevices) {
        if (device->enabled) {
            names.push_back(device->name);
        }
    }
    return names;
}

std::vector<std::string> MidiEngine::getEnabledOutputDeviceNames() const {
    
    
    std::vector<std::string> names;
    for (const auto& device : outputDevices) {
        if (device->enabled) {
            names.push_back(device->name);
        }
    }
    return names;
}

MidiEngine::InputDevice* MidiEngine::findInputDevice(const std::string& deviceName) {
    auto it = std::find_if(inputDevices.begin(), inputDevices.end(),
        [&deviceName](const std::unique_ptr<InputDevice>& device) {
            return device->name == deviceName;
        });
    
    return (it != inputDevices.end()) ? it->get() : nullptr;
}

MidiEngine::OutputDevice* MidiEngine::findOutputDevice(const std::string& deviceName) {
    auto it = std::find_if(outputDevices.begin(), outputDevices.end(),
        [&deviceName](const std::unique_ptr<OutputDevice>& device) {
            return device->name == deviceName;
        });
    
    return (it != outputDevices.end()) ? it->get() : nullptr;
}

void MidiEngine::rtMidiInputCallback(double timeStamp, std::vector<unsigned char>* message, void* userData) {
    if (!message || !userData) {
        return;
    }
    
    auto* callbackData = static_cast<MidiInputCallbackData*>(userData);
    if (!callbackData->engine) {
        return;
    }
    
    callbackData->engine->handleMidiInput(timeStamp, *message, 
                                        callbackData->deviceName, 
                                        callbackData->deviceIndex);
}

void MidiEngine::handleMidiInput(double timeStamp, const std::vector<unsigned char>& rawMessage, 
                                const std::string& deviceName, unsigned int deviceIndex) {
    // Convert raw MIDI to CHOC message
    Logger::debug("Received MIDI input from '{}' at time {}", deviceName, timeStamp);
    if (rawMessage.empty()) {
        return;
    }
    
    try {
        // Create CHOC MIDI message from raw data
        choc::midi::ShortMessage chocMessage(rawMessage[0], rawMessage[1], rawMessage[2]);
        
        
        // Process through control surface chain first
        bool handled = processControlSurfaceChain(chocMessage, deviceName, deviceIndex);
        
        // If not handled by control surfaces, send to user callback
        if (!handled) {
            // SpinLockGuard lock(callbackLock);
            if (userMidiCallback) {
                userMidiCallback(chocMessage, deviceName, deviceIndex);
            }
        }
        
        Logger::debug("MIDI input from '", deviceName, "': ", 
                     rawMessage.size(), " bytes, handled=", handled);
        
    } catch (const std::exception& e) {
        Logger::error("Error processing MIDI input from '", deviceName, "': ", e.what());
    }
}

bool MidiEngine::processControlSurfaceChain(const choc::midi::ShortMessage& message, 
                                          const std::string& deviceName, 
                                          unsigned int deviceIndex) {
    SpinLockGuard lock(controlSurfaceLock);
    
    // Process through control surfaces in registration order
    for (auto& surface : controlSurfaces) {
        try {
            if (surface->handleMidiMessage(message, deviceName, deviceIndex)) {
                Logger::debug("MIDI message handled by control surface '", surface->getName(), "'");
                return true; // Message was handled, stop processing
            }
        } catch (const std::exception& e) {
            Logger::error("Error in control surface '", surface->getName(), "': ", e.what());
            // Continue processing with other surfaces despite error
        }
    }
    
    return false; // Message not handled by any control surface
}
