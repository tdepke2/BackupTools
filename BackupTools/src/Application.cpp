#include "Application.h"
#include <iostream>

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

void Application::checkBackup(const fs::path& configFilename) {
    try {
        std::cout << "checkBackup():\n";
        fileHandler_.loadConfigFile(configFilename);
        for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
            // store a map<path, set<path>> to keep track of write locations. when a new write path added to map, use recursive iterator to add it's paths to the set. cross off these paths as we go.
            
            
        }
    } catch (std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
}

/*
ok:
C:/TD/Docs/source/a.txt -> C:/TD/Docs/dest/a.txt

bad:
C:/TD/Docs/source/a.txt -> C:/TD/Docs/source/a.txt
C:/TD/Docs/source/a.txt -> C:/TD/Docs/source/dest/a.txt
C:/TD/Docs/source/dest/a.txt -> C:/TD/Docs/source/a.txt

*/

void Application::startBackup(const fs::path& configFilename) {
    try {
        std::cout << "startBackup():\n";
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
