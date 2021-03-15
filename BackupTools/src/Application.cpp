#include "Application.h"
#include <algorithm>
#include <cassert>
#include <cctype>
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

void Application::printPaths(const fs::path& configFilename, const bool countOnly) {
    std::map<fs::path, fs::path> readPathsMapping;    // Maps read path to corresponding write path.
    std::map<fs::path, std::string> longestParentPaths;    // Longest common path among nextPath.readAbsolute entries (per root path).
    FileHandler fileHandler;
    fileHandler.loadConfigFile(configFilename);
    
    WriteReadPath nextPath = fileHandler.getNextWriteReadPath();
    if (nextPath.isEmpty()) {
        std::cout << "No files or directories found to track.\n";
        return;
    }
    
    while (!nextPath.isEmpty()) {
        if (!readPathsMapping.emplace(nextPath.readAbsolute, nextPath.writePath / nextPath.readLocal).second) {
            std::cout << CSI::Yellow << "Warning: Skipping duplicate read path: " << nextPath.readAbsolute.string() << CSI::Reset << "\n";
        }
        auto findResult = longestParentPaths.find(nextPath.readAbsolute.root_path());
        if (findResult == longestParentPaths.end()) {    // New root encountered, add entry for it.
            findResult = longestParentPaths.emplace(nextPath.readAbsolute.root_path(), nextPath.readAbsolute.string()).first;
        }
        std::string& longestParentPath = findResult->second;
        if (longestParentPath != nextPath.readAbsolute.string().substr(0, longestParentPath.size())) {    // Update longestParentPath.
            std::string currentParentPath = nextPath.readAbsolute.string();
            for (size_t i = 0; i < longestParentPath.size(); ++i) {
                if (i >= currentParentPath.size()) {
                    longestParentPath = currentParentPath;
                    break;
                } else if (longestParentPath[i] != currentParentPath[i]) {
                    longestParentPath = currentParentPath.substr(0, i);
                }
            }
        }
        
        nextPath = fileHandler.getNextWriteReadPath();
    }
    
    for (auto mapIter = longestParentPaths.begin(); mapIter != longestParentPaths.end(); ++mapIter) {
        if (mapIter != longestParentPaths.begin()) {
            std::cout << "\n";
        }
        if (fs::is_regular_file(mapIter->second) && mapIter->second.rfind(FileHandler::pathSeparator) != std::string::npos) {    // If parent path is a file, cut off the file portion before call to printTree().
            printTree(mapIter->second.substr(0, mapIter->second.rfind(FileHandler::pathSeparator) + 1), readPathsMapping, countOnly);
        } else {
            printTree(mapIter->second, readPathsMapping, countOnly);
        }
    }
}

