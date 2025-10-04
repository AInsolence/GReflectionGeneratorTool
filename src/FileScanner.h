#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace ReflectionGenerator {

/**
 * Scans directories for C++ header files that might contain reflection-enabled classes
 */
class FileScanner {
public:
    FileScanner() = default;
    ~FileScanner() = default;

    /**
     * Scan a directory for C++ header files
     * @param directory Path to directory to scan
     * @return Vector of header file paths
     */
    std::vector<std::string> ScanDirectory(const std::string& directory);

    /**
     * Check if a file should be processed for reflection
     * @param filePath Path to the file
     * @return True if file should be processed
     */
    bool ShouldProcessFile(const std::string& filePath);

    /**
     * Get all header files in a directory recursively
     * @param directory Path to directory
     * @param extensions File extensions to include (e.g., {".h", ".hpp"})
     * @return Vector of header file paths
     */
    std::vector<std::string> GetHeaderFiles(
        const std::string& directory,
        const std::vector<std::string>& extensions = {".h", ".hpp"}
    );

private:
    /**
     * Check if a file contains reflection macros
     * @param filePath Path to the file
     * @return True if file contains GCLASS, GPROPERTY, or GFUNCTION macros
     */
    bool ContainsReflectionMacros(const std::string& filePath);

    /**
     * Check if a directory should be excluded from scanning
     * @param dirPath Path to directory
     * @return True if directory should be excluded
     */
    bool ShouldExcludeDirectory(const std::string& dirPath);

    // Common directories to exclude
    static const std::vector<std::string> s_excludedDirectories;
    static const std::vector<std::string> s_headerExtensions;
};

} // namespace ReflectionGenerator

