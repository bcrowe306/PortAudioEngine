#pragma once

#include "rtmidi/RtMidi.h"
#include "audio/choc_MIDI.h"
#include "threading/choc_SpinLock.h"
#include "Logger.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <mutex>

/**
 * MidiEngine - Comprehensive MIDI input/output management with control surface support
 * 
 * This engine provides:
 * - Device discovery and management for MIDI input/output
 * - Real-time MIDI message processing with callbacks
 * - Control surface chain for message filtering/handling
 * - Thread-safe operation using CHOC utilities
 * - CHOC MIDI ShortMessage integration
 */
class MidiEngine {
public:
    // ===== DEVICE STRUCTURES =====
    
    /**
     * Structure representing a MIDI input device
     */
    struct InputDevice {
        std::string name;
        unsigned int index;
        bool enabled = false;
        std::unique_ptr<RtMidiIn> rtMidiIn;
        
        InputDevice(const std::string& deviceName, unsigned int deviceIndex)
            : name(deviceName), index(deviceIndex) {}
    };
    
    /**
     * Structure representing a MIDI output device
     */
    struct OutputDevice {
        std::string name;
        unsigned int index;
        bool enabled = false;
        std::unique_ptr<RtMidiOut> rtMidiOut;
        
        OutputDevice(const std::string& deviceName, unsigned int deviceIndex)
            : name(deviceName), index(deviceIndex) {}
    };

    // ===== CALLBACK TYPES =====
    
    /**
     * Callback data structure for MIDI input callbacks
     */
    struct MidiInputCallbackData {
        MidiEngine* engine;
        std::string deviceName;
        unsigned int deviceIndex;
        
        MidiInputCallbackData(MidiEngine* eng, const std::string& name, unsigned int index)
            : engine(eng), deviceName(name), deviceIndex(index) {}
    };
    
    /**
     * User callback function type for MIDI input
     * @param message The MIDI message received
     * @param deviceName Name of the device that sent the message
     * @param deviceIndex Index of the device that sent the message
     */
    using MidiInputCallback = std::function<void(const choc::midi::ShortMessage& message, 
                                                const std::string& deviceName, 
                                                unsigned int deviceIndex)>;

    // ===== CONTROL SURFACE SUPPORT =====
    
    /**
     * Base class for control surfaces
     * Control surfaces can intercept and handle MIDI messages before they reach the application
     */
    class ControlSurface {
    public:
        virtual ~ControlSurface() = default;
        
        /**
         * Handle a MIDI message
         * @param message The MIDI message to potentially handle
         * @param deviceName Name of the source device
         * @param deviceIndex Index of the source device
         * @return true if the message was handled (stops further processing), false to continue
         */
        virtual bool handleMidiMessage(const choc::midi::ShortMessage& message,
                                     const std::string& deviceName,
                                     unsigned int deviceIndex) = 0;
        
        /**
         * Get the name of this control surface
         */
        virtual std::string getName() const = 0;
        
        /**
         * Called when the control surface is registered
         */
        virtual void onRegistered() {}
        
        /**
         * Called when the control surface is unregistered
         */
        virtual void onUnregistered() {}
    };

    // ===== CONSTRUCTOR/DESTRUCTOR =====
    
    MidiEngine();
    virtual ~MidiEngine();

    // Non-copyable
    MidiEngine(const MidiEngine&) = delete;
    MidiEngine& operator=(const MidiEngine&) = delete;

    // ===== DEVICE MANAGEMENT =====
    
    /**
     * Scan for available MIDI devices and update internal lists
     */
    void scanDevices();
    
    /**
     * Get list of available input devices
     */
    const std::vector<std::unique_ptr<InputDevice>>& getInputDevices() const { return inputDevices; }
    
    /**
     * Get list of available output devices
     */
    const std::vector<std::unique_ptr<OutputDevice>>& getOutputDevices() const { return outputDevices; }
    
    /**
     * Enable a MIDI input device by name
     * @param deviceName Name of the device to enable
     * @return true if successful, false if device not found
     */
    bool enableInputDevice(const std::string& deviceName);
    
    /**
     * Enable a MIDI input device by index
     * @param deviceIndex Index of the device to enable
     * @return true if successful, false if device not found
     */
    bool enableInputDevice(unsigned int deviceIndex);
    
    /**
     * Disable a MIDI input device by name
     */
    bool disableInputDevice(const std::string& deviceName);
    
