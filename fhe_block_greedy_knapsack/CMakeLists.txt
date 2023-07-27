cmake_minimum_required(VERSION 3.10)

# Set the project name
project(fhe-block-builder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Locate the SEAL library
find_package(SEAL CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Add an executable
add_executable(fhe_client fhe_client.cpp)
add_executable(fhe_server fhe_server.cpp)

# Link the executable to the SEAL library.
target_link_libraries(fhe_client SEAL::seal)
target_link_libraries(fhe_server SEAL::seal)

