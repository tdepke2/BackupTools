#include "Application.h"
#include "FileHandler.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

// Quick desc:
// CLI backup tool ideal for backing up files to a flash drive. Target filename to include/exclude are stored in a config file.
// Supports diff checking, manual backup, and automatic backups. Ideally should work on linux too, and use CMake and TDD.

namespace fs = std::filesystem;

void showHelp() {
    std::cout << "Options:\n";
    std::cout << "  backup <config file>     Starts a backup.\n";
    std::cout << "  check <config file>      Checks backup status.\n";
    std::cout << "  tree <config file>       Displays tree of tracked files.\n";
    std::cout << "  restore <config file>    Restores a backup.\n";
    std::cout << "  help                     Shows this menu.\n";
    std::cout << "  exit                     Exits interactive shell.\n";
}

int main(int argc, char** argv) {
    Application app;
    
    showHelp();
    std::string line;
    while (true) {
        try {
            std::cout << "\n>>> ";
            std::getline(std::cin, line);
            
            std::string::size_type index = 0;
            FileHandler::skipWhitespace(index, line);
            if (index >= line.length()) {
                continue;
            }
            std::string command = FileHandler::parseNextWord(index, line);
            FileHandler::skipWhitespace(index, line);
            if (command == "backup") {
                if (index >= line.length()) {
                    throw std::runtime_error("Missing path to config file.");
                }
                fs::path configFilename = FileHandler::parseNextPath(index, line);
                
                app.startBackup(configFilename, false);
            } else if (command == "check") {
                if (index >= line.length()) {
                    throw std::runtime_error("Missing path to config file.");
                }
                fs::path configFilename = FileHandler::parseNextPath(index, line);
                
                app.checkBackup(configFilename);
            } else if (command == "tree") {
                
            } else if (command == "restore") {
                
            } else if (command == "help") {
                showHelp();
            } else if (command == "exit") {
                break;
            } else {
                throw std::runtime_error("Unknown command \"" + command + "\". Type \"help\" for command list.");
            }
            FileHandler::skipWhitespace(index, line);
            if (index < line.length()) {
                throw std::runtime_error("Invalid parameter(s) \"" + line.substr(index) + "\".");
            }
        } catch (std::exception& ex) {
            std::cout << CSI::Red << "Error: " << ex.what() << CSI::Reset << "\n";
        }
    }
    
    /*app.printPaths("sample.txt");
    std::cout << "\n";
    app.startBackup("sample.txt", false);
    std::cout << "\n";*/
    
    return 0;
}
