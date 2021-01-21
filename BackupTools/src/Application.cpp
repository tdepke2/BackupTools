#include "Application.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

char Application::pathSeparator = std::filesystem::path::preferred_separator;

bool Application::fnmatchPortable(char const* pattern, char const* str) {
    if (*pattern == '*' && *str == '.') {    // Exception when trying to match .file with star. Skip leading stars.
        do {
            ++pattern;
        } while (*pattern == '*');
    }
    
    return fnmatchPortable_(pattern, str);
}

bool Application::fnmatchPortable_(char const* pattern, char const* str) {
    while (*str != '\0') {
        if (*pattern == '*') {    // Star matches zero to n characters. Does not match separator and does not match a leading dot in a name.
            do {    // Skip consecutive stars (globstar not supported).
                ++pattern;
            } while (*pattern == '*');
            
            if (fnmatchPortable_(pattern, str)) {    // Attempt to match zero characters.
                return true;
            }
            if (*str == '.' && *(str - 1) == pathSeparator) {    // If sub-match failed and leading dot, match failed.
                return false;
            }
            do {
                if (*str == pathSeparator) {    // If sub-match failed with path separator, match failed.
                    return false;
                }
                ++str;
                if (fnmatchPortable_(pattern, str)) {    // Skip character and attempt sub-match again (includes matching against null character).
                    return true;
                }
            } while (*str != '\0');
            
            return false;
        } else if (*pattern == '?') {    // Question mark matches any one character. Does not match separator.
            if (*str == pathSeparator) {
                return false;
            }
            ++pattern;
            ++str;
        } else if (*pattern == '[') {    // Brackets match any characters contained within (including other brackets, a ] must come first) except when brackets are empty. Does not match separator.
            char const* patternHead = pattern;
            ++pattern;
            if (*pattern == '\0') {
                return *str == '[';
            }
            bool matchedChar = false;
            do {
                if (*pattern == *str) {
                    matchedChar = true;
                }
                ++pattern;
            } while (*pattern != ']' && *pattern != '\0');    // need to double check all of this ######################################################
            
            if (*pattern == ']') {
                if (matchedChar) {
                    ++pattern;
                } else {
                    return false;
                }
            } else {
                while (*patternHead != '\0') {
                    if (*pattern != *str) {
                        return false;
                    }
                    ++pattern;
                    ++str;
                }
            }
            
        } else if (*pattern == *str) {    // Else, characters should match exactly.
            ++pattern;
            ++str;
        } else {
            return false;
        }
    }
    
    while (*pattern == '*') {    // Skip trailing stars.
        ++pattern;
    }
    return *pattern == '\0';
}

bool Application::checkFileEquivalence(const fs::path& source, const fs::path& dest) const {
    /*std::uintmax_t sourceSize, destSize;
    try {
        sourceSize = fs::file_size(source);
        destSize = fs::file_size(dest);
    } catch(...) {
        return false;
    }
    if (sourceSize != destSize) {
        return false;
    }
    
    std::cout << "Sizes match, checking equivalence...\n";
    return ;*/
    
    std::ifstream sourceFile(source, std::ios::ate | std::ios::binary);    // Open files in binary mode and seek to end.
    std::ifstream destFile(dest, std::ios::ate | std::ios::binary);
    if (!sourceFile.is_open() || !destFile.is_open()) {
        return false;
    }
    const std::ios::pos_type sourceSize = sourceFile.tellg();    // Find file sizes.
    const std::ios::pos_type destSize = destFile.tellg();
    std::cout << "Size 1 = " << sourceSize << ", Size 2 = " << destSize << "\n";
    if (sourceSize != destSize) {
        return false;
    }
    
    sourceFile.seekg(0);
    destFile.seekg(0);
    
    std::istreambuf_iterator<char> sourceIter(sourceFile);
    std::istreambuf_iterator<char> destIter(destFile);
    
    return std::equal(sourceIter, std::istreambuf_iterator<char>(), destIter);    // Compare streams to check for equality (both streams are same length so this is safe).
}

void Application::loadFile(const fs::path& filename) {
    if (configFile_.is_open()) {
        configFile_.close();
    }
    configFile_.open(filename);
    if (!configFile_.is_open()) {
        throw std::runtime_error("Unable to open file.");
    }
    
    lineNumber_ = 0;
    rootPaths_.clear();
    ignorePaths_.clear();
    previousReadPaths_.clear();
    writePath_.clear();
    readPath_.clear();
    writePathSet_ = false;
    readPathSet_ = false;
}

void Application::printPaths() {
    for (std::pair<fs::path, fs::path> nextPath = getNextWriteReadPath(); !nextPath.first.empty() || !nextPath.second.empty(); nextPath = getNextWriteReadPath()) {
        
        std::cout << "        printPaths() [" << nextPath.first << "] <- [" << nextPath.second << "]\n";
    }
}

void Application::skipWhitespace(std::string::size_type& index, const std::string& str) {
    while (index < str.length() && str[index] == ' ') {
        ++index;
    }
}

std::string Application::parseNextCommand(std::string::size_type& index, const std::string& str) {
    std::string::size_type start = index;
    while (index < str.length()) {
        if (str[index] == ' ') {
            return str.substr(start, index - start);
        }
        ++index;
    }
    return str.substr(start);
}

