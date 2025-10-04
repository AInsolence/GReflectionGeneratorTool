#include "ClassParser.h"
#include "CodeGenerator.h"
#include "FileScanner.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

void PrintUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --scan-dirs <dir1,dir2,...>  Directories to scan for reflection-enabled classes\n";
    std::cout << "  --output-dir <dir>           Output directory for generated files\n";
    std::cout << "  --input-files <file1,file2>  Specific files to process\n";
    std::cout << "  --verbose                   Enable verbose output\n";
    std::cout << "  --help                      Show this help message\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " --scan-dirs Engine,Game --output-dir Build/Generated\n";
    std::cout << "  " << programName << " --input-files Engine/Public/Core/Player.h --output-dir Build/Generated\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> scanDirs;
    std::vector<std::string> inputFiles;
    std::string outputDir = "Build/Generated";
    bool verbose = false;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            PrintUsage(argv[0]);
            return 0;
        }
        else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        }
        else if (arg == "--scan-dirs" && i + 1 < argc) {
            std::string dirs = argv[++i];
            size_t pos = 0;
            while (pos < dirs.length()) {
                size_t nextPos = dirs.find(',', pos);
                if (nextPos == std::string::npos) {
                    scanDirs.push_back(dirs.substr(pos));
                    break;
                }
                scanDirs.push_back(dirs.substr(pos, nextPos - pos));
                pos = nextPos + 1;
            }
        }
        else if (arg == "--input-files" && i + 1 < argc) {
            std::string files = argv[++i];
            size_t pos = 0;
            while (pos < files.length()) {
                size_t nextPos = files.find(',', pos);
                if (nextPos == std::string::npos) {
                    inputFiles.push_back(files.substr(pos));
                    break;
                }
                inputFiles.push_back(files.substr(pos, nextPos - pos));
                pos = nextPos + 1;
            }
        }
        else if (arg == "--output-dir" && i + 1 < argc) {
            outputDir = argv[++i];
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            PrintUsage(argv[0]);
            return 1;
        }
    }

    if (scanDirs.empty() && inputFiles.empty()) {
        std::cerr << "Error: No input directories or files specified\n";
        PrintUsage(argv[0]);
        return 1;
    }

    try {
        // Create output directory
        fs::create_directories(outputDir);

        ReflectionGenerator::FileScanner scanner;
        ReflectionGenerator::ClassParser parser;
        ReflectionGenerator::CodeGenerator generator(outputDir);

        std::vector<std::string> filesToProcess;

        // Collect files to process
        if (!inputFiles.empty()) {
            filesToProcess = inputFiles;
        } else {
            for (const auto& dir : scanDirs) {
                if (verbose) {
                    std::cout << "Scanning directory: " << dir << "\n";
                }
                auto files = scanner.ScanDirectory(dir);
                filesToProcess.insert(filesToProcess.end(), files.begin(), files.end());
            }
        }

        if (verbose) {
            std::cout << "Found " << filesToProcess.size() << " files to process\n";
        }

        int processedCount = 0;
        int generatedCount = 0;

        // Process each file
        for (const auto& filePath : filesToProcess) {
            if (verbose) {
                std::cout << "Processing: " << filePath << "\n";
            }

            try {
                auto classes = parser.ParseFile(filePath);
                if (!classes.empty()) {
                    generator.GenerateCode(filePath, classes);
                    generatedCount += classes.size();
                    if (verbose) {
                        std::cout << "  Generated reflection for " << classes.size() << " classes\n";
                    }
                }
                processedCount++;
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing " << filePath << ": " << e.what() << "\n";
            }
        }

        std::cout << "Reflection generation completed:\n";
        std::cout << "  Files processed: " << processedCount << "\n";
        std::cout << "  Classes generated: " << generatedCount << "\n";
        std::cout << "  Output directory: " << outputDir << "\n";

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}

