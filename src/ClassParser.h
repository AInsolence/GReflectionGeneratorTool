#pragma once

#include "ReflectionAST.h"
#include <string>
#include <vector>
#include <memory>

// Clang includes
#include "clang/Tooling/Tooling.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/MacroInfo.h"

namespace ReflectionGenerator {

/**
 * AST visitor that finds reflection-enabled classes and extracts their information
 */
class ReflectionASTVisitor : public clang::RecursiveASTVisitor<ReflectionASTVisitor> {
public:
    explicit ReflectionASTVisitor(clang::ASTContext* context, ReflectionData& data);
    
    // Visit declarations
    bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl);
    bool VisitFieldDecl(clang::FieldDecl* decl);
    bool VisitCXXMethodDecl(clang::CXXMethodDecl* decl);
    
    // Visit macro expansions
    bool VisitMacroExpansion(clang::MacroExpansion* expansion);

private:
    clang::ASTContext* m_context;
    ReflectionData& m_data;
    ClassInfo* m_currentClass = nullptr;
    
    // Helper methods
    std::string GetQualifiedName(clang::NamedDecl* decl);
    std::string GetTypeAsString(clang::QualType type);
    std::string GetSourceText(clang::SourceRange range);
    
    // Macro parsing
    void ParseGClassMacro(clang::MacroExpansion* expansion, ClassInfo& classInfo);
    void ParseGPropertyMacro(clang::MacroExpansion* expansion, PropertyInfo& propertyInfo);
    void ParseGFunctionMacro(clang::MacroExpansion* expansion, FunctionInfo& functionInfo);
    
    // Parse macro arguments
    std::vector<std::string> ParseMacroArguments(const std::string& macroText);
    void ParseClassFlags(const std::vector<std::string>& args, ClassInfo& classInfo);
    void ParsePropertyFlags(const std::vector<std::string>& args, PropertyInfo& propertyInfo);
    void ParseFunctionFlags(const std::vector<std::string>& args, FunctionInfo& functionInfo);
    
    // Utility methods
    bool IsReflectionMacro(const std::string& macroName);
    std::string TrimWhitespace(const std::string& str);
    std::vector<std::string> SplitString(const std::string& str, char delimiter);
};

/**
 * AST consumer that processes the AST and extracts reflection information
 */
class ReflectionASTConsumer : public clang::ASTConsumer {
public:
    explicit ReflectionASTConsumer(clang::ASTContext* context, ReflectionData& data);
    
    void HandleTranslationUnit(clang::ASTContext& context) override;

private:
    clang::ASTContext* m_context;
    ReflectionData& m_data;
    std::unique_ptr<ReflectionASTVisitor> m_visitor;
};

/**
 * Frontend action for reflection parsing
 */
class ReflectionFrontendAction : public clang::ASTFrontendAction {
public:
    explicit ReflectionFrontendAction(ReflectionData& data);
    
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler,
        llvm::StringRef file) override;

private:
    ReflectionData& m_data;
};

/**
 * Main class parser that uses Clang LibTooling to parse C++ files
 */
class ClassParser {
public:
    ClassParser();
    ~ClassParser() = default;

    /**
     * Parse a C++ file and extract reflection information
     * @param filePath Path to the C++ file
     * @return Vector of ClassInfo objects found in the file
     */
    std::vector<ClassInfo> ParseFile(const std::string& filePath);

    /**
     * Parse multiple files
     * @param filePaths Vector of file paths
     * @return Vector of ClassInfo objects from all files
     */
    std::vector<ClassInfo> ParseFiles(const std::vector<std::string>& filePaths);

    /**
     * Set additional include directories for parsing
     * @param includeDirs Vector of include directory paths
     */
    void SetIncludeDirectories(const std::vector<std::string>& includeDirs);

    /**
     * Set preprocessor definitions
     * @param definitions Vector of preprocessor definitions
     */
    void SetDefinitions(const std::vector<std::string>& definitions);

private:
    std::vector<std::string> m_includeDirs;
    std::vector<std::string> m_definitions;
    
    // Helper methods
    std::vector<std::string> BuildCompilerArgs(const std::string& filePath);
    std::string GetStandardIncludePath();
};

} // namespace ReflectionGenerator

