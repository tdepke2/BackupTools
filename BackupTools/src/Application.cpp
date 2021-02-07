#include "Application.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>

std::ostream& operator<<(std::ostream& out, CSI csiCode) {
    return out << '\033' << '[' << static_cast<int>(csiCode) << 'm';
}

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
    if (inputCleaned == "y" || inputCleaned == "yee" || inputCleaned == "yes" || inputCleaned == "yeah") {
        return true;
    }
    return false;
}

void Application::printPaths(const fs::path& configFilename) {
    std::map<fs::path, fs::path> readPathsMapping;
    std::string longestParentPath;
    std::set<fs::path> rootPaths;
    fileHandler_.loadConfigFile(configFilename);
    
    WriteReadPath nextPath = fileHandler_.getNextWriteReadPath();
    if (nextPath.isEmpty()) {
        std::cout << "No files or directories found to track.\n";
        return;
    }
    longestParentPath = nextPath.readAbsolute.parent_path().string();
    rootPaths.emplace(nextPath.readAbsolute.root_path());
    while (!nextPath.isEmpty()) {
        if (!readPathsMapping.emplace(nextPath.readAbsolute, nextPath.writePath / nextPath.readLocal).second) {
            std::cout << CSI::Yellow << "Warn: Skipping duplicate read path: " << nextPath.readAbsolute.string() << CSI::Reset << "\n";
        }
        if (longestParentPath != nextPath.readAbsolute.parent_path().string().substr(0, longestParentPath.size())) {    // Update longestParentPath.
            std::string currentParentPath = nextPath.readAbsolute.parent_path().string();
            for (size_t i = 0; i < longestParentPath.size(); ++i) {
                if (i >= currentParentPath.size()) {
                    longestParentPath = currentParentPath;
                    break;
                } else if (longestParentPath[i] != currentParentPath[i]) {
                    longestParentPath = currentParentPath.substr(0, i);
                }
            }
        }
        rootPaths.emplace(nextPath.readAbsolute.root_path());
        
        nextPath = fileHandler_.getNextWriteReadPath();
    }
    
    // Need to modify longestParentPath so that it is a std::map<fs::path, fs::path> for longestParentPath per root directory, and remove rootPaths. ########################################################################
    printTree(longestParentPath, readPathsMapping);
}

