#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace ReflectionGenerator {

/**
 * Represents a property in a reflection-enabled class
 */
struct PropertyInfo {
    std::string name;
    std::string type;
    std::string qualifiedType;
    size_t offset = 0;
    
    // Flags from GPROPERTY macro
    bool save = false;
    bool edit = false;
    bool transient = false;
    bool editorOnly = false;
    bool readOnly = false;
    
    // Metadata
    std::string category;
    std::string tooltip;
    std::string defaultValue;
    std::string clampMin;
    std::string clampMax;
    
    // Source location
    std::string fileName;
    int lineNumber = 0;
};

/**
 * Represents a function in a reflection-enabled class
 */
struct FunctionInfo {
    std::string name;
    std::string returnType;
    std::vector<std::string> parameters;
    std::vector<std::string> parameterTypes;
    
    // Flags from GFUNCTION macro
    bool callable = false;
    bool blueprintEvent = false;
    bool blueprintCallable = false;
    
    // Metadata
    std::string category;
    std::string tooltip;
    
    // Source location
    std::string fileName;
    int lineNumber = 0;
};

/**
 * Represents a reflection-enabled class
 */
struct ClassInfo {
    std::string name;
    std::string qualifiedName;
    std::string baseClass;
    std::string namespaceName;
    
    // Flags from GCLASS macro
    bool blueprintable = false;
    bool serializable = false;
    bool abstract = false;
    bool defaultToInstanced = false;
    
    // Version for serialization
    uint32_t version = 1;
    
    // Properties and functions
    std::vector<PropertyInfo> properties;
    std::vector<FunctionInfo> functions;
    
    // Source location
    std::string fileName;
    int lineNumber = 0;
    
    // Helper methods
    bool HasProperty(const std::string& name) const {
        for (const auto& prop : properties) {
            if (prop.name == name) return true;
        }
        return false;
    }
    
    bool HasFunction(const std::string& name) const {
        for (const auto& func : functions) {
            if (func.name == name) return true;
        }
        return false;
    }
    
    const PropertyInfo* GetProperty(const std::string& name) const {
        for (const auto& prop : properties) {
            if (prop.name == name) return &prop;
        }
        return nullptr;
    }
    
    const FunctionInfo* GetFunction(const std::string& name) const {
        for (const auto& func : functions) {
            if (func.name == name) return &func;
        }
        return nullptr;
    }
};

/**
 * Represents the complete reflection information for a file
 */
struct ReflectionData {
    std::string fileName;
    std::vector<ClassInfo> classes;
    
    // Helper methods
    const ClassInfo* GetClass(const std::string& name) const {
        for (const auto& cls : classes) {
            if (cls.name == name) return &cls;
        }
        return nullptr;
    }
    
    bool HasClass(const std::string& name) const {
        return GetClass(name) != nullptr;
    }
};

} // namespace ReflectionGenerator

