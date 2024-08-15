cmake_minimum_required(VERSION 3.10)
project(hokuyo_udp_proxy VERSION 1.0.0 LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define source files
set(SOURCES
    src/hokuyo_udp_proxy.cpp
    src/hokuyo_udp_proxy_n.cpp
)

# Create executable
add_executable(${PROJECT_NAME} src/hokuyo_udp_proxy.cpp)
add_executable(hokuyo_udp_proxy_n src/hokuyo_udp_proxy_n.cpp)

# Find required packages
find_package(Threads REQUIRED)

# Link against required libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
)

# find YAML
find_package(yaml-cpp REQUIRED)

# Link against required libraries
target_link_libraries(hokuyo_udp_proxy_n
    PRIVATE
        Threads::Threads
        yaml-cpp
)

# Add compile options
target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

# Add compile options
target_compile_options(hokuyo_udp_proxy_n
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Include directories
target_include_directories(hokuyo_udp_proxy_n
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Packaging
include(CPack)