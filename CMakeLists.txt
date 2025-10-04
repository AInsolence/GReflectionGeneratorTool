cmake_minimum_required(VERSION 3.20)
project(ReflectionGenerator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force Release build for generator to match LLVM libraries
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Clang
# Try to find Clang in the custom installation path first
set(CLANG_DIR "C:/LLVM/install/lib/cmake/clang")
if(EXISTS "${CLANG_DIR}/ClangConfig.cmake")
    set(Clang_DIR "${CLANG_DIR}")
    find_package(Clang REQUIRED CONFIG)
else()
    # Fallback to standard find_package
    find_package(Clang REQUIRED CONFIG)
endif()

# If not found, try to find manually with explicit paths
if(NOT Clang_FOUND)
    # Set explicit paths to your LLVM installation
    set(CLANG_INCLUDE_DIR "C:/LLVM/install/include")
    set(CLANG_LIBRARY "C:/LLVM/install/lib/libclang.lib")
    set(CLANG_TOOLS_DIR "C:/LLVM/install/bin")
    set(LLVM_INCLUDE_DIR "C:/LLVM/install/include")
    set(LLVM_LIB_DIR "C:/LLVM/install/lib")
    
    # Check if files exist
    if(EXISTS "${CLANG_INCLUDE_DIR}/clang-c/Index.h" AND EXISTS "${CLANG_LIBRARY}")
        message(STATUS "Found Clang manually: ${CLANG_INCLUDE_DIR}, ${CLANG_LIBRARY}")
        set(Clang_FOUND TRUE)
        set(CLANG_INCLUDE_DIRS ${CLANG_INCLUDE_DIR})
        set(CLANG_LIBRARIES ${CLANG_LIBRARY})
        
        # Add LibTooling include directories
        include_directories(${LLVM_INCLUDE_DIR})
        include_directories(${LLVM_INCLUDE_DIR}/clang)
        include_directories(${LLVM_INCLUDE_DIR}/llvm)
        
        message(STATUS "LibTooling paths set: ${LLVM_INCLUDE_DIR}, ${LLVM_LIB_DIR}")
    else()
        message(FATAL_ERROR "Clang not found at C:/LLVM/install/")
    endif()
endif()

# Include directories
include_directories(${CLANG_INCLUDE_DIRS})

# Add compile definitions
add_definitions(${CLANG_DEFINITIONS})

# Source files
set(SOURCES
    src/main.cpp
    src/ClassParser.cpp
    src/CodeGenerator.cpp
    src/FileScanner.cpp
)

# Create executable
add_executable(reflect_gen ${SOURCES})

# Suppress Clang warnings
if(MSVC)
    target_compile_options(reflect_gen PRIVATE
        /wd4805  # unsafe mix of type 'unsigned int' and type 'bool'
        /wd4291  # no matching operator delete found
        /wd4834  # discarding return value
    )
endif()

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
)

# Add library search path
if(LLVM_LIB_DIR)
    target_link_directories(reflect_gen PRIVATE ${LLVM_LIB_DIR})
    message(STATUS "Added library search path: ${LLVM_LIB_DIR}")
endif()

# Set target properties
set_target_properties(reflect_gen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    VERSION ${PROJECT_VERSION}
)