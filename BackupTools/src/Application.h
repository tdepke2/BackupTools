#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FileHandler.h"
#include <chrono>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

bool compareFileChange(const std::pair<fs::path, fs::path>& lhs, const std::pair<fs::path, fs::path>& rhs);

class Application {
    public:
    struct FileChanges {
        std::set<fs::path, decltype(&compareFilename)> deletions;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> additions;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> modifications;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> renames;
        
        FileChanges() : deletions(&compareFilename), additions(&compareFileChange), modifications(&compareFileChange), renames(&compareFileChange) {}
        bool isEmpty() const { return deletions.empty() && additions.empty() && modifications.empty() && renames.empty(); }
        size_t getCount() const { return deletions.size() + additions.size() + modifications.size() + renames.size(); }
    };
    
    static bool checkUserConfirmation();    // Gets user input and returns true only if any variant of "yes" was entered.
    void printPaths(const fs::path& configFilename, bool verbose, bool countOnly);
    FileChanges checkBackup(const fs::path& configFilename, size_t outputLimit, bool displayConfirmation = false, bool silent = false);
    void printChanges(const FileChanges& changes, size_t outputLimit, bool displayConfirmation = false);
    void startBackup(const fs::path& configFilename, size_t outputLimit, bool forceBackup);
    
    private:
    struct PrintTreeStats {
        uintmax_t numDirectories;
        uintmax_t numFiles;
        uintmax_t numIgnoredDirectories;
        uintmax_t numIgnoredFiles;
        
        PrintTreeStats() : numDirectories(0), numFiles(0), numIgnoredDirectories(0), numIgnoredFiles(0) {}
    };
    
    static void printTree(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, bool verbose, bool countOnly);
    static void printTree2(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, bool verbose, bool printOutput, const std::string& prefix, PrintTreeStats* stats);
    static void optimizeForRenames(FileChanges& changes);    // Modifies changes so that files that are equivalent and have different paths are removed from changes.deletions/changes.additions and added to changes.renames.
    static void printSpinner(int& index, std::chrono::steady_clock::time_point& lastTime);    // Displays a spinner at the cursor, the spinner only updates every 200ms.
    static void printProgressBar(double progress);    // Display progress bar with a message below. Value for progress must be in the 0.0 to 1.0 range.
};

#endif
