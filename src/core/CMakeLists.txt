# Create core audio library



add_library(audio_core STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GainNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OscillatorNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioRecorder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioParameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LevelsNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnalyzerNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PlayheadNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ADSRNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FilterNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MidiEngine.cpp
)

#  PortAudio
find_package(portaudio CONFIG REQUIRED)
target_link_libraries(audio_core PRIVATE portaudio_static)

#  RTMidi
find_package(rtmidi CONFIG REQUIRED)
target_link_libraries(audio_core PRIVATE RtMidi::rtmidi)

target_include_directories(audio_core PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/concurrentqueue
    ${CMAKE_SOURCE_DIR}/lib/choc
    ${CMAKE_CURRENT_SOURCE_DIR}
)
