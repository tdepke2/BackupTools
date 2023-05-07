#include "BackupTools/Application.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>

bool compareFileChange(const std::pair<fs::path, fs::path>& lhs, const std::pair<fs::path, fs::path>& rhs) {
    return compareFilename(lhs.second, rhs.second);
}

bool Application::checkUserConfirmation() {
    std::string input, inputCleaned;
    std::getline(std::cin, input);
    inputCleaned.reserve(input.size());
    for (char c : input) {
        if (c != ' ') {
            inputCleaned.push_back(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    std::set<std::string> acceptInputs = {
        "y", "ya", "ye", "yas", "yea", "yee", "yep", "yes", "yeah", "yessir", "affirmative", "true", "sure"
    };
    return acceptInputs.count(inputCleaned) > 0;
}

void Application::printPaths(const fs::path& configFilename, bool verbose, bool countOnly, bool pruneIgnored) {
    std::map<fs::path, fs::path> readPathsMapping;    // Maps read path to corresponding write path.
    std::map<fs::path, std::string> longestParentPaths;    // Longest common path among readPath entries (per root path).
    FileHandler fileHandler;
    fileHandler.loadConfigFile(configFilename);
    
    int spinnerIndex = 0;
    std::chrono::steady_clock::time_point spinnerLastTime = std::chrono::steady_clock::now();
    std::cout << "Scanning directory structure...\n";
    size_t scanCounter = 0;
    
    WriteReadPathTree pathTree = fileHandler.nextWriteReadPathTree();
    auto relativePathIter = pathTree.relativePaths.begin();
    scanCounter += pathTree.relativePaths.size();
    
    if (pathTree.isEmpty()) {
        std::cout << "\nNo files or directories found to track.\n";
        return;
    }
    
    while (!pathTree.isEmpty()) {
        if (relativePathIter == pathTree.relativePaths.end()) {    // If end of relative paths, grab a new path tree.
            pathTree = fileHandler.nextWriteReadPathTree();
            relativePathIter = pathTree.relativePaths.begin();
            scanCounter += pathTree.relativePaths.size();
            continue;
        }
        
        fs::path readPath = pathTree.readPrefix / *relativePathIter;
        fs::path writePath;
        if (verbose) {    // Only set the writePath if we actually use it.
            writePath = pathTree.writePrefix / *relativePathIter;
        }
        if (!readPathsMapping.emplace(readPath, writePath).second) {
            std::cout << CSI::Yellow << "Warning: Skipping duplicate read path: " << readPath.string() << CSI::Reset << "\n";
        }
        auto findResult = longestParentPaths.find(readPath.root_path());
        if (findResult == longestParentPaths.end()) {    // New root encountered, add entry for it.
            findResult = longestParentPaths.emplace(readPath.root_path(), readPath.string()).first;
        }
        
        findCommonParentPath(findResult->second, readPath.string(), readPath.root_path().string());    // Update the longest parent path.
        
        printSpinner(spinnerIndex, spinnerLastTime);
        ++relativePathIter;
    }
    std::cout << "Discovered " << scanCounter << " items.\n\n";    // Clear spinner and output scan totals.
    
    for (auto mapIter = longestParentPaths.begin(); mapIter != longestParentPaths.end(); ++mapIter) {
        if (mapIter != longestParentPaths.begin()) {
            std::cout << "\n";
        }
        if (fs::is_regular_file(mapIter->second) && mapIter->second.rfind(FileHandler::pathSeparator) != std::string::npos) {    // If parent path is a file, cut off the file portion before call to printTree().
            std::string::size_type previousSeparator = mapIter->second.rfind(FileHandler::pathSeparator);
            fs::path searchPath;
            if (previousSeparator == std::string::npos || mapIter->first.string().length() > previousSeparator) {    // Edge case in case we're close to the root path.
                searchPath = mapIter->first;
            } else {
                searchPath = mapIter->second.substr(0, previousSeparator);
            }
            printTree(searchPath, readPathsMapping, verbose, countOnly, pruneIgnored);
        } else {
            printTree(fs::path(mapIter->second), readPathsMapping, verbose, countOnly, pruneIgnored);
        }
    }
}

Application::FileChanges Application::checkBackup(const fs::path& configFilename, const BackupOptions& options) {
    FileChanges changes;
    std::map<fs::path, std::set<fs::path>> writePathsChecklist;    // Maps a destination path to the current contents of that path.
    auto lastWritePathIter = writePathsChecklist.end();
    FileHandler fileHandler;
    fileHandler.loadConfigFile(configFilename);
    
    fs::path cacheFilePath(".backuptools/" + configFilename.string() + ".cache");
    fs::file_time_type configFileWriteTime = fs::last_write_time(configFilename);
    if (!options.skipCache && fs::exists(cacheFilePath)) {
        std::cout << "Parsing cache file...";
        if (!fileHandler.loadCacheFile(cacheFilePath, configFileWriteTime)) {
            std::cout << " Canceled (config file was updated).";
        }
        std::cout << "\n";
    }
    
    int spinnerIndex = 0;
    std::chrono::steady_clock::time_point spinnerLastTime = std::chrono::steady_clock::now();
    std::cout << "Scanning for changes...\n";
    size_t scanCounter = 0;
    
    WriteReadPathTree pathTree = fileHandler.nextWriteReadPathTree();
    auto relativePathIter = pathTree.relativePaths.begin();
    scanCounter += pathTree.relativePaths.size();
    
    while (!pathTree.isEmpty()) {
        if (relativePathIter == pathTree.relativePaths.end()) {    // If end of relative paths, grab a new path tree.
            pathTree = fileHandler.nextWriteReadPathTree();
            relativePathIter = pathTree.relativePaths.begin();
            scanCounter += pathTree.relativePaths.size();
            continue;
        }
        
        if (relativePathIter == pathTree.relativePaths.begin()) {    // If first path in the set, add directory contents if it is a new writePrefix.
            auto insertResult = writePathsChecklist.emplace(pathTree.writePrefix, std::set<fs::path>());
            if (insertResult.second) {
                try {
                    for (const auto& entry : fs::recursive_directory_iterator(pathTree.writePrefix)) {
                        insertResult.first->second.emplace(entry.path());
                    }
                } catch (fs::filesystem_error&) {    // If exception during iteration of writePrefix, assume the directory does not currently exist and attempt to create it.
                    fs::create_directories(pathTree.writePrefix);
                }
            }
            
            lastWritePathIter = insertResult.first;
        }
        
        fs::path readPath = pathTree.readPrefix / *relativePathIter;
        fs::path writePath = pathTree.writePrefix / *relativePathIter;
        if (lastWritePathIter->second.erase(writePath) == 0) {    // Attempt to remove the write path from the checklist. If it's not found, then it doesn't currently exist and needs to be added.
            auto emplaceResult = changes.additions.emplace(readPath, writePath);
            assert(emplaceResult.second);
        } else if (!fileHandler.checkFileEquivalence(readPath, writePath, options.skipCache, options.fastCompare)) {    // If file exists but contents differ, it needs to be updated.
            auto emplaceResult = changes.modifications.emplace(readPath, writePath);
            assert(emplaceResult.second);
        }
        
        printSpinner(spinnerIndex, spinnerLastTime);
        ++relativePathIter;
    }
    
    for (auto& writePath : writePathsChecklist) {    // Any remaining paths in the checklist (that do not match an ignore) do not belong, mark these for deletion.
        for (auto setIter = writePath.second.rbegin(); setIter != writePath.second.rend(); ++setIter) {    // Iterate in reverse order to check paths at the leaves of the directory tree first.
            if (!fileHandler.checkPathIgnored(*setIter)) {
                auto emplaceResult = changes.deletions.emplace(*setIter);
                assert(emplaceResult.second);
            } else {    // If one of these matches an ignore, it's parent paths are also ignored so they don't get deleted.
                fs::path ignoredPath = *setIter;
                std::string ignoredPathStr = ignoredPath.string();
                for (std::string::size_type lastSeparator = ignoredPathStr.rfind(FileHandler::pathSeparator); lastSeparator != std::string::npos; lastSeparator = ignoredPathStr.rfind(FileHandler::pathSeparator)) {
                    ignoredPathStr.erase(lastSeparator);
                    writePath.second.erase(fs::path(ignoredPathStr));
                }
                setIter = std::make_reverse_iterator(writePath.second.find(ignoredPath));
                assert(setIter != writePath.second.rbegin());
                --setIter;    // Converting to a reverse_iterator advances by 1, undo this.
            }
        }
        printSpinner(spinnerIndex, spinnerLastTime);
    }
    writePathsChecklist.clear();
    
    optimizeForRenames(fileHandler, changes, options.skipCache, options.fastCompare);
    
    if (!options.skipCache) {
        fs::create_directory(cacheFilePath.parent_path());
        fileHandler.saveCacheFile(cacheFilePath, configFileWriteTime);
    }
    
    std::cout << "Discovered " << scanCounter << " items.\n\n";    // Clear spinner and output scan totals.
    if (!options.forceBackup) {
        printChanges(changes, options.outputLimit, options.displayConfirmation);
    }
    
    return changes;
}

void Application::printChanges(const FileChanges& changes, size_t outputLimit, bool displayConfirmation) {
    if (changes.isEmpty()) {
        std::cout << "All up to date.\n";
        return;
    }
    
    if (!changes.deletions.empty()) {
        std::cout << "Deletions:\n" << CSI::Red;
        size_t i = 0;
        for (const auto& p : changes.deletions) {
            if (i == outputLimit) {
                std::cout << "    (and " << changes.deletions.size() - i << " more)\n";
                break;
            }
            std::cout << "-   " << p.string() << "\n";
            ++i;
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!changes.additions.empty()) {
        std::cout << "Additions:\n" << CSI::Green;
        size_t i = 0;
        for (const auto& p : changes.additions) {
            if (i == outputLimit) {
                std::cout << "    (and " << changes.additions.size() - i << " more)\n";
                break;
            }
            std::cout << "+   " << p.second.string() << "\n";
            ++i;
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!changes.modifications.empty()) {
        std::cout << "Modifications:\n" << CSI::Yellow;
        size_t i = 0;
        for (const auto& p : changes.modifications) {
            if (i == outputLimit) {
                std::cout << "    (and " << changes.modifications.size() - i << " more)\n";
                break;
            }
            std::cout << "*   " << p.second.string() << "\n";
            ++i;
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!changes.renames.empty()) {
        std::cout << "Renames:\n" << CSI::Magenta;
        size_t i = 0;
        for (const auto& p : changes.renames) {
            if (i == outputLimit) {
                std::cout << "    (and " << changes.renames.size() - i << " more)\n";
                break;
            }
            std::cout << "~   " << p.first.string() << " -> " << p.second.string() << "\n";
            ++i;
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (displayConfirmation) {
        std::cout << "After this operation:\n";
        if (!changes.deletions.empty()) {
            std::cout << std::setw(5) << changes.deletions.size() << " item(s) will be removed.\n";
        }
        if (!changes.additions.empty()) {
            std::cout << std::setw(5) << changes.additions.size() << " item(s) will be added.\n";
        }
        if (!changes.modifications.empty()) {
            std::cout << std::setw(5) << changes.modifications.size() << " item(s) will be modified.\n";
        }
        if (!changes.renames.empty()) {
            std::cout << std::setw(5) << changes.renames.size() << " item(s) will be renamed.\n";
        }
        std::cout << "\nDo you want to continue [Y/n]? ";
    } else {
        if (!changes.deletions.empty()) {
            std::cout << std::setw(5) << changes.deletions.size() << " item(s) to remove.\n";
        }
        if (!changes.additions.empty()) {
            std::cout << std::setw(5) << changes.additions.size() << " item(s) to add.\n";
        }
        if (!changes.modifications.empty()) {
            std::cout << std::setw(5) << changes.modifications.size() << " item(s) to modify.\n";
        }
        if (!changes.renames.empty()) {
            std::cout << std::setw(5) << changes.renames.size() << " item(s) to rename.\n";
        }
    }
}

void Application::startBackup(const fs::path& configFilename, const BackupOptions& options) {
    FileChanges changes = checkBackup(configFilename, options);
    if (changes.isEmpty()) {
        return;
    }
    if (!options.forceBackup && !checkUserConfirmation()) {
        std::cout << "\nBackup canceled.\n";
        return;
    }
    
    size_t numOperations = changes.getCount();
    size_t numCompleted = 0;
    std::cout << "\n\n\n";    // Go down 3 lines (1 for spacing, 2 for printProgressBar() alignment).
    
    for (const auto& p : changes.additions) {
        printProgressBar(numCompleted, numOperations);
        std::cout << "Adding " << p.second.string() << "\n";
        if (fs::is_directory(p.first)) {
            fs::create_directory(p.second);
        } else {
            fs::copy_file(p.first, p.second);    // Note, fs::copy_file() is used explicitly here since there seems to be some bugs present in fs::copy() (observed when copying single file from FAT32 to NTFS drive).
        }
        ++numCompleted;
    }
    
    for (const auto& p : changes.renames) {    // Renaming must happen after additions and before removals so that there are no missing directory conflicts.
        printProgressBar(numCompleted, numOperations);
        std::cout << "Renaming " << p.first.string() << "\n";
        fs::rename(p.first, p.second);
        ++numCompleted;
    }
    
    for (auto setIter = changes.deletions.rbegin(); setIter != changes.deletions.rend(); ++setIter) {    // Iterate through deletions in reverse to avoid using recursive delete function.
        printProgressBar(numCompleted, numOperations);
        std::cout << "Removing " << setIter->string() << "\n";
        fs::remove(*setIter);
        ++numCompleted;
    }
    
    for (const auto& p : changes.modifications) {
        printProgressBar(numCompleted, numOperations);
        std::cout << "Replacing " << p.second.string() << "\n";
        fs::copy_file(p.first, p.second, fs::copy_options::overwrite_existing);
        ++numCompleted;
    }
    
    printProgressBar(numCompleted, numOperations);
    std::cout << "File operations completed.\n";
    
    if (!options.forceBackup) {
        BackupOptions options2 = options;
        options2.forceBackup = true;
        FileChanges changesAfter = checkBackup(configFilename, options2);
        if (!changesAfter.isEmpty()) {
            std::cout << CSI::Yellow << "Warning: Found remaining changes after running backup. This may have been caused by an error during\n";
            std::cout << "file operations or recursive rules in the config file. Run \"check <config file>\" for more details." << CSI::Reset << "\n";
        } else {
            std::cout << "Done.\n";
        }
    }
}

void Application::findCommonParentPath(std::string& lastPath, const std::string& currentPath, const std::string& currentRootPath) {
    std::string::size_type nextSeparator = currentPath.find(FileHandler::pathSeparator, lastPath.length());    // Find the separator in currentPath after the length of lastPath (may not be found).
    
    if (lastPath != currentPath.substr(0, nextSeparator)) {    // If the paths differ, need to update lastPath.
        size_t i = 0;
        while (true) {
            if (i >= lastPath.length() || i >= currentPath.length() || lastPath[i] != currentPath[i]) {    // When a different sub-path is found, step back to the last sub-path that both have in common and make this the new lastPath.
                std::string::size_type previousSeparator = currentPath.rfind(FileHandler::pathSeparator, i - 1);
                
                if (previousSeparator == std::string::npos || currentRootPath.length() > previousSeparator) {    // Edge case in case we're close to the root path.
                    lastPath = currentRootPath;
                } else {
                    lastPath = currentPath.substr(0, previousSeparator);
                }
                return;
            }
            ++i;
        }
    }
}

void Application::printTree(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, bool verbose, bool countOnly, bool pruneIgnored) {
    fs::file_status searchPathStatus = fs::status(searchPath);
    if (!fs::exists(searchPathStatus)) {
        throw std::runtime_error("\"" + searchPath.string() + "\": Unable to find path.");
    } else if (fs::is_directory(searchPathStatus)) {
        std::cout << CSI::Cyan << searchPath.string() << CSI::Reset << "\n";
        PrintTreeStats stats;
        printTree2(searchPath, readPathsMapping, verbose, !countOnly, pruneIgnored, "", &stats);
        
        std::cout << "\n" << stats.numDirectories << " directories, " << stats.numFiles << " files\n";
        std::cout << stats.numIgnoredDirectories << " ignored directories, " << stats.numIgnoredFiles << " ignored files\n";
    } else {
        throw std::runtime_error("\"" + searchPath.string() + "\": No sub-directories found.");
    }
}

void Application::printTree2(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, bool verbose, bool printOutput, bool pruneIgnored, const std::string& prefix, PrintTreeStats* stats) {
    std::vector<fs::directory_entry> searchContents;
    //std::priority_queue<fs::directory_entry, std::vector<fs::directory_entry>, decltype(&compareFilename)> searchContents(&compareFilename);    // Tested priority queue optimization, but turned out to be about 1.5 times slower.
    try {
        for (const auto& entry : fs::directory_iterator(searchPath)) {
            searchContents.emplace_back(entry);
        }
    } catch (fs::filesystem_error& ex) {
        if (printOutput) {
            std::cout << CSI::Red << "Error: " << ex.code().message() << ": \"" << ex.path1().string() << "\"";
            if (!ex.path2().empty()) {
                std::cout << ", \"" << ex.path2().string() << "\"";
            }
            std::cout << CSI::Reset << "\n";
        }
        return;
    } catch (std::exception& ex) {
        if (printOutput) {
            std::cout << prefix << CSI::Red << "Error: " << ex.what() << CSI::Reset << "\n";
        }
        return;
    }
    if (searchContents.empty()) {    // Current directory is empty.
        auto findResult = readPathsMapping.find(searchPath);
        if (verbose && printOutput && findResult != readPathsMapping.end()) {
            std::cout << prefix << " -> " << findResult->second.string() << "\n";
        }
        return;
    }
    
    std::sort(searchContents.begin(), searchContents.end(), compareFilename);
    
    if (pruneIgnored) {    // Determine if all children are ignored, and display ellipsis if so.
        bool allIgnored = true;
        for (size_t i = 0; i < searchContents.size(); ++i) {
            if (readPathsMapping.find(searchContents[i].path()) != readPathsMapping.end()) {
                allIgnored = false;
                break;
            }
        }
        if (allIgnored && printOutput) {
            std::cout << prefix << "\'-- " << CSI::Yellow << "(...)" << CSI::Reset << "\n";
            printOutput = false;
        }
    }
    for (size_t i = 0; i < searchContents.size(); ++i) {
        if (printOutput) {
            std::cout << prefix;
        }
        auto findResult = readPathsMapping.find(searchContents[i].path());
        const bool isTracked = (findResult != readPathsMapping.end());
        const bool isLast = (i + 1 == searchContents.size());
        
        if (searchContents[i].is_directory()) {
            ++stats->numDirectories;
            if (printOutput) {
                std::cout << (isLast ? "\'-- " : "|-- ") << (isTracked ? CSI::Cyan : CSI::Yellow) << searchContents[i].path().filename().string() << CSI::Reset << "\n";
            }
            if (!isTracked) {
                ++stats->numIgnoredDirectories;
            }
            printTree2(searchContents[i].path(), readPathsMapping, verbose, printOutput, pruneIgnored, prefix + (isLast ? "    " : "|   "), stats);
        } else {
            ++stats->numFiles;
            if (printOutput) {
                std::cout << (isLast ? "\'-- " : "|-- ") << (isTracked ? CSI::Green : CSI::Yellow) << searchContents[i].path().filename().string() << CSI::Reset << "\n";
            }
            if (isTracked) {
                if (verbose && printOutput) {
                    std::cout << prefix << (isLast ? "    " : "|   ") << " -> " << findResult->second.string() << "\n";
                }
            } else {
                ++stats->numIgnoredFiles;
            }
        }
    }
}

/**
 * Due to the way rename detection works, only files can make their way out of
 * additions/deletions and into the renames bin. Directories are always either
 * added or deleted. Rename detection also does not consider which mount point a
 * file is located on, so a file can be marked as "renamed" when it needs to
 * travel across a drive partition (even from a FAT32 to NTFS format). In
 * practice this works fine as the fs::rename() operation is designed to handle
 * this.
 */
void Application::optimizeForRenames(FileHandler& fileHandler, FileChanges& changes, bool skipCache, bool fastCompare) {
    std::map<std::uintmax_t, std::set<fs::path>> deletionsFileSizes;    // Map file sizes to their paths for quick lookup of which files match the contents of a path.
    for (const auto& p : changes.deletions) {
        if (fs::is_regular_file(p)) {
            deletionsFileSizes[fs::file_size(p)].emplace(p);
        }
    }
    
    for (auto additionsIter = changes.additions.begin(); additionsIter != changes.additions.end();) {
        bool stepNextAddition = true;
        if (fs::is_regular_file(additionsIter->first)) {
            auto findResult = deletionsFileSizes.find(fs::file_size(additionsIter->first));
            if (findResult != deletionsFileSizes.end()) {    // If file matches size of one of the deleted ones, check if contents match.
                for (auto deletionsSetIter = findResult->second.begin(); deletionsSetIter != findResult->second.end(); ++deletionsSetIter) {
                    if (fileHandler.checkFileEquivalence(additionsIter->first, *deletionsSetIter, skipCache, fastCompare)) {
                        changes.renames.emplace(*deletionsSetIter, additionsIter->second);    // Found a match, add it as a rename and remove the corresponding addition and deletion (subdirectories are not touched because fs::rename() expects existing directories).
                        auto deletionsIter = changes.deletions.find(*deletionsSetIter);
                        changes.deletions.erase(deletionsIter);
                        findResult->second.erase(deletionsSetIter);
                        additionsIter = changes.additions.erase(additionsIter);
                        stepNextAddition = false;
                        break;
                    }
                }
            }
        }
        
        if (stepNextAddition) {    // Prevent increment if changes.additions was modified.
            ++additionsIter;
        }
    }
}

void Application::printSpinner(int& index, std::chrono::steady_clock::time_point& lastTime) {
    constexpr char spinnerStr[] = "|/-\\";
    std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
    
    if (std::chrono::duration_cast<std::chrono::milliseconds>(currTime - lastTime).count() > 200) {
        index = (index + 1) % 4;
        std::cout << spinnerStr[index];
        std::cout << "\033[1D";    // Move cursor back 1.
        lastTime = currTime;
    }
}

void Application::printProgressBar(size_t current, size_t total) {
    constexpr int MAX_BAR_SIZE = 80;
    
    std::string percent = std::to_string(std::lround(static_cast<double>(current) / total * 100.0)) + "%";
    int barLength = std::lround(static_cast<double>(current) / total * MAX_BAR_SIZE);
    std::cout << "\033[2A";    // Move cursor up by 2.
    std::cout << std::left << std::setw(4) << percent << std::right << '[' << std::setfill('=') << std::setw(barLength) << '>' << std::setfill(' ') << std::setw(MAX_BAR_SIZE - barLength + (barLength > 0 ? 1 : 0)) << ']' << '\n';
    std::cout << "\033[2K";    // Clear line.
    // The message line should be outputted next followed by a newline.
}
