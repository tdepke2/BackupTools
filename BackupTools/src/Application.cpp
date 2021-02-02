#include "Application.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <set>

std::ostream& operator<<(std::ostream& out, CSI csiCode) {
    return out << '\033' << '[' << static_cast<int>(csiCode) << 'm';
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
    
    printTree(longestParentPath, readPathsMapping);
}

std::pair<bool, std::vector<fs::path>> Application::checkBackup(const fs::path& configFilename, bool displayConfirmation) {
    std::vector<fs::path> pathDeletions, pathAdditions, pathModifications;
    std::map<fs::path, std::set<fs::path>> writePathsChecklist;
    auto lastWritePathIter = writePathsChecklist.end();
    fileHandler_.loadConfigFile(configFilename);
    
    for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
        if (lastWritePathIter == writePathsChecklist.end() || lastWritePathIter->first != nextPath.writePath) {
            auto insertResult = writePathsChecklist.emplace(nextPath.writePath, std::set<fs::path>());
            if (insertResult.second) {
                for (const auto& entry : fs::recursive_directory_iterator(nextPath.writePath)) {
                    insertResult.first->second.emplace(entry.path());
                }
            }
            
            lastWritePathIter = insertResult.first;
        }
        
        fs::path destinationPath = nextPath.writePath / nextPath.readLocal;
        if (lastWritePathIter->second.erase(destinationPath) == 0) {
            pathAdditions.push_back(destinationPath);
        } else if (!FileHandler::checkFileEquivalence(nextPath.readAbsolute, destinationPath)) {
            pathModifications.push_back(destinationPath);
        }
    }
    
    for (const auto& writePath : writePathsChecklist) {
        for (const auto& p : writePath.second) {
            pathDeletions.push_back(p);
        }
    }
    
    if (pathDeletions.empty() && pathAdditions.empty() && pathModifications.empty()) {
        std::cout << "All up to date.\n";
        return {false, pathDeletions};
    }
    
    if (!pathDeletions.empty()) {
        std::sort(pathDeletions.begin(), pathDeletions.end());
        std::cout << "Deletions:\n" << CSI::Red;
        for (const auto& p : pathDeletions) {
            std::cout << "  " << p << "\n";
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!pathAdditions.empty()) {
        std::sort(pathAdditions.begin(), pathAdditions.end());
        std::cout << "Additions:\n" << CSI::Green;
        for (const auto& p : pathAdditions) {
            std::cout << "  " << p << "\n";
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (!pathModifications.empty()) {
        std::sort(pathModifications.begin(), pathModifications.end());
        std::cout << "\nModifications:\n" << CSI::Yellow;
        for (const auto& p : pathModifications) {
            std::cout << "  " << p << "\n";
        }
        std::cout << CSI::Reset << "\n";
    }
    
    if (displayConfirmation) {
        std::cout << "After this operation, " << pathDeletions.size() << " item(s) will be removed, " << pathAdditions.size() << " item(s) will be added, and " << pathModifications.size() << " item(s) will be modified.\n";
        std::cout << "Are you sure? [Y/N] ";
    }
    return {true, pathDeletions};
}

void Application::startBackup(const fs::path& configFilename, bool forceBackup) {
    auto checkBackupResult = checkBackup(configFilename, !forceBackup);
    if (!checkBackupResult.first) {
        return;
    }
    if (!forceBackup) {
        if (!checkUserConfirmation()) {
            std::cout << "Backup canceled.\n";
            return;
        }
    }
    
    for (size_t i = checkBackupResult.second.size(); i > 0;) {
        --i;
        std::cout << "Removing " << checkBackupResult.second[i] << "\n";
        fs::remove(checkBackupResult.second[i]);
    }
    
    fileHandler_.loadConfigFile(configFilename);
    for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
        if (!FileHandler::checkFileEquivalence(nextPath.readAbsolute, nextPath.writePath / nextPath.readLocal)) {
            std::cout << "Replacing " << nextPath.writePath / nextPath.readLocal << ".\n";
            fs::copy(nextPath.readAbsolute, nextPath.writePath / nextPath.readLocal, fs::copy_options::overwrite_existing);
        }
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
    std::sort(searchContents.begin(), searchContents.end(), CompareFilename());
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
