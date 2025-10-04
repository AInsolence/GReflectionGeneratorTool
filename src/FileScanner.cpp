#include "FileScanner.h"
#include <fstream>
#include <iostream>
#include <regex>

namespace ReflectionGenerator {

const std::vector<std::string> FileScanner::s_excludedDirectories = {
    "External",
    "Build",
    "bin",
    "lib",
    "obj",
    "Debug",
    "Release",
    "x64",
    "x86",
    ".git",
    ".vs",
    "CMakeFiles",
    "node_modules"
};

const std::vector<std::string> FileScanner::s_headerExtensions = {
    ".h",
    ".hpp",
    ".hxx",
    ".hh"
};

std::vector<std::string> FileScanner::ScanDirectory(const std::string& directory) {
    std::vector<std::string> result;
    
    try {
        std::filesystem::path dirPath(directory);
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            std::cerr << "Warning: Directory does not exist or is not a directory: " << directory << "\n";
            return result;
        }

        auto headerFiles = GetHeaderFiles(directory);
        
        for (const auto& filePath : headerFiles) {
            if (ShouldProcessFile(filePath)) {
                result.push_back(filePath);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error scanning directory " << directory << ": " << e.what() << "\n";
    }
    
    return result;
}

bool FileScanner::ShouldProcessFile(const std::string& filePath) {
    // Check if file has a header extension
    std::filesystem::path path(filePath);
    std::string extension = path.extension().string();
    
    bool hasHeaderExtension = false;
    for (const auto& ext : s_headerExtensions) {
        if (extension == ext) {
            hasHeaderExtension = true;
            break;
        }
    }
    
    if (!hasHeaderExtension) {
        return false;
    }
    
    // Check if file contains reflection macros
    return ContainsReflectionMacros(filePath);
}

std::vector<std::string> FileScanner::GetHeaderFiles(
    const std::string& directory,
    const std::vector<std::string>& extensions) {
    
    std::vector<std::string> result;
    
    try {
        std::filesystem::path dirPath(directory);
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();
                std::string extension = entry.path().extension().string();
                
                // Check if file has one of the desired extensions
                bool hasDesiredExtension = false;
                for (const auto& ext : extensions) {
                    if (extension == ext) {
                        hasDesiredExtension = true;
                        break;
                    }
                }
                
                if (hasDesiredExtension) {
                    // Check if any parent directory should be excluded
                    bool shouldExclude = false;
                    std::filesystem::path currentPath = entry.path();
                    
                    while (currentPath.has_parent_path() && currentPath != dirPath) {
                        currentPath = currentPath.parent_path();
                        std::string dirName = currentPath.filename().string();
                        
                        if (ShouldExcludeDirectory(dirName)) {
                            shouldExclude = true;
                            break;
                        }
                    }
                    
                    if (!shouldExclude) {
                        result.push_back(filePath);
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error getting header files from " << directory << ": " << e.what() << "\n";
    }
    
    return result;
}

bool FileScanner::ContainsReflectionMacros(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        std::string line;
        std::regex gclassRegex(R"(GCLASS\s*\()");
        std::regex gpropertyRegex(R"(GPROPERTY\s*\()");
        std::regex gfunctionRegex(R"(GFUNCTION\s*\()");
        
        while (std::getline(file, line)) {
            if (std::regex_search(line, gclassRegex) ||
                std::regex_search(line, gpropertyRegex) ||
                std::regex_search(line, gfunctionRegex)) {
                return true;
            }
        }
        
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading file " << filePath << ": " << e.what() << "\n";
        return false;
    }
}

bool FileScanner::ShouldExcludeDirectory(const std::string& dirPath) {
    for (const auto& excluded : s_excludedDirectories) {
        if (dirPath == excluded) {
            return true;
        }
    }
    return false;
}

} // namespace ReflectionGenerator

