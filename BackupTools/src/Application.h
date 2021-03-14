#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FileHandler.h"
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
    };
    
    static bool checkUserConfirmation();    // Gets user input and returns true only if any variant of "yes" was entered.
    void printPaths(const fs::path& configFilename, const bool countOnly);
    FileChanges checkBackup(const fs::path& configFilename, size_t outputLimit, bool displayConfirmation = false, bool silent = false);
    void startBackup(const fs::path& configFilename, size_t outputLimit, bool forceBackup);
    void restoreFromBackup(const fs::path& configFilename);
    
    private:
    struct PrintTreeStats {
        uintmax_t numDirectories;
        uintmax_t numFiles;
        uintmax_t numIgnoredDirectories;
        uintmax_t numIgnoredFiles;
        
        PrintTreeStats() : numDirectories(0), numFiles(0), numIgnoredDirectories(0), numIgnoredFiles(0) {}
    };
    
    static void printTree(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, const bool countOnly);
    static void printTree2(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, const bool printOutput, const std::string& prefix, PrintTreeStats* stats);
    static void optimizeForRenames(FileChanges& changes);    // Modifies changes so that files that are equivalent and have different paths are removed from changes.deletions/changes.additions and added to changes.renames.
};

#endif
