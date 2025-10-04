# C++ Reflection Generator

A powerful C++ reflection code generator using Clang LibTooling for automatic generation of reflection metadata from C++ classes.

## Features

- **Accurate C++ Parsing**: Uses Clang LibTooling for precise AST parsing
- **Macro Support**: Processes `GCLASS`, `GPROPERTY`, and `GFUNCTION` macros
- **Code Generation**: Generates header and implementation files for reflection
- **Binary Serialization**: Supports fast binary serialization/deserialization
- **Type Registry**: Automatic type registration at startup
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **CMake Integration**: Easy integration with existing projects

## Quick Start

### Installation

```bash
git clone https://github.com/your-username/ReflectionGenerator.git
cd ReflectionGenerator
mkdir build && cd build
cmake ..
cmake --build .
```

### Usage

```bash
# Scan directories for reflection-enabled classes
./bin/reflect_gen --scan-dirs Engine,Game --output-dir Generated

# Process specific files
./bin/reflect_gen --input-files src/Player.h --output-dir Generated

# Enable verbose output
./bin/reflect_gen --scan-dirs Engine,Game --output-dir Generated --verbose
```

## Reflection Macros

### GCLASS

Marks a class as reflection-enabled:

```cpp
GCLASS(Blueprintable, Serializable, Version=2)
class Player : public GObject {
    GENERATED_BODY()
    // ...
};
```

**Flags:**
- `Blueprintable`: Class can be used in blueprints
- `Serializable`: Class can be serialized
- `Abstract`: Abstract class
- `DefaultToInstanced`: Default to instanced
- `Version=N`: Serialization version

### GPROPERTY

Marks a field for reflection:

```cpp
GPROPERTY(Save, Edit, Category("Identity"), Clamp(0, 100))
int health = 100;

GPROPERTY(Save, Edit, Category("Identity"))
std::string name;
```

**Flags:**
- `Save`: Include in serialization
- `Edit`: Show in editor
- `Transient`: Don't serialize
- `EditorOnly`: Only in editor
- `ReadOnly`: Read-only in editor

**Metadata:**
- `Category("Name")`: Group in editor
- `Clamp(min, max)`: Value constraints
- `Default(value)`: Default value
- `Tooltip("text")`: Help text

### GFUNCTION

Marks a function for reflection:

```cpp
GFUNCTION(Callable, Category("Gameplay"))
void Reset();

GFUNCTION(BlueprintEvent, Category("Gameplay"))
void OnPlayerDied();
```

**Flags:**
- `Callable`: Can be called from editor/scripts
- `BlueprintEvent`: Blueprint event
- `BlueprintCallable`: Can be called from blueprints

**Metadata:**
- `Category("Name")`: Group in editor
- `Tooltip("text")`: Help text

### GENERATED_BODY

Placeholder for generated code:

```cpp
class Player : public GObject {
    GENERATED_BODY()
    // Generated code will be inserted here
};
```

## Generated Code

For each reflection-enabled class, the generator creates:

### Header File (`ClassName.generated.h`)
- Forward declarations
- Registration function declarations
- Factory function declarations

### Implementation File (`ClassName.generated.cpp`)
- Type registration
- Property registration
- Function registration
- Serialization/deserialization code
- Static registration

## CMake Integration

### As a Submodule

```bash
git submodule add https://github.com/your-username/ReflectionGenerator.git Tools/ReflectionGenerator
```

```cmake
# In your CMakeLists.txt
add_subdirectory(Tools/ReflectionGenerator)

# Custom target for reflection generation
add_custom_target(GenerateReflection
    COMMAND reflect_gen --scan-dirs Engine,Game --output-dir ${CMAKE_BINARY_DIR}/Generated
    DEPENDS reflect_gen
    COMMENT "Generating reflection code"
)

add_dependencies(YourTarget GenerateReflection)
```

### As a Package

```cmake
find_package(ReflectionGenerator REQUIRED)

add_custom_target(GenerateReflection
    COMMAND ReflectionGenerator::reflect_gen --scan-dirs Engine,Game --output-dir ${CMAKE_BINARY_DIR}/Generated
    COMMENT "Generating reflection code"
)
```

## Build Requirements

- **Clang**: Version 15.0 or later
- **CMake**: Version 3.20 or later
- **C++ Standard**: C++23

## Dependencies

- Clang LibTooling
- Clang Frontend
- Clang AST
- Clang Basic
- Standard C++ Library

## Examples

See the `examples/` directory for complete examples of:
- Basic class reflection
- Property serialization
- Function reflection
- CMake integration

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [Clang LibTooling](https://clang.llvm.org/docs/LibTooling.html)
- Inspired by Unreal Engine's reflection system
- Designed for modern C++ projects