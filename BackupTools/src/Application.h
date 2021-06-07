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

/**
 * Comparator function to sort the second member of pairs by filename (case is
 * ignored).
 */
bool compareFileChange(const std::pair<fs::path, fs::path>& lhs, const std::pair<fs::path, fs::path>& rhs);

/**
 * Contains the implementation for the CLI functions (backup, check, tree).
 */
class Application {
    public:
    /**
     * Keeps track of all the changes to make during a backup. Uses a custom
     * compare function to ignore case when sorting names of files.
     */
    struct FileChanges {
        std::set<fs::path, decltype(&compareFilename)> deletions;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> additions;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> modifications;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> renames;
        
        FileChanges() : deletions(&compareFilename), additions(&compareFileChange), modifications(&compareFileChange), renames(&compareFileChange) {}
        bool isEmpty() const { return deletions.empty() && additions.empty() && modifications.empty() && renames.empty(); }
        size_t getCount() const { return deletions.size() + additions.size() + modifications.size() + renames.size(); }
    };
    
    /**
     * Gets user input and returns true only if any variant of "yes" was
     * entered.
     */
    static bool checkUserConfirmation();
    
    /**
     * Displays tree of tracked files.
     */
    void printPaths(const fs::path& configFilename, bool verbose, bool countOnly);
    
    /**
     * Lists changes to make during backup.
     */
    FileChanges checkBackup(const fs::path& configFilename, size_t outputLimit, bool displayConfirmation = false, bool quiet = false);
    
    /**
     * Prints a list of deletions, additions, modifications, renames, and the
     * totals from the given changes.
     */
    void printChanges(const FileChanges& changes, size_t outputLimit, bool displayConfirmation = false);
    
    /**
     * Starts a backup/restore of files.
     */
    void startBackup(const fs::path& configFilename, size_t outputLimit, bool forceBackup);
    
    private:
    /**
     * Used in printTree() to display totals at the end.
     */
    struct PrintTreeStats {
        uintmax_t numDirectories;
        uintmax_t numFiles;
        uintmax_t numIgnoredDirectories;
        uintmax_t numIgnoredFiles;
        
        PrintTreeStats() : numDirectories(0), numFiles(0), numIgnoredDirectories(0), numIgnoredFiles(0) {}
    };
    
    /**
     * Used in printPaths() to handle the output of the file tree once the split
     * points for each tree are found (a tree cannot span multiple root paths).
     */
    static void printTree(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, bool verbose, bool countOnly);
    
    /**
     * Recursive call in printTree().
     */
    static void printTree2(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, bool verbose, bool printOutput, const std::string& prefix, PrintTreeStats* stats);
    
    /**
     * Modifies changes so that files that are equivalent and have different
     * paths are removed from changes.deletions/changes.additions and added to
     * changes.renames.
     */
    static void optimizeForRenames(FileChanges& changes);
    
    /**
     * Displays a spinner at the cursor, the spinner only updates every 200ms.
     */
    static void printSpinner(int& index, std::chrono::steady_clock::time_point& lastTime);
    
    /**
     * Display progress bar with a message below. Value for progress must be in
     * the 0.0 to 1.0 range.
     */
    static void printProgressBar(double progress);
};

#endif
