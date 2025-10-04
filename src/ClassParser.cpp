#include "ClassParser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <memory>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecordLayout.h>

namespace ReflectionGenerator {

// ReflectionASTVisitor implementation
ReflectionASTVisitor::ReflectionASTVisitor(clang::ASTContext* context, ReflectionData& data)
    : m_context(context), m_data(data) {
}

bool ReflectionASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* decl) {
    if (!decl || decl->isImplicit() || !decl->isCompleteDefinition()) {
        return true;
    }
    
    // Check if this class has GCLASS macro
    bool hasGClass = false;
    for (auto it = decl->specific_attr_begin<clang::AnnotateAttr>(); 
         it != decl->specific_attr_end<clang::AnnotateAttr>(); ++it) {
        if (it->getAnnotation().str().find("GCLASS") != std::string::npos) {
            hasGClass = true;
            break;
        }
    }
    
    if (!hasGClass) {
        return true;
    }
    
    // Create new class info
    ClassInfo classInfo;
    classInfo.name = decl->getNameAsString();
    classInfo.qualifiedName = GetQualifiedName(decl);
    classInfo.fileName = m_context->getSourceManager().getFilename(decl->getLocation()).str();
    classInfo.lineNumber = m_context->getSourceManager().getSpellingLineNumber(decl->getLocation());
    
    // Get namespace
    if (decl->isInAnonymousNamespace()) {
        classInfo.namespaceName = "";
    } else {
        auto* ns = clang::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext());
        if (ns && !ns->isAnonymousNamespace()) {
            classInfo.namespaceName = ns->getNameAsString();
        }
    }
    
    // Get base class
    if (decl->getNumBases() > 0) {
        auto base = decl->bases_begin();
        classInfo.baseClass = GetTypeAsString(base->getType());
    }
    
    // Set current class for property/function parsing
    m_currentClass = &classInfo;
    
    // Traverse the class to find properties and functions
    TraverseDecl(decl);
    
    // Add to data
    m_data.classes.push_back(classInfo);
    m_currentClass = nullptr;
    
    return true;
}

bool ReflectionASTVisitor::VisitFieldDecl(clang::FieldDecl* decl) {
    if (!m_currentClass || !decl) {
        return true;
    }
    
    // Check if this field has GPROPERTY macro
    bool hasGProperty = false;
    for (auto it = decl->specific_attr_begin<clang::AnnotateAttr>(); 
         it != decl->specific_attr_end<clang::AnnotateAttr>(); ++it) {
        if (it->getAnnotation().str().find("GPROPERTY") != std::string::npos) {
            hasGProperty = true;
            break;
        }
    }
    
    if (!hasGProperty) {
        return true;
    }
    
    // Create property info
    PropertyInfo propertyInfo;
    propertyInfo.name = decl->getNameAsString();
    propertyInfo.type = GetTypeAsString(decl->getType());
    propertyInfo.qualifiedType = GetTypeAsString(decl->getType());
    propertyInfo.fileName = m_context->getSourceManager().getFilename(decl->getLocation()).str();
    propertyInfo.lineNumber = m_context->getSourceManager().getSpellingLineNumber(decl->getLocation());
    
    // Calculate offset (simplified)
    if (auto* record = clang::dyn_cast<clang::CXXRecordDecl>(decl->getParent())) {
        if (record->isCompleteDefinition()) {
            const auto& layout = m_context->getASTRecordLayout(record);
            propertyInfo.offset = layout.getFieldOffset(decl->getFieldIndex());
        }
    }
    
    // Parse GPROPERTY macro arguments
    for (auto it = decl->specific_attr_begin<clang::AnnotateAttr>(); 
         it != decl->specific_attr_end<clang::AnnotateAttr>(); ++it) {
        std::string annotation = it->getAnnotation().str();
        if (annotation.find("GPROPERTY") != std::string::npos) {
            ParseGPropertyMacro(nullptr, propertyInfo); // Simplified for now
            break;
        }
    }
    
    m_currentClass->properties.push_back(propertyInfo);
    return true;
}

