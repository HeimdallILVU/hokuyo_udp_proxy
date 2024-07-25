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
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Find required packages
find_package(Threads REQUIRED)

# Link against required libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
)

# Add compile options
target_compile_options(${PROJECT_NAME}
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

# Install rules
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Generate and install pkg-config file
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Packaging
include(CPack)