fs::path Application::parseNextPath(std::string::size_type& index, const std::string& str) {
    std::string::size_type start = index;
    if (index < str.length() && str[index] == '\"') {
        ++index;
        while (index < str.length()) {
            if (str[index] == '\"') {
                ++index;
                return fs::path(str.substr(start + 1, index - start - 2)).lexically_normal();
            }
            ++index;
        }
    } else {
        while (index < str.length()) {
            if (str[index] == ' ') {
                return fs::path(str.substr(start, index - start)).lexically_normal();
            }
            ++index;
        }
    }
    return fs::path(str.substr(start)).lexically_normal();
}

fs::path Application::substituteRootPath(const fs::path& path) {
    auto pathIter = path.begin();
    if (pathIter != path.end()) {
        auto findResult = rootPaths_.find(*pathIter);
        if (findResult != rootPaths_.end()) {
            return findResult->second / fs::path(path.string().substr(pathIter->string().length() + 1));
        }
    }
    return path;
}

void Application::parseNextLineInFile() {
    std::string line;
    if (!getline(configFile_, line)) {
        configFile_.close();
        return;
    }
    
    ++lineNumber_;
    /*std::string::size_type lastNonSpace = line.find_last_not_of(' ');    // Trim whitespace.
    if (lastNonSpace != std::string::npos && lastNonSpace != line.length() - 1) {
        line.erase(lastNonSpace + 1);
    }
    std::string::size_type firstNonSpace = line.find_first_not_of(' ');
    if (firstNonSpace != std::string::npos && firstNonSpace != 0) {
        line.erase(0, firstNonSpace);
    }*/
    std::string::size_type index = 0;
    skipWhitespace(index, line);
    std::cout << "Line " << lineNumber_ << ": [" << line << "]\n";
    if (index >= line.length() || line[index] == '#') {
        return;
    }
    
    std::string command = parseNextCommand(index, line);
    if (command == "root") {    // Syntax: root <identifier> <replacement path>
        skipWhitespace(index, line);
        if (index >= line.length()) {
            std::cout << "Error: Missing first path.\n";
        }
        fs::path keyPath = parseNextPath(index, line);
        skipWhitespace(index, line);
        fs::path valuePath = parseNextPath(index, line);
        std::cout << "    Root: [" << keyPath << "] [" << valuePath << "]\n";
        
        rootPaths_.insert({keyPath, valuePath});
    } else if (command == "in") {    // Syntax: in <write path> [add <read path>]
        skipWhitespace(index, line);
        if (index >= line.length()) {
            std::cout << "Error: Missing first path.\n";
        }
        writePath_ = substituteRootPath(parseNextPath(index, line));
        writePathSet_ = true;
        skipWhitespace(index, line);
        if (index < line.length()) {
            command = parseNextCommand(index, line);
            if (command != "add") {
                std::cout << "Error: Unexpected data.\n";
            }
            skipWhitespace(index, line);
            if (index >= line.length()) {
                std::cout << "Error: Missing path.\n";
            }
            readPath_ = substituteRootPath(parseNextPath(index, line));
            readPathSet_ = true;
            
            std::cout << "    In: [" << writePath_ << "] [" << readPath_ << "]\n";
        }
    } else if (command == "add") {    // Syntax (write path must have previously been set): add <read path>
        skipWhitespace(index, line);
        if (!writePathSet_) {
            std::cout << "Error: Missing write path.\n";
        }
        if (index >= line.length()) {
            std::cout << "Error: Missing path.\n";
        }
        readPath_ = substituteRootPath(parseNextPath(index, line));
        readPathSet_ = true;
        
        std::cout << "    Add: [" << writePath_ << "] [" << readPath_ << "]\n";
    } else if (command == "ignore") {    // Syntax: ignore <path>
        skipWhitespace(index, line);
        if (index >= line.length()) {
            std::cout << "Error: Missing path.\n";
        }
        fs::path ignorePath = substituteRootPath(parseNextPath(index, line));
        
        std::cout << "    Ignore: [" << ignorePath << "]\n";
        
        ignorePaths_.insert(ignorePath);
    } else if (command == "include") {    // Syntax: include <path>
        skipWhitespace(index, line);
        if (index >= line.length()) {
            std::cout << "Error: Missing path.\n";
        }
        fs::path includePath = substituteRootPath(parseNextPath(index, line));
        
        std::cout << "    Include: [" << includePath << "]\n";
        
        auto findResult = ignorePaths_.find(includePath);
        if (findResult != ignorePaths_.end()) {
            ignorePaths_.erase(findResult);
        }
    } else {
        std::cout << "Error: Unknown command [" << command << "]\n";
    }
    skipWhitespace(index, line);
    if (index < line.length()) {
        std::cout << "Error: Unexpected data.\n";
    }
}

std::pair<fs::path, fs::path> Application::getNextWriteReadPath() {
    while (!readPathSet_) {
        if (!configFile_.is_open()) {
            return {fs::path(), fs::path()};
        }
        parseNextLineInFile();
    }
    
    //for () {
        
    //}
    
    readPathSet_ = false;
    
    return {writePath_, readPath_};
}