bool ReflectionASTVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl* decl) {
    if (!m_currentClass || !decl || decl->isImplicit()) {
        return true;
    }
    
    // Check if this method has GFUNCTION macro
    bool hasGFunction = false;
    for (auto it = decl->specific_attr_begin<clang::AnnotateAttr>(); 
         it != decl->specific_attr_end<clang::AnnotateAttr>(); ++it) {
        if (it->getAnnotation().str().find("GFUNCTION") != std::string::npos) {
            hasGFunction = true;
            break;
        }
    }
    
    if (!hasGFunction) {
        return true;
    }
    
    // Create function info
    FunctionInfo functionInfo;
    functionInfo.name = decl->getNameAsString();
    functionInfo.returnType = GetTypeAsString(decl->getReturnType());
    functionInfo.fileName = m_context->getSourceManager().getFilename(decl->getLocation()).str();
    functionInfo.lineNumber = m_context->getSourceManager().getSpellingLineNumber(decl->getLocation());
    
    // Get parameters
    for (auto param : decl->parameters()) {
        functionInfo.parameters.push_back(param->getNameAsString());
        functionInfo.parameterTypes.push_back(GetTypeAsString(param->getType()));
    }
    
    // Parse GFUNCTION macro arguments
    for (auto it = decl->specific_attr_begin<clang::AnnotateAttr>(); 
         it != decl->specific_attr_end<clang::AnnotateAttr>(); ++it) {
        std::string annotation = it->getAnnotation().str();
        if (annotation.find("GFUNCTION") != std::string::npos) {
            ParseGFunctionMacro(nullptr, functionInfo); // Simplified for now
            break;
        }
    }
    
    m_currentClass->functions.push_back(functionInfo);
    return true;
}

bool ReflectionASTVisitor::VisitMacroExpansion(clang::MacroExpansion* expansion) {
    if (!expansion) {
        return true;
    }
    
    std::string macroName = expansion->getName()->getName().str();
    if (IsReflectionMacro(macroName)) {
        // Handle macro expansion
        // This is a simplified implementation
        // In a full implementation, we would parse the macro arguments here
    }
    
    return true;
}

std::string ReflectionASTVisitor::GetQualifiedName(clang::NamedDecl* decl) {
    if (!decl) {
        return "";
    }
    
    std::string qualifiedName;
    auto* context = decl->getDeclContext();
    
    // Build qualified name from context
    std::vector<std::string> nameParts;
    while (context) {
        if (auto* ns = clang::dyn_cast<clang::NamespaceDecl>(context)) {
            if (!ns->isAnonymousNamespace()) {
                nameParts.push_back(ns->getNameAsString());
            }
        }
        context = context->getParent();
    }
    
    // Reverse to get correct order
    std::reverse(nameParts.begin(), nameParts.end());
    
    for (const auto& part : nameParts) {
        qualifiedName += part + "::";
    }
    
    qualifiedName += decl->getNameAsString();
    return qualifiedName;
}

std::string ReflectionASTVisitor::GetTypeAsString(clang::QualType type) {
    if (type.isNull()) {
        return "";
    }
    
    // Remove qualifiers and get canonical type
    clang::QualType canonicalType = type.getCanonicalType();
    
    // Get type name
    std::string typeName = canonicalType.getAsString();
    
    // Clean up the type name
    // Remove unnecessary spaces and qualifiers
    std::string cleanedName;
    bool inTemplate = false;
    int templateDepth = 0;
    
    for (char c : typeName) {
        if (c == '<') {
            inTemplate = true;
            templateDepth++;
        } else if (c == '>') {
            templateDepth--;
            if (templateDepth == 0) {
                inTemplate = false;
            }
        }
        
        if (!inTemplate && c == ' ') {
            continue; // Skip spaces outside templates
        }
        
        cleanedName += c;
    }
    
    return cleanedName;
}

std::string ReflectionASTVisitor::GetSourceText(clang::SourceRange range) {
    if (range.isInvalid()) {
        return "";
    }
    
    clang::SourceManager& sm = m_context->getSourceManager();
    clang::SourceLocation start = range.getBegin();
    clang::SourceLocation end = range.getEnd();
    
    // Get the text
    clang::CharSourceRange charRange = clang::CharSourceRange::getCharRange(start, end);
    return clang::Lexer::getSourceText(charRange, sm, clang::LangOptions()).str();
}

void ReflectionASTVisitor::ParseGClassMacro(clang::MacroExpansion* expansion, ClassInfo& classInfo) {
    // Simplified implementation
    // In a full implementation, we would parse the macro arguments
    classInfo.blueprintable = true;
    classInfo.serializable = true;
}

