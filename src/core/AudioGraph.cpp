#include "AudioGraph.h"
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <cstring>
#include <iostream>
#include "Spinlock.h"


AudioGraph::AudioGraph() {
}

void AudioGraph::addNode(std::shared_ptr<AudioNode> node) {
    if (!node) return;
    
    SpinLockGuard lock(compilationLock);
    
    // Check if node already exists
    auto it = std::find(nodes.begin(), nodes.end(), node);
    if (it == nodes.end()) {
        nodes.push_back(node);
        connections[node] = std::vector<std::shared_ptr<AudioNode>>();
        markDirty();
    }
}

void AudioGraph::removeNode(std::shared_ptr<AudioNode> node) {
    if (!node) return;
    
    SpinLockGuard lock(compilationLock);
    
    // Remove from nodes list
    nodes.erase(std::remove(nodes.begin(), nodes.end(), node), nodes.end());
    
    // Remove from output nodes if it's there
    outputNodes.erase(std::remove(outputNodes.begin(), outputNodes.end(), node), outputNodes.end());
    
    // Remove all connections to and from this node
    connections.erase(node);
    for (auto& [sourceNode, targets] : connections) {
        targets.erase(std::remove(targets.begin(), targets.end(), node), targets.end());
    }
    
    markDirty();
}

void AudioGraph::clear() {
    SpinLockGuard lock(compilationLock);
    
    nodes.clear();
    outputNodes.clear();
    connections.clear();
    prepared = false;
    markDirty();
}

void AudioGraph::connectNodes(std::shared_ptr<AudioNode> source, std::shared_ptr<AudioNode> destination) {
    if (!source || !destination) return;
    
    SpinLockGuard lock(compilationLock);
    
    // Ensure both nodes are in the graph (without calling addNode to avoid recursive locking)
    auto sourceIt = std::find(nodes.begin(), nodes.end(), source);
    if (sourceIt == nodes.end()) {
        nodes.push_back(source);
        connections[source] = std::vector<std::shared_ptr<AudioNode>>();
    }
    
    auto destIt = std::find(nodes.begin(), nodes.end(), destination);
    if (destIt == nodes.end()) {
        nodes.push_back(destination);
        connections[destination] = std::vector<std::shared_ptr<AudioNode>>();
    }
    
    // Add connection if it doesn't exist
    auto& targets = connections[source];
    if (std::find(targets.begin(), targets.end(), destination) == targets.end()) {
        targets.push_back(destination);
        markDirty();
    }
}

void AudioGraph::disconnectNodes(std::shared_ptr<AudioNode> source, std::shared_ptr<AudioNode> destination) {
    if (!source || !destination) return;
    
    SpinLockGuard lock(compilationLock);
    
    auto it = connections.find(source);
    if (it != connections.end()) {
        auto& targets = it->second;
        targets.erase(std::remove(targets.begin(), targets.end(), destination), targets.end());
        markDirty();
    }
}

void AudioGraph::setOutputNode(std::shared_ptr<AudioNode> node) {
    SpinLockGuard lock(compilationLock);
    
    outputNodes.clear();
    if (node) {
        outputNodes.push_back(node);
        // Ensure it's in the graph (without calling addNode to avoid recursive locking)
        auto it = std::find(nodes.begin(), nodes.end(), node);
        if (it == nodes.end()) {
            nodes.push_back(node);
            connections[node] = std::vector<std::shared_ptr<AudioNode>>();
        }
    }
    markDirty();
}

void AudioGraph::addOutputNode(std::shared_ptr<AudioNode> node) {
    if (!node) return;

    SpinLockGuard lock(compilationLock);

    // Check if already an output node
    auto it = std::find(outputNodes.begin(), outputNodes.end(), node);
    if (it == outputNodes.end()) {
        outputNodes.push_back(node);
        // Ensure it's in the graph (without calling addNode to avoid recursive locking)
        auto nodeIt = std::find(nodes.begin(), nodes.end(), node);
        if (nodeIt == nodes.end()) {
            nodes.push_back(node);
            connections[node] = std::vector<std::shared_ptr<AudioNode>>();
        }
        markDirty();
    }
}

void AudioGraph::removeOutputNode(std::shared_ptr<AudioNode> node) {
    if (!node) return;
    
    SpinLockGuard lock(compilationLock);
    
    outputNodes.erase(std::remove(outputNodes.begin(), outputNodes.end(), node), outputNodes.end());
    markDirty();
}

