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
            inputCleaned.push_back(std::tolower(c));
        }
    }
    if (inputCleaned == "y" || inputCleaned == "yee" || inputCleaned == "yes" || inputCleaned == "yeah") {
        return true;
    }
    return false;
}

void Application::printPaths(const fs::path& configFilename) {
    std::cout << "printPaths():\n";
    fileHandler_.loadConfigFile(configFilename);
    for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
        std::cout << "[" << nextPath.writePath << "] ->\n";
        std::cout << "    [" << nextPath.readAbsolute << "]   [" << nextPath.readLocal << "]\n";
    }
}

std::pair<bool, std::vector<fs::path>> Application::checkBackup(const fs::path& configFilename, bool displayConfirmation) {
    std::vector<fs::path> pathDeletions, pathAdditions, pathModifications;
    std::map<fs::path, std::set<fs::path>> writePathsChecklist;
    auto lastWritePathIter = writePathsChecklist.end();
    std::cout << "checkBackup():\n";
    fileHandler_.loadConfigFile(configFilename);
    
    for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
        if (lastWritePathIter == writePathsChecklist.end() || lastWritePathIter->first != nextPath.writePath) {
            auto insertResult = writePathsChecklist.insert({nextPath.writePath, {}});
            if (insertResult.second) {
                for (const auto& entry : fs::recursive_directory_iterator(nextPath.writePath)) {
                    insertResult.first->second.insert(entry.path());
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
    
    std::cout << "startBackup():\n";
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
