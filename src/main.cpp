#include "BackupTools/Application.h"
#include "BackupTools/ArgumentParser.h"
#include "BackupTools/FileHandler.h"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#endif

namespace fs = std::filesystem;

/**
 * Starts a backup/restore of files.
 * 
 * Effectively runs the "check" command, asks for confirmation, then executes
 * each file operation in sequence. After completion the files are checked
 * again to confirm success and also determine if there are recursion problems
 * in the config.
 * 
 * The "limit" argument sets the max number of file operations to display for
 * each type, it does not effect any changes to files. The "skip-cache" argument
 * avoids usage of the cache file that normally keeps track of which files have
 * changed, using this option may reduce performance. The "fast-compare"
 * argument skips binary file scans and only considers files as changed if their
 * date-modified times differ. The "force" argument overrides the confirmation
 * check and the second file check at the end, ideal for automated backup
 * purposes.
 */
void runCommandBackup(int argc, const char** argv) {
    if (argc < 3) {
        throw std::runtime_error("Missing path to config file.");
    }
    fs::path configFilename = fs::path(argv[2]).lexically_normal();
    
    unsigned int outputLimit = 50;
    int skipCache = 0;
    int fastCompare = 0;
    int forceBackup = 0;
    ArgumentParser argParser({
        {'l', "limit", ArgumentParser::RequiredArg, nullptr, 'l'},
        {'\0', "skip-cache", ArgumentParser::NoArg, &skipCache, 1},
        {'\0', "fast-compare", ArgumentParser::NoArg, &fastCompare, 1},
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
    Application::BackupOptions options;
    options.outputLimit = outputLimit;
    options.displayConfirmation = true;
    options.skipCache = static_cast<bool>(skipCache);
    options.fastCompare = static_cast<bool>(fastCompare);
    options.forceBackup = static_cast<bool>(forceBackup);
    
    app.startBackup(configFilename, options);
}

/**
 * Lists changes to make during backup.
 * 
 * The configuration file is used to scan the directory hierarchy of the
 * selected files and check for changes. Changes are classified as removed
 * files, new files, modified files, and renamed/moved files. Note that files
 * are counted as modified even when two files swap their filenames (this is
 * technically just a rename, but difficult to detect without access to the
 * file inodes). Directories are also only counted towards additions or
 * deletions (never renames) for simplicity.
 * 
 * The "limit" argument sets the max number of file operations to display for
 * each type, it does not effect any changes to files. The "skip-cache" argument
 * avoids usage of the cache file that normally keeps track of which files have
 * changed, using this option may reduce performance. The "fast-compare"
 * argument skips binary file scans and only considers files as changed if their
 * date-modified times differ.
 */
void runCommandCheck(int argc, const char** argv) {
    if (argc < 3) {
        throw std::runtime_error("Missing path to config file.");
    }
    fs::path configFilename = fs::path(argv[2]).lexically_normal();
    
    unsigned int outputLimit = 50;
    int skipCache = 0;
    int fastCompare = 0;
    ArgumentParser argParser({
        {'l', "limit", ArgumentParser::RequiredArg, nullptr, 'l'},
        {'\0', "skip-cache", ArgumentParser::NoArg, &skipCache, 1},
        {'\0', "fast-compare", ArgumentParser::NoArg, &fastCompare, 1}
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
    Application::BackupOptions options;
    options.outputLimit = outputLimit;
    options.displayConfirmation = false;
    options.skipCache = static_cast<bool>(skipCache);
    options.fastCompare = static_cast<bool>(fastCompare);
    options.forceBackup = false;
    
    app.checkBackup(configFilename, options);
}

/**
 * Displays tree of tracked files.
 * 
 * The display format is similar to the unix "tree" command. Directories are
 * displayed in blue, and files in green, but ignored directories/files show up
 * in yellow. Tracked files are grouped together to tie them to a common parent
 * directory. In the case of a backup that spans multiple mount points (spread
 * across different drives) then a separate tree is displayed for each.
 * 
 * The "count" argument skips displaying the tree and just outputs the final
 * count of tracked/ignored files and directories. The "verbose" argument
 * additionally displays the destination for each tracked file in the tree. The
 * "prune" argument hides away sections of the tree that only contain ignored
 * files/directories, this does not effect the total file and directory counts.
 * These are shown with an "(...)" symbol.
 */
void runCommandTree(int argc, const char** argv) {
    if (argc < 3) {
        throw std::runtime_error("Missing path to config file.");
    }
    fs::path configFilename = fs::path(argv[2]).lexically_normal();
    
    int countOnly = 0;
    int verbose = 0;
    int pruneIgnored = 0;
    ArgumentParser argParser({
        {'c', "count", ArgumentParser::NoArg, &countOnly, 1},
        {'v', "verbose", ArgumentParser::NoArg, &verbose, 1},
        {'p', "prune", ArgumentParser::NoArg, &pruneIgnored, 1}
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
    app.printPaths(configFilename, static_cast<bool>(verbose), static_cast<bool>(countOnly), static_cast<bool>(pruneIgnored));
}

/**
 * Shows the command help menu.
 */
void showHelp();

/**
 * Shows the config file documentation.
 */
void showConfigHelp();

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
        } else if (command == "help-config") {
            showConfigHelp();
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
                strcpy(argumentVec.back(), currentArg.c_str());
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
        strcpy(argumentVec.back(), currentArg.c_str());
    }
    argumentVec.push_back(nullptr);    // Add trailing nullptr to mark the end.
    
    return argumentVec;
}

int main(int argc, const char** argv) {
    // On Windows, adjust the console mode to get escape sequences working (colored text in console).
    #ifdef _WIN32
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hConsole, ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    #endif
    
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

void showHelp() {
    std::cout << "Commands:\n";
    std::cout << "  backup <CONFIG FILE> [OPTION]    Starts a backup/restore of files.\n";
    std::cout << "    -l, --limit N                      Limits output to N lines (50 by default). Use negative value for no limit.\n";
    std::cout << "    --skip-cache                       Skips reading/writing to cache file (tracks file modifications by timestamp).\n";
    std::cout << "    --fast-compare                     Only considers modification timestamp when checking files (no binary scan).\n";
    std::cout << "    -f, --force                        Forces backup to run without confirmation check.\n";
    std::cout << "\n";
    std::cout << "  check <CONFIG FILE> [OPTION]     Lists changes to make during backup.\n";
    std::cout << "    -l, --limit N                      Limits output to N lines (50 by default). Use negative value for no limit.\n";
    std::cout << "    --skip-cache                       Skips reading/writing to cache file (tracks file modifications by timestamp).\n";
    std::cout << "    --fast-compare                     Only considers modification timestamp when checking files (no binary scan).\n";
    std::cout << "\n";
    std::cout << "  tree <CONFIG FILE> [OPTION]      Displays tree of tracked files.\n";
    std::cout << "    -c, --count                        Only display the total count.\n";
    std::cout << "    -v, --verbose                      Show tracked file destinations.\n";
    std::cout << "    -p, --prune                        Hide sub-trees that only contain ignored items.\n";
    std::cout << "\n";
    std::cout << "  help-config                      Shows config help with examples.\n";
    std::cout << "\n";
    std::cout << "  help                             Shows this menu.\n";
    std::cout << "\n";
    std::cout << "  exit                             Exits interactive shell.\n";
}

void showConfigHelp() {
    std::cout << R"RAW_STRING(
################################################################################
# Configuration files documentation (last updated on 06/23/2021).              #
################################################################################


# Comments begin with a # symbol. They must be on their own line (only
# whitespace can appear in front of the # symbol).


# Some notes about paths and glob matching:
# Paths to locations on the file system can use either forward slashes or back
# slashes, it does not matter. This is to ensure compatibility between different
# operating systems (Windows typically uses '\' characters while UNIX systems
# use '/').
# 
# If a path contains a space anywhere, then the whole path must be encased in
# double quotes. It doesn't hurt to always encase a path in double quotes to
# avoid mistakes with spaces.
# 
# Glob matching is a way to specify multiple files/directories that match some
# naming criteria. For example, matching all text files (.txt) and only text
# files in the current directory can be done with a "*.txt" pattern. The
# supported glob patterns are the following:
# 
#     Asterisk '*' matches any number of characters, including none. It does not
#     match directory separators ('/' and '\') or root path names like "C:\" or
#     "/home". See the section on "set match-hidden" for hidden files
#     exceptions.
#     Ex: "C:\Temp\*" will match all files and directories in the Temp\ area,
#         but will not match anything contained within the directories.
#     Ex: "C:\*\test*.exe" will search only the top-level directories in the C:\
#         drive and match executable files that begin with the string "test".
#     
#     Question mark '?' matches any single character. Like the asterisk, it does
#     not match directory separators or root path names.
#     Ex: "/home/bin/???" will match any three letter items in the bin/
#         directory.
#     
#     Square brackets '[' and ']' match any single character within the
#     brackets. Putting a '!' or '^' character after the first bracket inverts
#     the match (any character not contained within the brackets will match). A
#     '-' can also be used between two characters to specify a range.
#     Ex: "D:\Games\[abc]*" will match any items beginning with an a, b, or c.
#     Ex: "D:\Data\log[0-4]" will match only the first five log files like
#         "log0", "log1", etc.
#     Ex: "C:\Windows\[!a-zA-Z]*" will match any items that do not begin with a
#         letter (ones that start with a number would match).
#     
#     Double asterisk "**" matches any number of directories, including none.
#     This is similar to the regular asterisk, but can match directory
#     separators. For simplicity, this pattern must not include other adjacent
#     characters besides directory separators.
#     Ex: "C:\**\*.dat" will match any .dat files on the C:\ drive.
#     Ex: "D:\Games\**" will match everything in the Games\ directory. This is
#         slightly different than the pattern "D:\Games" because the directory
#         name "Games" will not be included in the backup.


# set <variable-name> <value>
# 
# The "set" keyword is used to set a variable that controls the behavior of the
# program after reaching this point. A variable can be set to something else at
# a later time. Currently supported variables are:
# 
# glob-matching <true/false>
#     Enables/disables glob matching. If this is set to true, all of the details
#     discussed in the previous section about glob matching will be in effect.
#     For technical reasons, the double astrisk "**" pattern can still be used
#     if glob matching is disabled.
#     
#     Default is true (globbing enabled).
# 
# match-hidden <true/false>
#     Controls glob matching of hidden items (any file or directory that begins
#     with a . character). The default behavior in UNIX systems is to skip
#     files/directories beginning with a dot that are matched by a glob pattern.
#     This variable can be used to match that behavior if desired.
#     
#     Note that this only affects glob patterns, a filename that explicitly
#     contains a leading dot will not be affected. This also does not behave
#     like the "ignore" keyword, a hidden file that does not match in the source
#     area will be checked to not exist at the destination.
#     
#     Default is true (match everything).

# This will disable glob patterns.
set glob-matching false

# This will skip tracking of hidden files/folders.
set match-hidden false


# root <identifier> <replacement-path>
# 
# The "root" keyword creates an alias for a path prefix. This is useful for
# making the backup location easy to change by keeping the path in one place.
# The path can be either relative to the current working directory, or absolute.
# To reference this path, the identifier must appear as the first element in the
# path and is used just like a directory name. Note that matching is case-
# sensitive and the identifier can cause a directory with the same name to be
# ignored.

# Create an alias for backing up the contents of the Documents\ directory.
root SOURCE_PATH1 C:\Users\Myself\Documents

# An alias for archive\ in the parent directory (using relative path).
root SOURCE_PATH2 ..\archive

# Create a destination for backups (on a flash drive, for example).
root DEST_PATH "E:\My Backup"


# ignore <path>
# 
# The "ignore" keyword marks a type of file/directory or absolute path that will
# not be considered in the backup. The ignore applies to both the source and
# destination areas.

# Skip tracking of all executable files.
ignore *.exe

# Skip tracking of the "System Volume Information" directory created on a flash
# drive (so that it doesn't get marked for deletion when running backup).
ignore "E:\System Volume Information"


# include <path>
# 
# The "include" keyword removes a previously ignored path from the list,
# effectively undoing the ignore operation past this point. This must match a
# pattern that was ignored already, otherwise an error will be shown.

# Include tracking of executable files again.
include *.exe


# in <destination-path> [add <source-path>]
# 
# The "in" keyword specifies a destination where tracked files will be placed.
# This is typically followed by one or more "add" keywords marking the items to
# track. One "add" keyword can be included on the same line, or multiple can
# follow later on.

# Keep the contents of Images\ synced with all .png files under Pictures\ (using
# some glob patterns discussed earlier).
in D:\Images add C:\Users\Myself\Pictures\**\*.png

# Backup all of SOURCE_PATH1 to DEST_PATH from previous usage of the "root"
# keyword.
in DEST_PATH add SOURCE_PATH1


# add <source-path>
# 
# The "add" keyword specifies the source files/directories to copy from. This
# requires a destination to have been set previously from the "in" keyword. Note
# that if a source item shows up more than once through usage of this function,
# then only the first instance will be considered. This is to prevent cases
# where a file would be copied to multiple locations when running a backup.

# Sync all of src\, text files in the current directory, and doc\output\ to the
# specified destination. The spacing in front of the "add" keywords is optional.
in "DEST_PATH\more stuff"
    add src
    add *.txt
    add doc\output
)RAW_STRING";
}
