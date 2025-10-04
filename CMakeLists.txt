cmake_minimum_required(VERSION 3.20)
project(ReflectionGenerator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Clang
find_package(Clang REQUIRED CONFIG)

# Include directories
include_directories(${CLANG_INCLUDE_DIRS})

# Add compile definitions
add_definitions(${CLANG_DEFINITIONS})

# Source files
set(SOURCES
    src/main.cpp
    src/ClassParser.cpp
    src/CodeGenerator.cpp
    src/ReflectionAST.cpp
    src/FileScanner.cpp
)

# Create executable
add_executable(reflect_gen ${SOURCES})

# Link libraries
target_link_libraries(reflect_gen
    clangTooling
    clangFrontend
    clangAST
    clangBasic
    clangLex
    clangParse
    clangSema
    clangAnalysis
    clangEdit
    clangRewrite
    clangDriver
    clangSerialization
    clangToolingCore
    clangToolingInclusions
    clangToolingRefactoring
    clangToolingASTDiff
    clangToolingSyntax
    clangIndex
    clangFormat
    clang
    clang-cpp
)

# Set target properties
set_target_properties(reflect_gen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    VERSION ${PROJECT_VERSION}
)

# Install target
install(TARGETS reflect_gen
    RUNTIME DESTINATION bin
)

# Install headers (for library usage)
install(DIRECTORY src/
    DESTINATION include/ReflectionGenerator
    FILES_MATCHING PATTERN "*.h"
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ReflectionGeneratorConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ReflectionGeneratorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ReflectionGeneratorConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ReflectionGenerator
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ReflectionGeneratorConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ReflectionGeneratorConfigVersion.cmake"
    DESTINATION lib/cmake/ReflectionGenerator
)

# Export target
install(EXPORT ReflectionGeneratorTargets
    FILE ReflectionGeneratorTargets.cmake
    NAMESPACE ReflectionGenerator::
    DESTINATION lib/cmake/ReflectionGenerator
)