Application::FileChanges Application::checkBackup(const fs::path& configFilename, bool displayConfirmation) {
    FileChanges changes;
    std::map<fs::path, std::set<fs::path>> writePathsChecklist;    // Maps a destination path to the current contents of that path.
    auto lastWritePathIter = writePathsChecklist.end();
    fileHandler_.loadConfigFile(configFilename);
    
    for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
        if (lastWritePathIter == writePathsChecklist.end() || lastWritePathIter->first != nextPath.writePath) {    // Check if the write path changed and add directory contents if it is a new path.
            auto insertResult = writePathsChecklist.emplace(nextPath.writePath, std::set<fs::path>());
            if (insertResult.second) {
                for (const auto& entry : fs::recursive_directory_iterator(nextPath.writePath)) {
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
    
    for (const auto& writePath : writePathsChecklist) {    // Any remaining paths in the checklist do not belong, mark these for deletion.
        for (const auto& p : writePath.second) {
            auto emplaceResult = changes.deletions.emplace(p);
            assert(emplaceResult.second);
        }
    }
    writePathsChecklist.clear();
    
    if (changes.isEmpty()) {
        std::cout << "All up to date.\n";
        return changes;
    }
    
    optimizeForRenames(changes);
    
    if (!changes.deletions.empty()) {
        std::cout << "Deletions:\n" << CSI::Red;
        for (const auto& p : changes.deletions) {
            std::cout << "-   " << p.string() << "\n";
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!changes.additions.empty()) {
        std::cout << "Additions:\n" << CSI::Green;
        for (const auto& p : changes.additions) {
            std::cout << "+   " << p.second.string() << "\n";
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!changes.modifications.empty()) {
        std::cout << "Modifications:\n" << CSI::Yellow;
        for (const auto& p : changes.modifications) {
            std::cout << "*   " << p.second.string() << "\n";
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!changes.renames.empty()) {
        std::cout << "Renames:\n" << CSI::Magenta;
        for (const auto& p : changes.renames) {
            std::cout << "~   " << p.first.string() << " -> " << p.second.string() << "\n";
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
    }
    return changes;
}

void Application::startBackup(const fs::path& configFilename, bool forceBackup) {
    FileChanges changes = checkBackup(configFilename, !forceBackup);
    if (changes.isEmpty()) {
        return;
    }
    if (!forceBackup) {
        if (!checkUserConfirmation()) {
            std::cout << "Backup canceled.\n";
            return;
        }
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
}

void Application::restoreFromBackup(const fs::path& configFilename) {
    
}

void Application::printTree(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping) {
    fs::file_status searchPathStatus = fs::status(searchPath);
    if (!fs::exists(searchPathStatus)) {
        throw std::runtime_error("\"" + searchPath.string() + "\": Unable to find path.");
    } else if (fs::is_directory(searchPathStatus)) {
        std::cout << CSI::Cyan << searchPath.string() << CSI::Reset << "\n";
        unsigned int numDirectories = 0, numFiles = 0;
        printTree2(searchPath, readPathsMapping, "", &numDirectories, &numFiles);
        
        std::cout << "\n" << numDirectories << " directories, " << numFiles << " files\n";
    } else {
        throw std::runtime_error("\"" + searchPath.string() + "\": No sub-directories found.");
    }
}

void Application::printTree2(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, const std::string& prefix, unsigned int* numDirectories, unsigned int* numFiles) {
    std::vector<fs::directory_entry> searchContents;
    try {
        for (const auto& entry : fs::directory_iterator(searchPath)) {
            searchContents.emplace_back(entry);
        }
    } catch (std::exception& ex) {
        std::cout << prefix << CSI::Red << "Error: " << ex.what() << CSI::Reset << "\n";
        return;
    }
    if (searchContents.empty()) {
        auto findResult = readPathsMapping.find(searchPath);
        if (findResult != readPathsMapping.end()) {
            std::cout << prefix << " -> " << findResult->second.string() << "\n";
        }
        return;
    }
    std::sort(searchContents.begin(), searchContents.end(), compareFilename);
    for (size_t i = 0; i < searchContents.size(); ++i) {
        std::cout << prefix;
        if (searchContents[i].is_directory()) {
            ++(*numDirectories);
            if (i + 1 != searchContents.size()) {
                std::cout << "|-- " << CSI::Cyan << searchContents[i].path().filename().string() << CSI::Reset << "\n";
                printTree2(searchContents[i].path(), readPathsMapping, prefix + "|   ", numDirectories, numFiles);
            } else {
                std::cout << "\'-- " << CSI::Cyan << searchContents[i].path().filename().string() << CSI::Reset << "\n";
                printTree2(searchContents[i].path(), readPathsMapping, prefix + "    ", numDirectories, numFiles);
            }
        } else {
            ++(*numFiles);
            auto findResult = readPathsMapping.find(searchContents[i].path());
            if (findResult != readPathsMapping.end()) {
                std::cout << (i + 1 != searchContents.size() ? "|-- " : "\'-- ") << CSI::Green << searchContents[i].path().filename().string() << CSI::Reset << "\n";
                std::cout << prefix << (i + 1 != searchContents.size() ? "|   " : "    ") << " -> " << findResult->second.string() << "\n";
            } else {
                std::cout << (i + 1 != searchContents.size() ? "|-- " : "\'-- ") << CSI::Yellow << searchContents[i].path().filename().string() << CSI::Reset << "\n";
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
                for (const fs::path& deletionPath : findResult->second) {
                    if (FileHandler::checkFileEquivalence(additionsIter->first, deletionPath)) {
                        changes.renames.emplace(deletionPath, additionsIter->second);    // Found a match, add it as a rename and remove the corresponding addition and delete (subdirectories are not touched because fs::rename() expects existing directories).
                        changes.deletions.erase(changes.deletions.find(deletionPath));
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