void AudioGraph::prepare(const AudioNode::PrepareInfo& info) {
    std::cout << "Preparing AudioGraph with " << nodes.size() << " nodes..." << std::endl;
    SpinLockGuard lock(compilationLock);
    
    currentPrepareInfo = info;
    
    // Prepare all nodes
    for (auto& node : nodes) {
        if (node) {
            node->prepare(info);
        }
    }
    
    prepared = true;
    markDirty();
}

void AudioGraph::performGraphModification(std::function<void()> modification) {
    SpinLockGuard lock(compilationLock);
    modification();
    markDirty();
}

std::shared_ptr<AudioGraph::CompiledGraph> AudioGraph::getCompiledGraph() {
    if (needsRecompile()) {
        std::cout << "Graph recompiling..." << std::endl;
        currentCompiledGraph = compileGraph();
        isDirty.store(false);
    }
    return currentCompiledGraph;
}

std::shared_ptr<AudioGraph::CompiledGraph> AudioGraph::compileGraph() {
    SpinLockGuard lock(compilationLock);
    
    auto compiled = std::make_shared<CompiledGraph>();
    
    if (!prepared || nodes.empty()) {
        return compiled;
    }
    
    // Check for cycles
    if (hasCycle()) {
        // Handle cycle error - for now, return empty graph
        return compiled;
    }
    
    // Topological sort to determine processing order
    auto sortedNodes = topologicalSort();
    
    // Create processing instructions
    compiled->instructions.reserve(sortedNodes.size());
    assignBufferIndices(sortedNodes, compiled->instructions);
    
    // Set the number of temp buffers needed
    compiled->numTempBuffers = static_cast<int>(sortedNodes.size());
    
    // Set output nodes
    compiled->outputNodes = outputNodes;
    compiled->prepared = prepared;
    compiled->prepareInfo = currentPrepareInfo;
    return compiled;
}

std::vector<std::shared_ptr<AudioNode>> AudioGraph::topologicalSort() {
    std::vector<std::shared_ptr<AudioNode>> result;
    std::unordered_map<std::shared_ptr<AudioNode>, int> inDegree;
    
    // Initialize in-degree count
    for (auto& node : nodes) {
        inDegree[node] = 0;
    }
    
    // Calculate in-degrees
    for (auto& [source, targets] : connections) {
        for (auto& target : targets) {
            inDegree[target]++;
        }
    }
    
    // Queue for nodes with no incoming edges
    std::queue<std::shared_ptr<AudioNode>> queue;
    for (auto& [node, degree] : inDegree) {
        if (degree == 0) {
            queue.push(node);
        }
    }
    
    // Process nodes
    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        result.push_back(current);
        
        // Reduce in-degree for connected nodes
        auto it = connections.find(current);
        if (it != connections.end()) {
            for (auto& target : it->second) {
                inDegree[target]--;
                if (inDegree[target] == 0) {
                    queue.push(target);
                }
            }
        }
    }
    
    return result;
}

void AudioGraph::assignBufferIndices(const std::vector<std::shared_ptr<AudioNode>>& sortedNodes, 
                                    std::vector<ProcessingInstruction>& instructions) {
    std::unordered_map<std::shared_ptr<AudioNode>, int> nodeToBufferIndex;
    int nextBufferIndex = 0;
    
    for (auto& node : sortedNodes) {
        ProcessingInstruction instruction;
        instruction.node = node;
        
        // Find input buffer indices
        for (auto& [source, targets] : connections) {
            if (std::find(targets.begin(), targets.end(), node) != targets.end()) {
                auto bufferIt = nodeToBufferIndex.find(source);
                if (bufferIt != nodeToBufferIndex.end()) {
                    instruction.inputBufferIndices.push_back(bufferIt->second);
                }
            }
        }
        
        // Assign output buffer index
        instruction.outputBufferIndex = nextBufferIndex++;
        nodeToBufferIndex[node] = instruction.outputBufferIndex;
        
        instructions.push_back(instruction);
    }
}

bool AudioGraph::hasCycle() const {
    std::unordered_set<std::shared_ptr<AudioNode>> visited;
    std::unordered_set<std::shared_ptr<AudioNode>> recursionStack;
    bool cycleFound = false;
    
    for (auto& node : nodes) {
        if (visited.find(node) == visited.end()) {
            dfsVisit(node, visited, recursionStack, cycleFound);
            if (cycleFound) return true;
        }
    }
    
    return false;
}

