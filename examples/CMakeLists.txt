cmake_minimum_required(VERSION 3.20)
project(ReflectionGeneratorExamples)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ReflectionGenerator
find_package(ReflectionGenerator REQUIRED)

# Example executable
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage ReflectionGenerator::reflect_gen)
