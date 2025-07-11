/*
 * Thread-Safe Audio Graph Usage Example
 * 
 * This example demonstrates how to use the new thread-safe audio graph system:
 * 1. Graph modifications happen on a background thread
 * 2. A dirty flag triggers recompilation
 * 3. The compiled graph is atomically swapped into the real-time processor
 * 4. The real-time thread processes pre-compiled instructions with no allocations
 */

#include "AudioGraph.h"
#include "OscillatorNode.h"
#include "GainNode.h"
#include <thread>
#include <chrono>

class ThreadSafeAudioEngine {
public:
    void initialize() {
        // Create the graph builder (runs on background thread)
        audioGraph = std::make_unique<AudioGraph>();
        
        // Create the real-time processor (used by audio callback)
        processor = std::make_unique<AudioGraphProcessor>();
        
        // Start the background compilation thread
        startBackgroundThread();
    }
    
    void shutdown() {
        stopBackgroundThread();
    }
    
    // Called from UI/control thread - safe to allocate and modify graph
    void buildExampleGraph() {
        // Create nodes
        auto oscillator = std::make_shared<OscillatorNode>(440.0f, OscillatorNode::WaveType::Sine);
        auto gainNode = std::make_shared<GainNode>(0.3f);
        
        // Add nodes to graph
        audioGraph->addNode(oscillator);
        audioGraph->addNode(gainNode);
        
        // Connect: oscillator -> gain -> output
        audioGraph->connectNodes(oscillator, gainNode);
        audioGraph->setOutputNode(gainNode);
        
        // Prepare the graph
        AudioNode::PrepareInfo info;
        info.sampleRate = 44100.0;
        info.maxBufferSize = 512;
        info.numChannels = 2;
        audioGraph->prepare(info);
        
        // Mark dirty to trigger recompilation
        audioGraph->markDirty();
    }
    
    // Called from real-time audio callback - lock-free processing
    void processAudio(float* const* outputBuffers, int numChannels, int numSamples, double sampleRate) {
        processor->processGraph(outputBuffers, numChannels, numSamples, sampleRate, numSamples);
    }
    
    // Modify graph safely (from UI thread)
    void changeOscillatorFrequency(float newFreq) {
        // Find oscillator node and change frequency
        for (auto& node : audioGraph->getNodes()) {
            if (auto osc = std::dynamic_pointer_cast<OscillatorNode>(node)) {
                osc->setFrequency(newFreq);
                break;
            }
        }
        // No need to mark dirty for parameter changes, only structural changes
    }
    
    // Add effect to chain (from UI thread)
    void addGainStage() {
        auto newGain = std::make_shared<GainNode>(0.8f, "ExtraGain");
        
        // Insert between existing nodes
        auto& nodes = audioGraph->getNodes();
        if (nodes.size() >= 2) {
            auto source = nodes[0];  // oscillator
            auto destination = nodes[1];  // original gain
            
            // Reconnect: source -> newGain -> destination
            audioGraph->disconnectNodes(source, destination);
            audioGraph->connectNodes(source, newGain);
            audioGraph->connectNodes(newGain, destination);
            audioGraph->addNode(newGain);
            
            // Mark dirty to trigger recompilation
            audioGraph->markDirty();
        }
    }

private:
    std::unique_ptr<AudioGraph> audioGraph;
    std::unique_ptr<AudioGraphProcessor> processor;
    
    std::thread backgroundThread;
    std::atomic<bool> shouldStop{false};
    
    void startBackgroundThread() {
        backgroundThread = std::thread([this]() {
            while (!shouldStop.load()) {
                // Check if graph needs recompilation
                if (audioGraph && audioGraph->needsRecompile()) {
                    // Compile the graph on background thread
                    auto compiledGraph = audioGraph->getCompiledGraph();
                    
                    // Atomically swap the compiled graph into the processor
                    processor->setCompiledGraph(compiledGraph);
                }
                
                // Sleep briefly to avoid busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    void stopBackgroundThread() {
        shouldStop.store(true);
        if (backgroundThread.joinable()) {
            backgroundThread.join();
        }
    }
};

/*
 * Key Benefits of This Approach:
 * 
 * 1. REAL-TIME SAFETY: 
 *    - Audio callback only processes pre-compiled instructions
 *    - No dynamic allocation, no graph traversal
 *    - Predictable, bounded execution time
 * 
 * 2. THREAD SAFETY:
 *    - Graph modifications happen on background thread
 *    - Compiled graph is atomically swapped
 *    - No locks in the audio callback
 * 
 * 3. FLEXIBILITY:
 *    - Can modify graph structure from UI thread
 *    - Changes are compiled in background
 *    - Seamless transition to new graph structure
 * 
 * 4. PERFORMANCE:
 *    - Topological sort done once during compilation
 *    - Optimal buffer allocation pre-computed
 *    - Cache-friendly sequential processing
 */
