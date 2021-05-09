#include "Application.h"
#include "ArgumentParser.h"
#include "FileHandler.h"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// Quick desc:
// CLI backup tool ideal for backing up files to a flash drive. Target filename to include/exclude are stored in a config file.
// Supports diff checking, manual backup, and automatic backups. Ideally should work on linux too, and use CMake and TDD.

namespace fs = std::filesystem;

/**
 * Starts a backup/restore of files.
 */
void runCommandBackup(int argc, const char** argv) {
    if (argc < 3) {
        throw std::runtime_error("Missing path to config file.");
    }
    fs::path configFilename = fs::path(argv[2]).lexically_normal();
    
    unsigned int outputLimit = 50;
    int forceBackup = 0;
    ArgumentParser argParser({
        {'l', "limit", ArgumentParser::RequiredArg, nullptr, 'l'},
        {'f', "force", ArgumentParser::NoArg, &forceBackup, 1}
    });
    argParser.setArguments(argv, 3);
    
    int opt;
    std::string errorMessage;
    while ((opt = argParser.nextOption(&errorMessage)) != -1) {
        if (opt == 'l') {
            try {
                int n = std::stoi(argParser.getOptionArg());
                if (n < 0) {
                    outputLimit = std::numeric_limits<unsigned int>::max();
                } else {
                    outputLimit = static_cast<unsigned int>(n);
                }
            } catch (...) {
                throw std::runtime_error("Value for \"limit\" must be integer.");
            }
        } else if (opt == '?' || opt == ':') {
            throw std::runtime_error(errorMessage + ".");
        }
    }
    if (argParser.getIndex() < argc) {
        throw std::runtime_error("Invalid argument \"" + std::string(argv[argParser.getIndex()]) + "\".");
    }
    
    Application app;
    app.startBackup(configFilename, outputLimit, static_cast<bool>(forceBackup));
}

/**
 * Lists changes to make during backup.
 */
void runCommandCheck(int argc, const char** argv) {
    if (argc < 3) {
        throw std::runtime_error("Missing path to config file.");
    }
    fs::path configFilename = fs::path(argv[2]).lexically_normal();
    
    unsigned int outputLimit = 50;
    ArgumentParser argParser({
        {'l', "limit", ArgumentParser::RequiredArg, nullptr, 'l'}
    });
    argParser.setArguments(argv, 3);
    
    int opt;
    std::string errorMessage;
    while ((opt = argParser.nextOption(&errorMessage)) != -1) {
        if (opt == 'l') {
            try {
                int n = std::stoi(argParser.getOptionArg());
                if (n < 0) {
                    outputLimit = std::numeric_limits<unsigned int>::max();
                } else {
                    outputLimit = static_cast<unsigned int>(n);
                }
            } catch (...) {
                throw std::runtime_error("Value for \"limit\" must be integer.");
            }
        } else if (opt == '?' || opt == ':') {
            throw std::runtime_error(errorMessage + ".");
        }
    }
    if (argParser.getIndex() < argc) {
        throw std::runtime_error("Invalid argument \"" + std::string(argv[argParser.getIndex()]) + "\".");
    }
    
    Application app;
    app.checkBackup(configFilename, outputLimit);
}

/**
 * Displays tree of tracked files.
 */
void runCommandTree(int argc, const char** argv) {
    if (argc < 3) {
        throw std::runtime_error("Missing path to config file.");
    }
    fs::path configFilename = fs::path(argv[2]).lexically_normal();
    
    int countOnly = 0;
    int verbose = 0;
    ArgumentParser argParser({
        {'c', "count", ArgumentParser::NoArg, &countOnly, 1},
        {'v', "verbose", ArgumentParser::NoArg, &verbose, 1}
    });
    argParser.setArguments(argv, 3);
    
    int opt;
    std::string errorMessage;
    while ((opt = argParser.nextOption(&errorMessage)) != -1) {
        if (opt == '?' || opt == ':') {
            throw std::runtime_error(errorMessage + ".");
        }
    }
    if (argParser.getIndex() < argc) {
        throw std::runtime_error("Invalid argument \"" + std::string(argv[argParser.getIndex()]) + "\".");
    }
    
    Application app;
    app.printPaths(configFilename, static_cast<bool>(verbose), static_cast<bool>(countOnly));
}

