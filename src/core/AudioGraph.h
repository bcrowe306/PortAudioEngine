#pragma once

#include "AudioNode.h"
#include "../../lib/choc/threading/choc_SpinLock.h"
#include "../../lib/choc/audio/choc_SampleBuffers.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include <functional>

class AudioGraph {
public:
    // Compiled processing instruction for real-time thread
    struct ProcessingInstruction {
        std::shared_ptr<AudioNode> node;
        std::vector<int> inputBufferIndices;  // Which temp buffers to read from
        int outputBufferIndex;                // Which temp buffer to write to
    };

    // Compiled graph for real-time processing
    struct CompiledGraph {
        std::vector<ProcessingInstruction> instructions;
        std::vector<std::shared_ptr<AudioNode>> outputNodes;
        int numTempBuffers;
        bool prepared = false;
        AudioNode::PrepareInfo prepareInfo;
    };

    AudioGraph();
    ~AudioGraph() = default;

    // Graph management (called from non-real-time thread)
    void addNode(std::shared_ptr<AudioNode> node);
    void removeNode(std::shared_ptr<AudioNode> node);
    void clear();
    void connectNodes(std::shared_ptr<AudioNode> source, std::shared_ptr<AudioNode> destination);
    void disconnectNodes(std::shared_ptr<AudioNode> source, std::shared_ptr<AudioNode> destination);

    // Set the output node(s) - these are the final nodes in the chain
    void setOutputNode(std::shared_ptr<AudioNode> node);
    void addOutputNode(std::shared_ptr<AudioNode> node);
    void removeOutputNode(std::shared_ptr<AudioNode> node);

    // Prepare and compile the graph (called from non-real-time thread)
    void prepare(const AudioNode::PrepareInfo& info);
    void markDirty() { isDirty.store(true); }
    bool needsRecompile() const { return isDirty.load(); }

    // Get compiled graph for real-time processing
    std::shared_ptr<CompiledGraph> getCompiledGraph();
    
    // Get current compiled graph without recompilation (lock-free)
    std::shared_ptr<CompiledGraph> getCurrentCompiledGraph() const { return currentCompiledGraph; }

    // Utility functions
    size_t getNodeCount() const { return nodes.size(); }
    const std::vector<std::shared_ptr<AudioNode>>& getNodes() const { return nodes; }
    
    // Safe way to modify graph from application thread
    void performGraphModification(std::function<void()> modification);

private:
    // Graph structure (modified from non-real-time thread)
    std::vector<std::shared_ptr<AudioNode>> nodes;
    std::vector<std::shared_ptr<AudioNode>> outputNodes;
    std::unordered_map<std::shared_ptr<AudioNode>, std::vector<std::shared_ptr<AudioNode>>> connections;
    
    // Compilation state
    std::atomic<bool> isDirty{true};
    std::shared_ptr<CompiledGraph> currentCompiledGraph;
    mutable choc::threading::SpinLock compilationLock;
    
    AudioNode::PrepareInfo currentPrepareInfo;
    bool prepared = false;

    // Graph compilation methods
    std::shared_ptr<CompiledGraph> compileGraph();
    std::vector<std::shared_ptr<AudioNode>> topologicalSort();
    void assignBufferIndices(const std::vector<std::shared_ptr<AudioNode>>& sortedNodes, 
                           std::vector<ProcessingInstruction>& instructions);
    bool hasCycle() const;
    void dfsVisit(std::shared_ptr<AudioNode> node, 
                  std::unordered_set<std::shared_ptr<AudioNode>>& visited,
                  std::unordered_set<std::shared_ptr<AudioNode>>& recursionStack,
                  bool& cycleFound) const;
};

// Real-time safe graph processor
class AudioGraphProcessor {
public:
    AudioGraphProcessor();
    ~AudioGraphProcessor() = default;

    // Set the compiled graph (called from non-real-time thread)
    void setCompiledGraph(std::shared_ptr<AudioGraph::CompiledGraph> graph);

    // Process the graph (called from real-time thread)
    void processGraph(
        choc::buffer::ChannelArrayView<const float> inputBuffers,
        choc::buffer::ChannelArrayView<float> outputBuffers,
        double sampleRate,
        int blockSize
    );

private:
    std::shared_ptr<AudioGraph::CompiledGraph> compiledGraph;
    mutable choc::threading::SpinLock graphLock;
    
    // Temporary buffers for intermediate processing
    std::vector<std::vector<float>> tempBuffers;
    std::vector<float*> tempBufferPtrs;
    
    void ensureTempBuffersSize(int numBuffers, int numChannels, int numSamples);
};