void AudioGraph::dfsVisit(std::shared_ptr<AudioNode> node, 
                         std::unordered_set<std::shared_ptr<AudioNode>>& visited,
                         std::unordered_set<std::shared_ptr<AudioNode>>& recursionStack,
                         bool& cycleFound) const {
    visited.insert(node);
    recursionStack.insert(node);
    
    auto it = connections.find(node);
    if (it != connections.end()) {
        for (auto& neighbor : it->second) {
            if (recursionStack.find(neighbor) != recursionStack.end()) {
                cycleFound = true;
                return;
            }
            if (visited.find(neighbor) == visited.end()) {
                dfsVisit(neighbor, visited, recursionStack, cycleFound);
                if (cycleFound) return;
            }
        }
    }
    
    recursionStack.erase(node);
}

// AudioGraphProcessor implementation
AudioGraphProcessor::AudioGraphProcessor() {
}

void AudioGraphProcessor::setCompiledGraph(std::shared_ptr<AudioGraph::CompiledGraph> graph) {
    std::cout << "Setting compiled graph with " << graph->instructions.size() << " instructions..." << std::endl;
    SpinLockGuard lock(graphLock);
    compiledGraph = graph;
}

void AudioGraphProcessor::processGraph(
    float* const* outputBuffers,
    int numOutputChannels,
    int numSamples,
    double sampleRate,
    int blockSize
) {
    std::shared_ptr<AudioGraph::CompiledGraph> graph;
    {
        SpinLockGuard lock(graphLock);
        graph = compiledGraph;
    }
    
    if (!graph || !graph->prepared || graph->instructions.empty()) {
        // Clear output buffers if no graph
        for (int ch = 0; ch < numOutputChannels; ++ch) {
            std::memset(outputBuffers[ch], 0, numSamples * sizeof(float));
        }
        return;
    }
    
    // Ensure temp buffers are sized correctly
    ensureTempBuffersSize(graph->numTempBuffers, numOutputChannels, numSamples);
    
    // Clear all temp buffers
    for (int bufferIdx = 0; bufferIdx < graph->numTempBuffers; ++bufferIdx) {
        if (bufferIdx < static_cast<int>(tempBuffers.size())) {
            std::memset(tempBuffers[bufferIdx].data(), 0, numSamples * sizeof(float));
        }
    }
    
    // Process each instruction in order
    for (const auto& instruction : graph->instructions) {
        if (!instruction.node) continue;
        
        // Prepare input buffers for this node
        std::vector<float*> inputPtrs;
        for (int inputIndex : instruction.inputBufferIndices) {
            if (inputIndex < static_cast<int>(tempBufferPtrs.size())) {
                inputPtrs.push_back(tempBufferPtrs[inputIndex]);
            }
        }
        
        // Get output buffer for this node
        float* outputPtr = nullptr;
        if (instruction.outputBufferIndex < static_cast<int>(tempBufferPtrs.size())) {
            outputPtr = tempBufferPtrs[instruction.outputBufferIndex];
        }
        
        if (outputPtr) {
            // Process the node
            std::vector<float*> outputPtrs = {outputPtr};
            instruction.node->processCallback(
                inputPtrs.empty() ? nullptr : inputPtrs.data(),
                outputPtrs.data(),
                static_cast<int>(inputPtrs.size()),
                1, // Single channel temp buffer
                numSamples,
                sampleRate,
                blockSize
            );
        }
    }
    
    // Mix output nodes to final output buffers
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        std::memset(outputBuffers[ch], 0, numSamples * sizeof(float));
    }
    
    for (auto& outputNode : graph->outputNodes) {
        // Find the buffer index for this output node
        for (const auto& instruction : graph->instructions) {
            if (instruction.node == outputNode) {
                if (instruction.outputBufferIndex < static_cast<int>(tempBufferPtrs.size())) {
                    float* nodeOutput = tempBufferPtrs[instruction.outputBufferIndex];
                    
                    // Mix to all output channels (mono to stereo)
                    for (int sample = 0; sample < numSamples; ++sample) {
                        for (int ch = 0; ch < numOutputChannels; ++ch) {
                            outputBuffers[ch][sample] += nodeOutput[sample];
                        }
                    }
                }
                break;
            }
        }
    }
}

void AudioGraphProcessor::ensureTempBuffersSize(int numBuffers, int numChannels, int numSamples) {
    if (tempBuffers.size() < static_cast<size_t>(numBuffers)) {
        tempBuffers.resize(numBuffers);
        tempBufferPtrs.resize(numBuffers);
    }
    
    for (int bufferIdx = 0; bufferIdx < numBuffers; ++bufferIdx) {
        if (tempBuffers[bufferIdx].size() < static_cast<size_t>(numSamples)) {
            tempBuffers[bufferIdx].resize(numSamples);
        }
        tempBufferPtrs[bufferIdx] = tempBuffers[bufferIdx].data();
    }
}
