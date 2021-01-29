#include "Application.h"
#include <cctype>
#include <iostream>
#include <map>
#include <set>

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
    try {
        std::cout << "printPaths():\n";
        fileHandler_.loadConfigFile(configFilename);
        for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
            std::cout << "[" << nextPath.writePath << "] ->\n";
            std::cout << "    [" << nextPath.readAbsolute << "]   [" << nextPath.readLocal << "]\n";
        }
    } catch (std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
}

std::vector<fs::path> Application::checkBackup(const fs::path& configFilename, bool displayConfirmation) {
    std::vector<fs::path> pathDeletions, pathAdditions, pathModifications;
    try {
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
        
        std::cout << "Deletions:\n";
        for (const auto& p : pathDeletions) {
            std::cout << "  " << p << "\n";
        }
        
        std::cout << "\nAdditions:\n";
        for (const auto& p : pathAdditions) {
            std::cout << "  " << p << "\n";
        }
        
        std::cout << "\nModifications:\n";
        for (const auto& p : pathModifications) {
            std::cout << "  " << p << "\n";
        }
        
        if (displayConfirmation) {
            std::cout << "\nAfter this operation, " << pathDeletions.size() << " items will be removed, " << pathAdditions.size() << " items will be added, and " << pathModifications.size() << " items will be modified.\n";
            std::cout << "Are you sure? [Y/N] ";
        }
    } catch (std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return pathDeletions;
}

void Application::startBackup(const fs::path& configFilename, bool forceBackup) {
    try {
        std::vector<fs::path> pathDeletions = checkBackup(configFilename, !forceBackup);
        if (!forceBackup) {
            if (!checkUserConfirmation()) {
                std::cout << "Operation cancelled.\n";
                return;
            }
        }
        
        std::cout << "startBackup():\n";
        for (const auto& p : pathDeletions) {
            std::cout << "Removing " << p << "\n";
            fs::remove(p);
        }
        
        fileHandler_.loadConfigFile(configFilename);
        for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
            if (!FileHandler::checkFileEquivalence(nextPath.readAbsolute, nextPath.writePath / nextPath.readLocal)) {
                std::cout << "Replacing " << nextPath.writePath / nextPath.readLocal << ".\n";
                fs::copy(nextPath.readAbsolute, nextPath.writePath / nextPath.readLocal, fs::copy_options::overwrite_existing);
            }
        }
    } catch (std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
}

void Application::restoreFromBackup(const fs::path& configFilename) {
    
}
