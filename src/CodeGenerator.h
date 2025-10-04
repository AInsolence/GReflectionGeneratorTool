#pragma once

#include "ReflectionAST.h"
#include <string>
#include <fstream>
#include <filesystem>

namespace ReflectionGenerator {

/**
 * Generates reflection code from parsed class information
 */
class CodeGenerator {
public:
    explicit CodeGenerator(const std::string& outputDir);
    ~CodeGenerator() = default;

    /**
     * Generate reflection code for a file
     * @param filePath Path to the source file
     * @param classes Vector of ClassInfo objects to generate code for
     */
    void GenerateCode(const std::string& filePath, const std::vector<ClassInfo>& classes);

    /**
     * Generate header file for a class
     * @param classInfo Class information
     * @param outputPath Output file path
     */
    void GenerateHeader(const ClassInfo& classInfo, const std::string& outputPath);

    /**
     * Generate implementation file for a class
     * @param classInfo Class information
     * @param outputPath Output file path
     */
    void GenerateImplementation(const ClassInfo& classInfo, const std::string& outputPath);

    /**
     * Generate registration code for all classes
     * @param classes Vector of all ClassInfo objects
     * @param outputPath Output file path
     */
    void GenerateRegistration(const std::vector<ClassInfo>& classes, const std::string& outputPath);

private:
    std::string m_outputDir;
    
    // Helper methods
    std::string GetOutputPath(const std::string& filePath, const std::string& suffix);
    std::string GetIncludeGuard(const std::string& className);
    std::string GetNamespacePrefix(const std::string& namespaceName);
    std::string GetNamespaceSuffix(const std::string& namespaceName);
    
    // Code generation helpers
    void WriteHeaderPreamble(std::ofstream& file, const ClassInfo& classInfo);
    void WriteHeaderIncludes(std::ofstream& file, const ClassInfo& classInfo);
    void WriteHeaderClass(std::ofstream& file, const ClassInfo& classInfo);
    void WriteHeaderEpilogue(std::ofstream& file, const ClassInfo& classInfo);
    
    void WriteImplementationPreamble(std::ofstream& file, const ClassInfo& classInfo);
    void WriteImplementationIncludes(std::ofstream& file, const ClassInfo& classInfo);
    void WriteImplementationClass(std::ofstream& file, const ClassInfo& classInfo);
    void WriteImplementationEpilogue(std::ofstream& file, const ClassInfo& classInfo);
    
    // Property and function generation
    void WritePropertyRegistration(std::ofstream& file, const PropertyInfo& property);
    void WriteFunctionRegistration(std::ofstream& file, const FunctionInfo& function);
    
    // Serialization generation
    void WriteSerializationCode(std::ofstream& file, const ClassInfo& classInfo);
    void WriteDeserializationCode(std::ofstream& file, const ClassInfo& classInfo);
    
    // Utility methods
    std::string GetPropertyFlagsString(const PropertyInfo& property);
    std::string GetFunctionFlagsString(const FunctionInfo& function);
    std::string GetClassFlagsString(const ClassInfo& classInfo);
    std::string GetTypeRegistrationName(const std::string& typeName);
    std::string GetPropertyTypeName(const std::string& typeName);
    
    // File system helpers
    void EnsureDirectoryExists(const std::string& path);
    std::string GetRelativePath(const std::string& from, const std::string& to);
};

} // namespace ReflectionGenerator