    /**
     * Disable a MIDI input device by index
     */
    bool disableInputDevice(unsigned int deviceIndex);
    
    /**
     * Enable a MIDI output device by name
     */
    bool enableOutputDevice(const std::string& deviceName);
    
    /**
     * Enable a MIDI output device by index
     */
    bool enableOutputDevice(unsigned int deviceIndex);
    
    /**
     * Disable a MIDI output device by name
     */
    bool disableOutputDevice(const std::string& deviceName);
    
    /**
     * Disable a MIDI output device by index
     */
    bool disableOutputDevice(unsigned int deviceIndex);

    // ===== MIDI INPUT CALLBACK =====
    
    /**
     * Set the user callback function for MIDI input
     * This callback will be called for all MIDI messages that aren't handled by control surfaces
     */
    void setMidiInputCallback(MidiInputCallback callback);
    
    /**
     * Clear the user callback function
     */
    void clearMidiInputCallback();

    // ===== MIDI OUTPUT =====
    
    /**
     * Send a MIDI message to a specific output device
     * @param message The MIDI message to send
     * @param deviceName Name of the output device
     * @return true if successful
     */
    bool sendMidiMessage(const choc::midi::ShortMessage& message, const std::string& deviceName);
    
    /**
     * Send a MIDI message to a specific output device by index
     */
    bool sendMidiMessage(const choc::midi::ShortMessage& message, unsigned int deviceIndex);
    
    /**
     * Send a MIDI message to all enabled output devices
     */
    bool broadcastMidiMessage(const choc::midi::ShortMessage& message);

    // ===== CONTROL SURFACE MANAGEMENT =====
    
    /**
     * Register a control surface
     * Control surfaces are processed in registration order
     */
    void registerControlSurface(std::shared_ptr<ControlSurface> surface);
    
    /**
     * Unregister a control surface
     */
    void unregisterControlSurface(std::shared_ptr<ControlSurface> surface);
    
    /**
     * Clear all registered control surfaces
     */
    void clearControlSurfaces();
    
    /**
     * Get list of registered control surfaces
     */
    const std::vector<std::shared_ptr<ControlSurface>>& getControlSurfaces() const { return controlSurfaces; }

    // ===== UTILITY METHODS =====
    
    /**
     * Get names of all input devices
     */
    std::vector<std::string> getInputDeviceNames() const;
    
    /**
     * Get names of all output devices
     */
    std::vector<std::string> getOutputDeviceNames() const;
    
    /**
     * Get names of all enabled input devices
     */
    std::vector<std::string> getEnabledInputDeviceNames() const;
    
    /**
     * Get names of all enabled output devices
     */
    std::vector<std::string> getEnabledOutputDeviceNames() const;
    
    /**
     * Find input device by name
     */
    InputDevice* findInputDevice(const std::string& deviceName);
    
    /**
     * Find output device by name
     */
    OutputDevice* findOutputDevice(const std::string& deviceName);

private:
    // ===== INTERNAL CALLBACK HANDLING =====
    
    /**
     * Static callback function for RtMidi input
     * This is called by RtMidi and forwards to the instance method
     */
    static void rtMidiInputCallback(double timeStamp, std::vector<unsigned char>* message, void* userData);
    
    /**
     * Instance method for handling MIDI input
     */
    void handleMidiInput(double timeStamp, const std::vector<unsigned char>& rawMessage, 
                        const std::string& deviceName, unsigned int deviceIndex);
    
    /**
     * Process a MIDI message through the control surface chain
     * @return true if the message was handled by a control surface
     */
    bool processControlSurfaceChain(const choc::midi::ShortMessage& message, 
                                   const std::string& deviceName, 
                                   unsigned int deviceIndex);

    // ===== DEVICE LISTS =====
    std::vector<std::unique_ptr<InputDevice>> inputDevices;
    std::vector<std::unique_ptr<OutputDevice>> outputDevices;
    
    // ===== CALLBACK DATA =====
    std::vector<std::unique_ptr<MidiInputCallbackData>> callbackData;
    MidiInputCallback userMidiCallback;
    
    // ===== CONTROL SURFACES =====
    std::vector<std::shared_ptr<ControlSurface>> controlSurfaces;
    
    // ===== THREAD SAFETY =====
    mutable choc::threading::SpinLock deviceListLock;
    mutable choc::threading::SpinLock callbackLock;
    mutable choc::threading::SpinLock controlSurfaceLock;
    
    // ===== INITIALIZATION STATE =====
    std::atomic<bool> initialized{false};
};