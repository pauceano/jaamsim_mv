cmake_minimum_required(VERSION 3.20)
project(jaamsim-cpp VERSION 2026.04 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only libraries
add_library(jaamsim_math INTERFACE)
target_include_directories(jaamsim_math INTERFACE math)
target_compile_features(jaamsim_math INTERFACE cxx_std_17)

add_library(jaamsim_json INTERFACE)
target_include_directories(jaamsim_json INTERFACE json)
target_link_libraries(jaamsim_json INTERFACE jaamsim_math)

add_library(jaamsim_units INTERFACE)
target_include_directories(jaamsim_units INTERFACE units)

add_library(jaamsim_rng STATIC
    rng/mrg1999a.cpp
)
target_include_directories(jaamsim_rng PUBLIC rng)
target_compile_features(jaamsim_rng PUBLIC cxx_std_17)

add_library(jaamsim_states INTERFACE)
target_include_directories(jaamsim_states INTERFACE states)
target_link_libraries(jaamsim_states INTERFACE jaamsim_math)

add_library(jaamsim_basicsim INTERFACE)
target_include_directories(jaamsim_basicsim INTERFACE basicsim)
target_link_libraries(jaamsim_basicsim INTERFACE jaamsim_math jaamsim_units jaamsim_states)

add_library(jaamsim_events INTERFACE)
target_include_directories(jaamsim_events INTERFACE events)
target_link_libraries(jaamsim_events INTERFACE jaamsim_basicsim)

# ZMQ interface
add_library(jaamsim_remote INTERFACE)
target_include_directories(jaamsim_remote INTERFACE remote)
target_link_libraries(jaamsim_remote INTERFACE jaamsim_basicsim jaamsim_json)

# Examples / main
add_executable(headless_model headless_model.cpp)
target_link_libraries(headless_model PRIVATE
    jaamsim_basicsim
    jaamsim_events
    jaamsim_json
    jaamsim_rng
    jaamsim_remote
    cppzmq
    Threads::Threads
)

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