void ReflectionASTVisitor::ParseGPropertyMacro(clang::MacroExpansion* expansion, PropertyInfo& propertyInfo) {
    // Simplified implementation
    // In a full implementation, we would parse the macro arguments
    propertyInfo.save = true;
    propertyInfo.edit = true;
}

void ReflectionASTVisitor::ParseGFunctionMacro(clang::MacroExpansion* expansion, FunctionInfo& functionInfo) {
    // Simplified implementation
    // In a full implementation, we would parse the macro arguments
    functionInfo.callable = true;
}

std::vector<std::string> ReflectionASTVisitor::ParseMacroArguments(const std::string& macroText) {
    std::vector<std::string> args;
    
    // Find the opening parenthesis
    size_t start = macroText.find('(');
    if (start == std::string::npos) {
        return args;
    }
    
    // Find the closing parenthesis
    size_t end = macroText.rfind(')');
    if (end == std::string::npos || end <= start) {
        return args;
    }
    
    // Extract arguments
    std::string argsText = macroText.substr(start + 1, end - start - 1);
    args = SplitString(argsText, ',');
    
    // Trim whitespace from each argument
    for (auto& arg : args) {
        arg = TrimWhitespace(arg);
    }
    
    return args;
}

void ReflectionASTVisitor::ParseClassFlags(const std::vector<std::string>& args, ClassInfo& classInfo) {
    for (const auto& arg : args) {
        if (arg == "Blueprintable") {
            classInfo.blueprintable = true;
        } else if (arg == "Serializable") {
            classInfo.serializable = true;
        } else if (arg == "Abstract") {
            classInfo.abstract = true;
        } else if (arg == "DefaultToInstanced") {
            classInfo.defaultToInstanced = true;
        } else if (arg.find("Version=") == 0) {
            std::string versionStr = arg.substr(8);
            classInfo.version = std::stoul(versionStr);
        }
    }
}

void ReflectionASTVisitor::ParsePropertyFlags(const std::vector<std::string>& args, PropertyInfo& propertyInfo) {
    for (const auto& arg : args) {
        if (arg == "Save") {
            propertyInfo.save = true;
        } else if (arg == "Edit") {
            propertyInfo.edit = true;
        } else if (arg == "Transient") {
            propertyInfo.transient = true;
        } else if (arg == "EditorOnly") {
            propertyInfo.editorOnly = true;
        } else if (arg == "ReadOnly") {
            propertyInfo.readOnly = true;
        } else if (arg.find("Category(") == 0) {
            // Extract category name
            size_t start = arg.find('"');
            size_t end = arg.rfind('"');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                propertyInfo.category = arg.substr(start + 1, end - start - 1);
            }
        } else if (arg.find("Clamp(") == 0) {
            // Extract clamp values
            size_t start = arg.find('(');
            size_t end = arg.rfind(')');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string clampText = arg.substr(start + 1, end - start - 1);
                auto clampArgs = SplitString(clampText, ',');
                if (clampArgs.size() >= 2) {
                    propertyInfo.clampMin = TrimWhitespace(clampArgs[0]);
                    propertyInfo.clampMax = TrimWhitespace(clampArgs[1]);
                }
            }
        } else if (arg.find("Default(") == 0) {
            // Extract default value
            size_t start = arg.find('(');
            size_t end = arg.rfind(')');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                propertyInfo.defaultValue = arg.substr(start + 1, end - start - 1);
            }
        } else if (arg.find("Tooltip(") == 0) {
            // Extract tooltip
            size_t start = arg.find('"');
            size_t end = arg.rfind('"');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                propertyInfo.tooltip = arg.substr(start + 1, end - start - 1);
            }
        }
    }
}

void ReflectionASTVisitor::ParseFunctionFlags(const std::vector<std::string>& args, FunctionInfo& functionInfo) {
    for (const auto& arg : args) {
        if (arg == "Callable") {
            functionInfo.callable = true;
        } else if (arg == "BlueprintEvent") {
            functionInfo.blueprintEvent = true;
        } else if (arg == "BlueprintCallable") {
            functionInfo.blueprintCallable = true;
        } else if (arg.find("Category(") == 0) {
            // Extract category name
            size_t start = arg.find('"');
            size_t end = arg.rfind('"');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                functionInfo.category = arg.substr(start + 1, end - start - 1);
            }
        } else if (arg.find("Tooltip(") == 0) {
            // Extract tooltip
            size_t start = arg.find('"');
            size_t end = arg.rfind('"');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                functionInfo.tooltip = arg.substr(start + 1, end - start - 1);
            }
        }
    }
}