Application::FileChanges Application::checkBackup(const fs::path& configFilename, size_t outputLimit, bool displayConfirmation, bool silent) {
    FileChanges changes;
    std::map<fs::path, std::set<fs::path>> writePathsChecklist;    // Maps a destination path to the current contents of that path.
    auto lastWritePathIter = writePathsChecklist.end();
    FileHandler fileHandler;
    fileHandler.loadConfigFile(configFilename);
    
    for (WriteReadPath nextPath = fileHandler.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler.getNextWriteReadPath()) {
        if (lastWritePathIter == writePathsChecklist.end() || lastWritePathIter->first != nextPath.writePath) {    // Check if the write path changed and add directory contents if it is a new path.
            auto insertResult = writePathsChecklist.emplace(nextPath.writePath, std::set<fs::path>());
            if (insertResult.second) {
                for (const auto& entry : fs::recursive_directory_iterator(nextPath.writePath)) {    // If exception during iteration of write path, checkBackup() must stop.
                    insertResult.first->second.emplace(entry.path());
                }
            }
            
            lastWritePathIter = insertResult.first;
        }
        
        fs::path destinationPath = nextPath.writePath / nextPath.readLocal;
        if (lastWritePathIter->second.erase(destinationPath) == 0) {    // Attempt to remove the destination path from the checklist. If it's not found, then it doesn't currently exist and needs to be added.
            auto emplaceResult = changes.additions.emplace(nextPath.readAbsolute, destinationPath);
            assert(emplaceResult.second);
        } else if (!FileHandler::checkFileEquivalence(nextPath.readAbsolute, destinationPath)) {    // If file exists but contents differ, it needs to be updated.
            auto emplaceResult = changes.modifications.emplace(nextPath.readAbsolute, destinationPath);
            assert(emplaceResult.second);
        }
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
    }
    writePathsChecklist.clear();
    
    optimizeForRenames(changes);
    
    if (silent) {
        return changes;
    } else if (changes.isEmpty()) {
        std::cout << "All up to date.\n";
        return changes;
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
        std::cout << "\nAre you sure? [Y/N] ";
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
    return changes;
}

void Application::startBackup(const fs::path& configFilename, size_t outputLimit, bool forceBackup) {
    FileChanges changes = checkBackup(configFilename, outputLimit, true, forceBackup);
    if (changes.isEmpty()) {
        return;
    }
    if (!forceBackup && !checkUserConfirmation()) {
        std::cout << "Backup canceled.\n";
        return;
    }
    
    for (const auto& p : changes.additions) {
        std::cout << "Adding " << p.second.string() << "\n";
        if (fs::is_directory(p.first)) {
            fs::create_directory(p.second);
        } else {
            fs::copy(p.first, p.second);
        }
    }
    
    for (const auto& p : changes.renames) {    // Renaming must happen after additions and before removals so that there are no missing directory conflicts.
        std::cout << "Renaming " << p.first.string() << "\n";
        fs::rename(p.first, p.second);
    }
    
    for (auto setIter = changes.deletions.rbegin(); setIter != changes.deletions.rend(); ++setIter) {    // Iterate through deletions in reverse to avoid using recursive delete function.
        std::cout << "Removing " << setIter->string() << "\n";
        fs::remove(*setIter);
    }
    
    for (const auto& p : changes.modifications) {
        std::cout << "Replacing " << p.second.string() << "\n";
        fs::copy(p.first, p.second, fs::copy_options::overwrite_existing);
    }
    
    if (!forceBackup) {
        FileChanges changesAfter = checkBackup(configFilename, outputLimit, true, true);
        if (!changesAfter.isEmpty()) {
            std::cout << "\n" << CSI::Yellow << "Warning: Found remaining changes after running backup. This may have been caused by an error during\n";
            std::cout << "file operations or recursive rules in the config file. Run \"check <config file>\" for more details." << CSI::Reset << "\n";
        }
    }
}

void Application::restoreFromBackup(const fs::path& configFilename) {
    
}

void Application::printTree(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, const bool countOnly) {
    fs::file_status searchPathStatus = fs::status(searchPath);
    if (!fs::exists(searchPathStatus)) {
        throw std::runtime_error("\"" + searchPath.string() + "\": Unable to find path.");
    } else if (fs::is_directory(searchPathStatus)) {
        std::cout << CSI::Cyan << searchPath.string() << CSI::Reset << "\n";
        PrintTreeStats stats;
        printTree2(searchPath, readPathsMapping, !countOnly, "", &stats);
        
        std::cout << "\n" << stats.numDirectories << " directories, " << stats.numFiles << " files\n";
        std::cout << stats.numIgnoredDirectories << " ignored directories, " << stats.numIgnoredFiles << " ignored files\n";
    } else {
        throw std::runtime_error("\"" + searchPath.string() + "\": No sub-directories found.");
    }
}

void Application::printTree2(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, const bool printOutput, const std::string& prefix, PrintTreeStats* stats) {
    std::vector<fs::directory_entry> searchContents;
    //std::priority_queue<fs::directory_entry, std::vector<fs::directory_entry>, decltype(&compareFilename)> searchContents(&compareFilename);    // Tested priority queue optimization, but turned out to be about 1.5 times slower.
    try {
        for (const auto& entry : fs::directory_iterator(searchPath)) {
            searchContents.emplace_back(entry);
        }
    } catch (std::exception& ex) {
        if (printOutput) {
            std::cout << prefix << CSI::Red << "Error: " << ex.what() << CSI::Reset << "\n";
        }
        return;
    }
    if (searchContents.empty()) {
        auto findResult = readPathsMapping.find(searchPath);
        if (printOutput && findResult != readPathsMapping.end()) {
            std::cout << prefix << " -> " << findResult->second.string() << "\n";
        }
        return;
    }
    
    std::sort(searchContents.begin(), searchContents.end(), compareFilename);
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
            printTree2(searchContents[i].path(), readPathsMapping, printOutput, prefix + (isLast ? "    " : "|   "), stats);
        } else {
            ++stats->numFiles;
            if (printOutput) {
                std::cout << (isLast ? "\'-- " : "|-- ") << (isTracked ? CSI::Green : CSI::Yellow) << searchContents[i].path().filename().string() << CSI::Reset << "\n";
            }
            if (isTracked) {
                if (printOutput) {
                    std::cout << prefix << (isLast ? "    " : "|   ") << " -> " << findResult->second.string() << "\n";
                }
            } else {
                ++stats->numIgnoredFiles;
            }
        }
    }
}

void Application::optimizeForRenames(FileChanges& changes) {
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
                    if (FileHandler::checkFileEquivalence(additionsIter->first, *deletionsSetIter)) {
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
