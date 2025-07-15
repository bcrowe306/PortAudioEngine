#pragma once

#include "core/AudioEngine.h"
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
        
        std::cout << "Rendering " << seconds << " seconds to: " << outputPath << std::endl;
        return engine.renderOffline(params);
    }
    
    // Example 2: Render by sample count
    static bool renderBySamples(AudioEngine& engine, const std::string& outputPath, int samples) {
        AudioEngine::OfflineRenderParams params;
        params.outputFilePath = outputPath;
        params.lengthInSamples = samples;
        params.renderSampleRate = 44100.0;
        params.renderBufferSize = 1024;
        
        std::cout << "Rendering " << samples << " samples to: " << outputPath << std::endl;
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
        std::cout << "Rendering " << ticks << " ticks (" << seconds << " seconds at " 
                  << bpm << " BPM) to: " << outputPath << std::endl;
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
        
        std::cout << "Rendering single node for " << seconds << " seconds to: " << outputPath << std::endl;
        return engine.renderOffline(params);
    }
    
    // Example 5: High quality render (higher sample rate)
    static bool renderHighQuality(AudioEngine& engine, const std::string& outputPath, double seconds) {
        AudioEngine::OfflineRenderParams params;
        params.outputFilePath = outputPath;
        params.lengthInSeconds = seconds;
        params.renderSampleRate = 96000.0;  // High quality sample rate
        params.renderBufferSize = 2048;     // Larger buffer for better processing
        
        std::cout << "Rendering high quality (" << params.renderSampleRate << " Hz) for " 
                  << seconds << " seconds to: " << outputPath << std::endl;
        return engine.renderOffline(params);
    }
    
    // Utility: Print render parameters info
    static void printRenderInfo(const AudioEngine::OfflineRenderParams& params) {
        std::cout << "\n=== Offline Render Parameters ===" << std::endl;
        std::cout << "Output file: " << params.outputFilePath << std::endl;
        std::cout << "Sample rate: " << params.renderSampleRate << " Hz" << std::endl;
        std::cout << "Buffer size: " << params.renderBufferSize << " samples" << std::endl;
        
        int totalSamples = AudioEngine::calculateSamplesFromParams(params);
        if (totalSamples > 0) {
            double duration = totalSamples / params.renderSampleRate;
            std::cout << "Duration: " << duration << " seconds (" << totalSamples << " samples)" << std::endl;
        }
        
        if (params.sourceNode) {
            std::cout << "Rendering single node only" << std::endl;
        } else {
            std::cout << "Rendering entire audio graph" << std::endl;
        }
        
        std::cout << "Include input: " << (params.includeInput ? "Yes" : "No") << std::endl;
        std::cout << "================================\n" << std::endl;
    }
};