bool ReflectionASTVisitor::IsReflectionMacro(const std::string& macroName) {
    return macroName == "GCLASS" || macroName == "GPROPERTY" || macroName == "GFUNCTION";
}

std::string ReflectionASTVisitor::TrimWhitespace(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> ReflectionASTVisitor::SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

// ReflectionASTConsumer implementation
ReflectionASTConsumer::ReflectionASTConsumer(clang::ASTContext* context, ReflectionData& data)
    : m_context(context), m_data(data) {
    m_visitor = std::make_unique<ReflectionASTVisitor>(context, data);
}

void ReflectionASTConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    m_visitor->TraverseDecl(context.getTranslationUnitDecl());
}

// ReflectionFrontendAction implementation
ReflectionFrontendAction::ReflectionFrontendAction(ReflectionData& data)
    : m_data(data) {
}

std::unique_ptr<clang::ASTConsumer> ReflectionFrontendAction::CreateASTConsumer(
    clang::CompilerInstance& compiler,
    llvm::StringRef file) {
    
    return std::make_unique<ReflectionASTConsumer>(&compiler.getASTContext(), m_data);
}

// ClassParser implementation
ClassParser::ClassParser() {
    // Set default include directories
    m_includeDirs.push_back("Engine/Public");
    m_includeDirs.push_back("Game");
    m_includeDirs.push_back("External/GLM");
    m_includeDirs.push_back("External/GLEW/include");
    m_includeDirs.push_back("External/GLFW/include");
    m_includeDirs.push_back("External/ASSIMP/include");
    
    // Set default definitions
    m_definitions.push_back("ENGINE_REFLECTION_GENERATION=1");
}

std::vector<ClassInfo> ClassParser::ParseFile(const std::string& filePath) {
    ReflectionData data;
    data.fileName = filePath;
    
    // Build compiler arguments
    std::vector<std::string> args = BuildCompilerArgs(filePath);
    
    // Create tool
    auto compilations = std::make_unique<clang::tooling::FixedCompilationDatabase>(".", args);
    clang::tooling::ClangTool tool(*compilations, {filePath});
    
    // Create a custom factory
    class ReflectionActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        explicit ReflectionActionFactory(ReflectionData& data) : m_data(data) {}
        
        std::unique_ptr<clang::FrontendAction> create() override {
            return std::make_unique<ReflectionFrontendAction>(m_data);
        }
        
    private:
        ReflectionData& m_data;
    };
    
    // Run the tool
    ReflectionActionFactory factory(data);
    int result = tool.run(&factory);
    
    if (result != 0) {
        std::cerr << "Error parsing file: " << filePath << "\n";
        return {};
    }
    
    return data.classes;
}

std::vector<ClassInfo> ClassParser::ParseFiles(const std::vector<std::string>& filePaths) {
    std::vector<ClassInfo> allClasses;
    
    for (const auto& filePath : filePaths) {
        auto classes = ParseFile(filePath);
        allClasses.insert(allClasses.end(), classes.begin(), classes.end());
    }
    
    return allClasses;
}

void ClassParser::SetIncludeDirectories(const std::vector<std::string>& includeDirs) {
    m_includeDirs = includeDirs;
}

void ClassParser::SetDefinitions(const std::vector<std::string>& definitions) {
    m_definitions = definitions;
}

std::vector<std::string> ClassParser::BuildCompilerArgs(const std::string& filePath) {
    std::vector<std::string> args;
    
    // Add include directories
    for (const auto& includeDir : m_includeDirs) {
        args.push_back("-I" + includeDir);
    }
    
    // Add definitions
    for (const auto& definition : m_definitions) {
        args.push_back("-D" + definition);
    }
    
    // Add standard include path
    std::string stdIncludePath = GetStandardIncludePath();
    if (!stdIncludePath.empty()) {
        args.push_back("-I" + stdIncludePath);
    }
    
    // Add C++ standard
    args.push_back("-std=c++23");
    
    // Add other flags
    args.push_back("-fparse-all-comments");
    args.push_back("-Wno-pragma-once-outside-header");
    
    return args;
}

std::string ClassParser::GetStandardIncludePath() {
    // This is a simplified implementation
    // In a real implementation, we would query the compiler for the standard include path
    return "";
}

} // namespace ReflectionGenerator

