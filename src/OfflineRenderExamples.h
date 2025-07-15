#pragma once

#include "core/AudioEngine.h"
#include "core/Logger.h"
#include <iostream>

class OfflineRenderExamples {
public:
    // Example 1: Render by time in seconds
    static bool renderBySeconds(AudioEngine& engine, const std::string& outputPath, double seconds) {
        AudioEngine::OfflineRenderParams params;
        params.outputFilePath = outputPath;
        params.lengthInSeconds = seconds;
        params.renderSampleRate = 44100.0;
        params.renderBufferSize = 1024;
        
        Logger::info("Rendering {} seconds to: {}", seconds, outputPath);
        return engine.renderOffline(params);
    }
    
    // Example 2: Render by sample count
    static bool renderBySamples(AudioEngine& engine, const std::string& outputPath, int samples) {
        AudioEngine::OfflineRenderParams params;
        params.outputFilePath = outputPath;
        params.lengthInSamples = samples;
        params.renderSampleRate = 44100.0;
        params.renderBufferSize = 1024;
        
        Logger::info("Rendering {} samples to: {}", samples, outputPath);
        return engine.renderOffline(params);
    }
    
    // Example 3: Render by musical time (ticks)
    static bool renderByTicks(AudioEngine& engine, const std::string& outputPath, 
                             int ticks, double bpm = 120.0, int tpqn = 480) {
        AudioEngine::OfflineRenderParams params;
        params.outputFilePath = outputPath;
        params.lengthInTicks = ticks;
        params.tempoBeatsPerMinute = bpm;
        params.ticksPerQuarterNote = tpqn;
        params.renderSampleRate = 44100.0;
        params.renderBufferSize = 1024;
        
        double seconds = (ticks / (double)tpqn) * (60.0 / bpm);
        Logger::info("Rendering {} ticks ({:.2f} seconds at {} BPM) to: {}", ticks, seconds, bpm, outputPath);
        return engine.renderOffline(params);
    }
    
    // Example 4: Render specific node only
    static bool renderSingleNode(AudioEngine& engine, const std::string& outputPath, 
                                std::shared_ptr<AudioNode> node, double seconds) {
        AudioEngine::OfflineRenderParams params;
        params.outputFilePath = outputPath;
        params.lengthInSeconds = seconds;
        params.sourceNode = node;
        params.renderSampleRate = 44100.0;
        params.renderBufferSize = 1024;
        
        Logger::info("Rendering single node for {} seconds to: {}", seconds, outputPath);
        return engine.renderOffline(params);
    }
    
    // Example 5: High quality render (higher sample rate)
    static bool renderHighQuality(AudioEngine& engine, const std::string& outputPath, double seconds) {
        AudioEngine::OfflineRenderParams params;
        params.outputFilePath = outputPath;
        params.lengthInSeconds = seconds;
        params.renderSampleRate = 96000.0;  // High quality sample rate
        params.renderBufferSize = 2048;     // Larger buffer for better processing
        
        Logger::info("Rendering high quality ({} Hz) for {} seconds to: {}", params.renderSampleRate, seconds, outputPath);
        return engine.renderOffline(params);
    }
    
    // Utility: Print render parameters info
    static void printRenderInfo(const AudioEngine::OfflineRenderParams& params) {
        Logger::info("=== Offline Render Parameters ===");
        Logger::info("Output file: {}", params.outputFilePath);
        Logger::info("Sample rate: {} Hz", params.renderSampleRate);
        Logger::info("Buffer size: {} samples", params.renderBufferSize);
        
        int totalSamples = AudioEngine::calculateSamplesFromParams(params);
        if (totalSamples > 0) {
            double duration = totalSamples / params.renderSampleRate;
            Logger::info("Duration: {:.2f} seconds ({} samples)", duration, totalSamples);
        }
        
        if (params.sourceNode) {
            Logger::info("Rendering single node only");
        } else {
            Logger::info("Rendering entire audio graph");
        }
        
        Logger::info("Include input: {}", (params.includeInput ? "Yes" : "No"));
        Logger::info("================================");
    }
};
