#include "Application.h"
#include "FileHandler.h"
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

// Quick desc:
// CLI backup tool ideal for backing up files to a flash drive. Target filename to include/exclude are stored in a config file.
// Supports diff checking, manual backup, and automatic backups. Ideally should work on linux too, and use CMake and TDD.

namespace fs = std::filesystem;

void showHelp() {
    std::cout << "Options:\n";
    std::cout << "  backup <config file> [--limit n]    Starts a backup/restore of files.\n";
    std::cout << "    --limit n                           Limits output to n lines (50 by default). Use -1 for no limit.\n";
    std::cout << "    --force                             Forces backup to run without confirmation check.\n";
    std::cout << "  check <config file> [--limit n]     Lists changes to make during backup.\n";
    std::cout << "    --limit n                           Limits output to n lines (50 by default). Use -1 for no limit.\n";
    std::cout << "  tree <config file> [--count]        Displays tree of tracked files.\n";
    std::cout << "    --verbose                           Show tracked file destinations.\n";
    std::cout << "    --count                             Only display the total count.\n";
    std::cout << "  help                                Shows this menu.\n";
    std::cout << "  exit                                Exits interactive shell.\n";
}

int main(int argc, char** argv) {
    showHelp();
    while (true) {
        try {
            std::cout << "\n>>> ";
            std::string line;
            std::getline(std::cin, line);
            
            std::string::size_type index = 0;
            FileHandler::skipWhitespace(index, line);
            if (index >= line.length()) {
                continue;
            }
            std::string command = FileHandler::parseNextWord(index, line);
            if (command == "backup") {
                if (index >= line.length()) {
                    throw std::runtime_error("Missing path to config file.");
                }
                fs::path configFilename = FileHandler::parseNextPath(index, line);
                
                unsigned int outputLimit = 50;    // FIXME: May want to rework this later for less duplicated code. ################################################
                bool forceBackup = false;
                while (index < line.length()) {
                    command = FileHandler::parseNextWord(index, line);
                    if (command == "--limit") {
                        if (index >= line.length()) {
                            throw std::runtime_error("Missing value for \"--limit\".");
                        }
                        try {
                            int n = FileHandler::parseNextInt(index, line);
                            if (n < 0) {
                                outputLimit = std::numeric_limits<unsigned int>::max();
                            } else {
                                outputLimit = static_cast<unsigned int>(n);
                            }
                        } catch (...) {
                            throw std::runtime_error("Value for \"--limit\" must be integer.");
                        }
                    } else if (command == "--force") {
                        forceBackup = true;
                    } else {
                        throw std::runtime_error("Invalid parameter \"" + command + "\".");
                    }
                }
                
                Application app;
                app.startBackup(configFilename, outputLimit, forceBackup);
            } else if (command == "check") {
                if (index >= line.length()) {
                    throw std::runtime_error("Missing path to config file.");
                }
                fs::path configFilename = FileHandler::parseNextPath(index, line);
                
                unsigned int outputLimit = 50;
                while (index < line.length()) {
                    command = FileHandler::parseNextWord(index, line);
                    if (command == "--limit") {
                        if (index >= line.length()) {
                            throw std::runtime_error("Missing value for \"--limit\".");
                        }
                        try {
                            int n = FileHandler::parseNextInt(index, line);
                            if (n < 0) {
                                outputLimit = std::numeric_limits<unsigned int>::max();
                            } else {
                                outputLimit = static_cast<unsigned int>(n);
                            }
                        } catch (...) {
                            throw std::runtime_error("Value for \"--limit\" must be integer.");
                        }
                    } else {
                        throw std::runtime_error("Invalid parameter \"" + command + "\".");
                    }
                }
                
                Application app;
                app.checkBackup(configFilename, outputLimit);
            } else if (command == "tree") {
                if (index >= line.length()) {
                    throw std::runtime_error("Missing path to config file.");
                }
                fs::path configFilename = FileHandler::parseNextPath(index, line);
                
                bool verbose = false;
                bool countOnly = false;
                while (index < line.length()) {
                    command = FileHandler::parseNextWord(index, line);
                    if (command == "--verbose") {
                        verbose = true;
                    } else if (command == "--count") {
                        countOnly = true;
                    } else {
                        throw std::runtime_error("Invalid parameter \"" + command + "\".");
                    }
                }
                
                Application app;
                app.printPaths(configFilename, verbose, countOnly);
            } else if (command == "help") {
                showHelp();
            } else if (command == "exit") {
                break;
            } else {
                throw std::runtime_error("Unknown command \"" + command + "\". Type \"help\" for command list.");
            }
        } catch (fs::filesystem_error& ex) {
            std::cout << CSI::Red << "Error: " << ex.code().message() << ": \"" << ex.path1().string() << "\"";
            if (!ex.path2().empty()) {
                std::cout << ", \"" << ex.path2().string() << "\"";
            }
            std::cout << CSI::Reset << "\n";
        } catch (std::exception& ex) {
            std::cout << CSI::Red << "Error: " << ex.what() << CSI::Reset << "\n";
        }
    }
    
    return 0;
}
