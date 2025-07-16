cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(port_audio_engine VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)


add_subdirectory(src/core)
target_link_libraries(${PROJECT_NAME} PRIVATE audio_core)


#  PortAudio
find_package(portaudio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE portaudio_static)

#  RTMidi
find_package(rtmidi CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE RtMidi::rtmidi)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# concurrentqueue
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/concurrentqueue
)
# choc
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/choc
)

# spdlog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)

if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE
  "-framework CoreMidi"
  "-framework Cocoa"
  "-framework Accelerate"
  "-framework AudioToolbox"
  "-framework AudioUnit"
  "-framework CoreServices"
  "-framework CoreAudio"
  "-framework Carbon"
  "-framework CoreFoundation"
    "-framework IOKit"
  )
endif()

# Add test executable for SamplePlayerNode
add_executable(test_sample_player
    ${CMAKE_SOURCE_DIR}/test_sample_player.cpp
)

target_link_libraries(test_sample_player PRIVATE audio_core)
target_link_libraries(test_sample_player PRIVATE fmt::fmt)
target_link_libraries(test_sample_player PRIVATE spdlog::spdlog_header_only)

target_include_directories(test_sample_player PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/concurrentqueue
    ${CMAKE_SOURCE_DIR}/lib/choc
    ${CMAKE_SOURCE_DIR}
)

# Add test executable for PolyphonicSampler
add_executable(test_polyphonic_sampler
    ${CMAKE_SOURCE_DIR}/test_polyphonic_sampler.cpp
)

target_link_libraries(test_polyphonic_sampler PRIVATE audio_core)
target_link_libraries(test_polyphonic_sampler PRIVATE fmt::fmt)
target_link_libraries(test_polyphonic_sampler PRIVATE spdlog::spdlog_header_only)

target_include_directories(test_polyphonic_sampler PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/concurrentqueue
    ${CMAKE_SOURCE_DIR}/lib/choc
    ${CMAKE_SOURCE_DIR}
)