/**
 * Shows the command help menu.
 */
void showHelp() {
    std::cout << "Commands:\n";
    std::cout << "  backup <CONFIG FILE> [OPTION]    Starts a backup/restore of files.\n";
    std::cout << "    -l, --limit N                      Limits output to N lines (50 by default). Use negative value for no limit.\n";
    std::cout << "    -f, --force                        Forces backup to run without confirmation check.\n";
    std::cout << "\n";
    std::cout << "  check <CONFIG FILE> [OPTION]     Lists changes to make during backup.\n";
    std::cout << "    -l, --limit N                      Limits output to N lines (50 by default). Use negative value for no limit.\n";
    std::cout << "\n";
    std::cout << "  tree <CONFIG FILE> [OPTION]      Displays tree of tracked files.\n";
    std::cout << "    -c, --count                        Only display the total count.\n";
    std::cout << "    -v, --verbose                      Show tracked file destinations.\n";
    std::cout << "\n";
    std::cout << "  help-config                      Shows config help with examples.\n";    // TODO ################################################
    std::cout << "\n";
    std::cout << "  help                             Shows this menu.\n";
    std::cout << "\n";
    std::cout << "  exit                             Exits interactive shell.\n";
}

/**
 * Parses the given arguments and attempts to run the command. The argc and
 * argv must be provided just like how main() is called (first argument is the
 * program name). Errors are caught within this function and printed to
 * standard output.
 */
void runCommand(int argc, const char** argv) {
    try {
        if (argc < 2) {
            return;
        }
        
        std::string command(argv[1]);
        if (command == "backup") {
            runCommandBackup(argc, argv);
        } else if (command == "check") {
            runCommandCheck(argc, argv);
        } else if (command == "tree") {
            runCommandTree(argc, argv);
        } else if (command == "help") {
            showHelp();
        } else if (command == "exit") {
            exit(0);
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

/**
 * Splits the given string into an array of arguments, similar to the behavior
 * of main() for the int argc and const char** argv parameters. Double quotes
 * can be used to include whitespace in an argument (double quotes can be
 * escaped with a backslash too).
 * 
 * Note: There is currently a known issue with patterns like \\\" that do not
 * behave the same way as in the Windows CMD implementation. Not sure what the
 * correct behavior should be.
 */
std::vector<char*> splitArguments(const std::string& str) {
    std::vector<char*> argumentVec;
    std::string::size_type index = 0;
    std::string currentArg = "";
    bool withinArg = false;
    bool withinQuotes = false;
    
    while (index < str.length()) {
        if (str[index] == ' ' && !withinQuotes) {    // If space and no quotes, this marks end the argument.
            if (withinArg) {
                argumentVec.push_back(new char[currentArg.size() + 1]);
                strcpy_s(argumentVec.back(), currentArg.size() + 1, currentArg.c_str());
                currentArg.clear();
                withinArg = false;
            }
        } else {
            withinArg = true;
            if (str[index] == '\"') {    // If double quote found, ignore the char and toggle withinQuotes.
                withinQuotes = !withinQuotes;
            } else if (str[index] == '\\' && index + 1 < str.length() && str[index + 1] == '\"') {    // Special case for \" sequence.
                currentArg.push_back('\"');
                ++index;
            } else {    // Else, add the regular char.
                currentArg.push_back(str[index]);
            }
        }
        ++index;
    }
    
    if (withinArg) {    // Add any remaining argument.
        argumentVec.push_back(new char[currentArg.size() + 1]);
        strcpy_s(argumentVec.back(), currentArg.size() + 1, currentArg.c_str());
    }
    argumentVec.push_back(nullptr);    // Add trailing nullptr to mark the end.
    
    return argumentVec;
}

int main(int argc, const char** argv) {
    if (argc >= 2) {
        runCommand(argc, argv);
    } else {
        showHelp();
        
        while (true) {
            std::cout << "\n>>> ";
            std::string input;
            std::getline(std::cin, input);
            input = "\"" + std::string(argv[0]) + "\" " + input;
            std::vector<char*> argumentVec = splitArguments(input);
            
            runCommand(static_cast<int>(argumentVec.size() - 1), const_cast<const char**>(argumentVec.data()));
        }
    }
    
    return 0;
}
